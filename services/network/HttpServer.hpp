#ifndef DI_COMM_HTTP_SERVER_HPP
#define DI_COMM_HTTP_SERVER_HPP

#include "infra/timer/Timer.hpp"
#include "infra/util/IntrusiveForwardList.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/WithStorage.hpp"
#include "services/network/Connection.hpp"
#include "services/network/HttpRequestParser.hpp"

namespace services
{
    class HttpPage;

    class HttpPageServer
    {
    public:
        HttpPageServer() = default;
        HttpPageServer(const HttpPageServer& other) = delete;
        HttpPageServer& operator=(const HttpPageServer& other) = delete;
        ~HttpPageServer() = default;

    public:
        virtual HttpPage* PageForRequest(const HttpRequestParser& request);
        void AddPage(services::HttpPage& page);

    private:
        infra::IntrusiveForwardList<services::HttpPage> pages;
    };

    class HttpResponseHeaderBuilder
    {
    public:
        HttpResponseHeaderBuilder(infra::TextOutputStream& output);
        HttpResponseHeaderBuilder(infra::TextOutputStream& output, infra::BoundedConstString result);

        void AddHeader(infra::BoundedConstString key, infra::BoundedConstString value);
        void AddHeader(infra::BoundedConstString key, uint32_t value);
        void AddHeader(infra::BoundedConstString key);
        void StartBody();

    private:
        infra::TextOutputStream& output;
    };

    class HttpResponse
    {
    protected:
        HttpResponse(std::size_t maxBodySize);
        HttpResponse(const HttpResponse& other) = delete;
        HttpResponse& operator=(const HttpResponse& other) = delete;
        ~HttpResponse() = default;

    public:
        void WriteResponse(infra::TextOutputStream& stream) const;

    public: // For test access
        virtual infra::BoundedConstString Status() const = 0;
        virtual void WriteBody(infra::TextOutputStream& stream) const = 0;
        virtual infra::BoundedConstString ContentType() const = 0;
        virtual void AddHeaders(HttpResponseHeaderBuilder& builder) const;

    private:
        std::size_t maxBodySize;
    };

    class HttpServerConnection
    {
    protected:
        HttpServerConnection() = default;
        HttpServerConnection(const HttpServerConnection& other) = delete;
        HttpServerConnection& operator=(const HttpServerConnection& other) = delete;
        ~HttpServerConnection() = default;

    public:
        virtual void SendResponse(const HttpResponse& response) = 0;
        virtual void TakeOverConnection(ConnectionObserver& observer) = 0;
    };

    class HttpPage
        : public infra::IntrusiveForwardList<HttpPage>::NodeType
    {
    protected:
        HttpPage() = default;
        HttpPage(const HttpPage& other) = delete;
        HttpPage& operator=(const HttpPage& other) = delete;
        ~HttpPage() = default;

    public:
        virtual bool ServesRequest(const infra::Tokenizer& pathTokens) const = 0;
        virtual void RespondToRequest(HttpRequestParser& parser, HttpServerConnection& connection) = 0;
    };

    struct DefaultConnectionCreationContext
    {
        infra::BoundedString& buffer;
        HttpPageServer& httpServer;
    };

    class HttpServerConnectionObserver
        : public ConnectionObserver
        , public HttpServerConnection
        , public HttpPageServer
    {
    public:
        template<::size_t BufferSize>
            using WithBuffer = infra::WithStorage<HttpServerConnectionObserver, infra::BoundedString::WithStorage<BufferSize>>;

        HttpServerConnectionObserver(infra::BoundedString& buffer, HttpPageServer& httpServer);
        HttpServerConnectionObserver(const DefaultConnectionCreationContext& creationContext);

        // Implementation of ConnectionObserver
        virtual void Connected() override;
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;
        virtual void ClosingConnection() override;

        void Close();
        void CloseWhenIdle();

        // Implementation of HttpServerConnection
        virtual void SendResponse(const HttpResponse& response) override;
        virtual void TakeOverConnection(ConnectionObserver& newObserver) override;

    protected:
        virtual void SendingHttpResponse(infra::BoundedConstString response) {}
        virtual void ReceivedHttpRequest(infra::BoundedConstString request) {}
        virtual HttpPage* PageForRequest(const HttpRequestParser& request) override;

    private:
        void ReceivedTooMuchData(infra::TextInputStream& receiveStream);
        void ReceivedRequest(infra::TextInputStream& receiveStream, std::size_t available);
        void TryHandleRequest(HttpRequestParser& request);
        void HandleRequest(HttpRequestParser& request);
        void RequestIsNowInProgress();
        void ServePage(HttpRequestParser& request);
        void RequestSendStream();
        void PrepareForNextRequest();
        bool Expect100(HttpRequestParser& request) const;

    protected:
        infra::SharedPtr<infra::StreamWriter> streamWriter;

    protected:
        HttpPageServer& httpServer;
        Connection* connection = nullptr;
        bool send100Response = false;
        infra::BoundedString& buffer;
        bool closeWhenIdle = false;
        bool idle = false;
        bool requestInProgress = false;
        infra::TimerSingleShot initialIdle;
    };

    template<class ConnectionCreationContext>
    class HttpServerConnectionObserverFactory
    {
    protected:
        HttpServerConnectionObserverFactory() = default;
        HttpServerConnectionObserverFactory(const HttpServerConnectionObserverFactory& other) = delete;
        HttpServerConnectionObserverFactory& operator=(const HttpServerConnectionObserverFactory& other) = delete;
        ~HttpServerConnectionObserverFactory() = default;

    public:
        virtual infra::SharedPtr<HttpServerConnectionObserver> Emplace(const ConnectionCreationContext& context) = 0;
        virtual void OnAllocatable(infra::Function<void()> onAllocatable) = 0;
        virtual bool Allocatable() const = 0;
        virtual bool Allocated() const = 0;
        virtual HttpServerConnectionObserver& Get() = 0;
    };

    template<class ConnectionCreationContext, class Connection>
    class HttpServerConnectionObserverFactoryImpl
        : public HttpServerConnectionObserverFactory<ConnectionCreationContext>
    {
    public:
        HttpServerConnectionObserverFactoryImpl();

        virtual infra::SharedPtr<HttpServerConnectionObserver> Emplace(const ConnectionCreationContext& context) override;
        virtual void OnAllocatable(infra::Function<void()> onAllocatable) override;
        virtual bool Allocatable() const override;
        virtual bool Allocated() const override;
        virtual HttpServerConnectionObserver& Get() override;

    private:
        infra::NotifyingSharedOptional<Connection> connectionObserver;
        infra::Function<void()> onAllocatable;
    };

    template<class ConnectionCreationContext>
    class SingleConnectionHttpServer
        : public ServerConnectionObserverFactory
        , public HttpPageServer
    {
    public:
        template<class Connection>
            using WithFactoryFor = infra::WithStorage<SingleConnectionHttpServer, HttpServerConnectionObserverFactoryImpl<ConnectionCreationContext, Connection>>;

        SingleConnectionHttpServer(HttpServerConnectionObserverFactory<ConnectionCreationContext>& factory, ConnectionCreationContext& creationContext, ConnectionFactory& connectionFactory, uint16_t port);
        ~SingleConnectionHttpServer();

        virtual void Stop(const infra::Function<void()>& onDone);

    public:
        virtual void ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver, IPAddress address) override;

    private:
        void ObserverAllocatable();

    private:
        HttpServerConnectionObserverFactory<ConnectionCreationContext>& factory;
        ConnectionCreationContext& creationContext;
        infra::SharedPtr<void> listener;

        infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)> createdObserver;
        IPAddress address;
        infra::Function<void()> onStopped;
    };

    class DefaultHttpServer
        : public SingleConnectionHttpServer<DefaultConnectionCreationContext>::WithFactoryFor<HttpServerConnectionObserver>
    {
    public:
        template<::size_t BufferSize>
            using WithBuffer = infra::WithStorage<DefaultHttpServer, infra::BoundedString::WithStorage<BufferSize>>;

        DefaultHttpServer(infra::BoundedString& buffer, ConnectionFactory& connectionFactory, uint16_t port);

    private:
        DefaultConnectionCreationContext creationContext;
    };

    ////   Implementation    ////

    template<class ConnectionCreationContext, class Connection>
    HttpServerConnectionObserverFactoryImpl<ConnectionCreationContext, Connection>::HttpServerConnectionObserverFactoryImpl()
        : connectionObserver([this]() { this->onAllocatable(); })
    {}

    template<class ConnectionCreationContext, class Connection>
    infra::SharedPtr<HttpServerConnectionObserver> HttpServerConnectionObserverFactoryImpl<ConnectionCreationContext, Connection>::Emplace(const ConnectionCreationContext& creationContext)
    {
        return connectionObserver.Emplace(creationContext);
    }

    template<class ConnectionCreationContext, class Connection>
    void HttpServerConnectionObserverFactoryImpl<ConnectionCreationContext, Connection>::OnAllocatable(infra::Function<void()> onAllocatable)
    {
        this->onAllocatable = onAllocatable;
    }

    template<class ConnectionCreationContext, class Connection>
    bool HttpServerConnectionObserverFactoryImpl<ConnectionCreationContext, Connection>::Allocatable() const
    {
        return connectionObserver.Allocatable();
    }

    template<class ConnectionCreationContext, class Connection>
    bool HttpServerConnectionObserverFactoryImpl<ConnectionCreationContext, Connection>::Allocated() const
    {
        return static_cast<bool>(connectionObserver);
    }

    template<class ConnectionCreationContext, class Connection>
    HttpServerConnectionObserver& HttpServerConnectionObserverFactoryImpl<ConnectionCreationContext, Connection>::Get()
    {
        return *connectionObserver;
    }

    template<class ConnectionCreationContext>
    SingleConnectionHttpServer<ConnectionCreationContext>::SingleConnectionHttpServer(HttpServerConnectionObserverFactory<ConnectionCreationContext>& factory, ConnectionCreationContext& creationContext, ConnectionFactory& connectionFactory, uint16_t port)
        : factory(factory)
        , creationContext(creationContext)
    {
        factory.OnAllocatable([this]() { ObserverAllocatable(); });
        listener = connectionFactory.Listen(port, *this);
    }

    template<class ConnectionCreationContext>
    SingleConnectionHttpServer<ConnectionCreationContext>::~SingleConnectionHttpServer()
    {
        really_assert(factory.Allocatable());
    }

    template<class ConnectionCreationContext>
    void SingleConnectionHttpServer<ConnectionCreationContext>::Stop(const infra::Function<void()>& onDone)
    {
        onStopped = onDone;
        if (!factory.Allocatable())
        {
            factory.OnAllocatable([this]() { onStopped(); });

            if (factory.Allocated())
                factory.Get().Close();
        }
        else
            onStopped();
    }

    template<class ConnectionCreationContext>
    void SingleConnectionHttpServer<ConnectionCreationContext>::ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver, IPAddress address)
    {
        this->address = address;
        this->createdObserver = std::move(createdObserver);

        if (factory.Allocatable())
            ObserverAllocatable();
        else if (factory.Allocated())
            factory.Get().CloseWhenIdle();
    }

    template<class ConnectionCreationContext>
    void SingleConnectionHttpServer<ConnectionCreationContext>::ObserverAllocatable()
    {
        if (createdObserver)
            createdObserver(factory.Emplace(creationContext));
    }
}

#endif
