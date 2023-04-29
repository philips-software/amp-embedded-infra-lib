#include "args.hxx"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "hal/generic/TimerServiceGeneric.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/network/HttpClientBasic.hpp"
#include "services/network/HttpClientImpl.hpp"
#include "services/network/WebSocketClientConnectionObserver.hpp"
#include "services/network_instantiations/NetworkAdapter.hpp"
#include "services/tracer/TracerOnIoOutputInfrastructure.hpp"
#include <mutex>

namespace application
{
    class ConsoleClientConnection
        : public services::ConnectionObserver
    {
    public:
        explicit ConsoleClientConnection();

        // Implementation of ConnectionObserver
        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void DataReceived() override;
        void Attached() override;

    private:
        void CheckDataToBeSent();

    private:
        std::mutex mutex;
        std::string dataToBeSent;
        infra::SharedPtr<infra::StreamWriter> writer;
    };

    ConsoleClientConnection::ConsoleClientConnection()
    {
        std::thread([this]()
            {
                while (true)
                {
                    std::string data;
                    std::getline(std::cin, data);
                    data += "\n";

                    std::scoped_lock lock(mutex);
                    dataToBeSent += data;
                    infra::EventDispatcher::Instance().Schedule([this]() { CheckDataToBeSent(); });
                } })
            .detach();
    }

    void ConsoleClientConnection::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        this->writer = writer;
        writer = nullptr;

        CheckDataToBeSent();
    }

    void ConsoleClientConnection::DataReceived()
    {
        auto reader = Subject().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*reader);

        while (!stream.Empty())
            std::cout << infra::ByteRangeAsStdString(stream.ContiguousRange()) << std::flush;

        Subject().AckReceived();
    }

    void ConsoleClientConnection::Attached()
    {
        services::ConnectionObserver::Subject().RequestSendStream(services::ConnectionObserver::Subject().MaxSendStreamSize());
    }

    void ConsoleClientConnection::CheckDataToBeSent()
    {
        std::scoped_lock lock(mutex);

        if (writer != nullptr && !dataToBeSent.empty())
        {
            infra::TextOutputStream::WithErrorPolicy stream(*writer);
            std::size_t amount = std::min(stream.Available(), dataToBeSent.size());
            stream << dataToBeSent.substr(0, amount);
            dataToBeSent.erase(0, amount);

            writer = nullptr;
            services::ConnectionObserver::Subject().RequestSendStream(services::ConnectionObserver::Subject().MaxSendStreamSize());
        }
    }

    class ConsoleFactory
        : public services::WebSocketClientObserverFactory
    {
    public:
        explicit ConsoleFactory(infra::BoundedString url, services::Tracer& tracer);

    public:
        virtual infra::BoundedString Url() const override;
        virtual uint16_t Port() const override;
        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> client)>&& createdClientObserver) override;
        virtual void ConnectionFailed(ConnectFailReason reason) override;

    private:
        infra::BoundedString url;
        services::Tracer& tracer;
        infra::SharedOptional<ConsoleClientConnection> consoleClientConnection;
    };

    ConsoleFactory::ConsoleFactory(infra::BoundedString url, services::Tracer& tracer)
        : url(url)
        , tracer(tracer)
    {}

    infra::BoundedString ConsoleFactory::Url() const
    {
        return url;
    }

    uint16_t ConsoleFactory::Port() const
    {
        return 80;
    }

    void ConsoleFactory::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> client)>&& createdClientObserver)
    {
        tracer.Trace() << "Connection established";
        tracer.Trace();
        createdClientObserver(consoleClientConnection.Emplace());
    }

    void ConsoleFactory::ConnectionFailed(ConnectFailReason reason)
    {
        tracer.Trace() << "Connection failed";
        exit(1);
    }
}

int main(int argc, const char* argv[], const char* env[])
{
    std::string toolname = argv[0];
    args::ArgumentParser parser(toolname + " is a tool to demonstrate a websocket client.");
    args::Group arguments(parser, "Arguments:");
    args::Positional<std::string> url(arguments, "url", "URL of a websocket", args::Options::Required);
    args::Group flags(parser, "Optional flags:");
    args::HelpFlag help(flags, "help", "Display this help menu.", { 'h', "help" });

    try
    {
        parser.Prog(toolname);
        parser.ParseCLI(argc, argv);

        static hal::TimerServiceGeneric timerService;
        static hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
        static main_::TracerOnIoOutputInfrastructure tracer;
        static main_::NetworkAdapter network;

        static services::ConnectionFactoryWithNameResolverImpl::WithStorage<1> connectionFactory(network.ConnectionFactory(), network.NameResolver());
        static services::HttpClientConnectorWithNameResolverImpl<> clientConnector{ connectionFactory };
        static infra::Creator<services::Stoppable, services::HttpClientWebSocketInitiation, void(services::WebSocketClientObserverFactory & clientObserverFactory, services::HttpClientWebSocketInitiationResult & result, hal::SynchronousRandomDataGenerator & randomDataGenerator)> httpClientInitiationCreator{ [](infra::Optional<services::HttpClientWebSocketInitiation>& value, services::WebSocketClientObserverFactory& clientObserverFactory,
                                                                                                                                                                                                                                                                                                                        services::HttpClientWebSocketInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator)
            {
                value.Emplace(clientObserverFactory, clientConnector, result, randomDataGenerator);
            } };
        static services::WebSocketClientFactorySingleConnection webSocketFactory{ randomDataGenerator, { httpClientInitiationCreator } };

        application::ConsoleFactory consoleFactory(get(url), tracer.tracer);
        webSocketFactory.Connect(consoleFactory);

        network.Run();
    }
    catch (const args::Help&)
    {
        std::cout << parser;
        return 1;
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
