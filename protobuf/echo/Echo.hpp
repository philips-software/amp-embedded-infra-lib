#ifndef PROTOBUF_ECHO_HPP
#define PROTOBUF_ECHO_HPP

#include "infra/syntax/ProtoFormatter.hpp"
#include "infra/syntax/ProtoParser.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/Compatibility.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Optional.hpp"
#include "services/util/MessageCommunication.hpp"

namespace services
{
    class Echo;
    class Service;
    class ServiceProxy;

    struct ProtoBool
    {};

    struct ProtoUInt32
    {};

    struct ProtoInt32
    {};

    struct ProtoUInt64
    {};

    struct ProtoInt64
    {};

    struct ProtoFixed32
    {};

    struct ProtoFixed64
    {};

    struct ProtoSFixed32
    {};

    struct ProtoSFixed64
    {};

    struct ProtoUnboundedString
    {};

    struct ProtoUnboundedBytes
    {};

    template<class T>
    struct ProtoMessage
    {};

    template<class T>
    struct ProtoEnum
    {};

    template<std::size_t Max>
    struct ProtoBytes
    {};

    template<std::size_t Max>
    struct ProtoString
    {};

    template<std::size_t Max, class T>
    struct ProtoRepeated
    {};

    template<class T>
    struct ProtoUnboundedRepeated
    {};

    void SerializeField(ProtoBool, infra::ProtoFormatter& formatter, bool value, uint32_t fieldNumber);
    void SerializeField(ProtoUInt32, infra::ProtoFormatter& formatter, uint32_t value, uint32_t fieldNumber);
    void SerializeField(ProtoInt32, infra::ProtoFormatter& formatter, int32_t value, uint32_t fieldNumber);
    void SerializeField(ProtoUInt64, infra::ProtoFormatter& formatter, uint64_t value, uint32_t fieldNumber);
    void SerializeField(ProtoInt64, infra::ProtoFormatter& formatter, int64_t value, uint32_t fieldNumber);
    void SerializeField(ProtoFixed32, infra::ProtoFormatter& formatter, uint32_t value, uint32_t fieldNumber);
    void SerializeField(ProtoFixed64, infra::ProtoFormatter& formatter, uint64_t value, uint32_t fieldNumber);
    void SerializeField(ProtoSFixed32, infra::ProtoFormatter& formatter, int32_t value, uint32_t fieldNumber);
    void SerializeField(ProtoSFixed64, infra::ProtoFormatter& formatter, int64_t value, uint32_t fieldNumber);
    void SerializeField(ProtoUnboundedString, infra::ProtoFormatter& formatter, const std::string& value, uint32_t fieldNumber);
    void SerializeField(ProtoUnboundedBytes, infra::ProtoFormatter& formatter, const std::vector<uint8_t>& value, uint32_t fieldNumber);

    template<std::size_t Max, class T, class U>
    void SerializeField(ProtoRepeated<Max, T>, infra::ProtoFormatter& formatter, const infra::BoundedVector<U>& value, uint32_t fieldNumber);
    template<class T, class U>
    void SerializeField(ProtoUnboundedRepeated<T>, infra::ProtoFormatter& formatter, const std::vector<U>& value, uint32_t fieldNumber);
    template<class T>
    void SerializeField(ProtoUnboundedRepeated<T>, infra::ProtoFormatter& formatter, const std::vector<bool>& value, uint32_t fieldNumber);
    template<class T, class U>
    void SerializeField(ProtoMessage<T>, infra::ProtoFormatter& formatter, const U& value, uint32_t fieldNumber);
    template<class T>
    void SerializeField(ProtoEnum<T>, infra::ProtoFormatter& formatter, T value, uint32_t fieldNumber);
    template<std::size_t Max>
    void SerializeField(ProtoBytes<Max>, infra::ProtoFormatter& formatter, const infra::BoundedVector<uint8_t>& value, uint32_t fieldNumber);
    template<std::size_t Max>
    void SerializeField(ProtoString<Max>, infra::ProtoFormatter& formatter, infra::BoundedConstString value, uint32_t fieldNumber);

    void DeserializeField(ProtoBool, infra::ProtoParser& parser, infra::ProtoParser::Field& field, bool& value);
    void DeserializeField(ProtoUInt32, infra::ProtoParser& parser, infra::ProtoParser::Field& field, uint32_t& value);
    void DeserializeField(ProtoInt32, infra::ProtoParser& parser, infra::ProtoParser::Field& field, int32_t& value);
    void DeserializeField(ProtoUInt64, infra::ProtoParser& parser, infra::ProtoParser::Field& field, uint64_t& value);
    void DeserializeField(ProtoInt64, infra::ProtoParser& parser, infra::ProtoParser::Field& field, int64_t& value);
    void DeserializeField(ProtoFixed32, infra::ProtoParser& parser, infra::ProtoParser::Field& field, uint32_t& value);
    void DeserializeField(ProtoFixed64, infra::ProtoParser& parser, infra::ProtoParser::Field& field, uint64_t& value);
    void DeserializeField(ProtoSFixed32, infra::ProtoParser& parser, infra::ProtoParser::Field& field, int32_t& value);
    void DeserializeField(ProtoSFixed64, infra::ProtoParser& parser, infra::ProtoParser::Field& field, int64_t& value);
    void DeserializeField(ProtoUnboundedString, infra::ProtoParser& parser, infra::ProtoParser::Field& field, std::string& value);
    void DeserializeField(ProtoUnboundedBytes, infra::ProtoParser& parser, infra::ProtoParser::Field& field, std::vector<uint8_t>& value);

    template<std::size_t Max, class T, class U>
    void DeserializeField(ProtoRepeated<Max, T>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, infra::BoundedVector<U>& value);
    template<class T, class U>
    void DeserializeField(ProtoUnboundedRepeated<T>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, std::vector<U>& value);
    template<class T>
    void DeserializeField(ProtoUnboundedRepeated<T>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, std::vector<bool>& value);
    template<class T, class U>
    void DeserializeField(ProtoMessage<T>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, U& value);
    template<class T>
    void DeserializeField(ProtoEnum<T>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, T& value);
    template<std::size_t Max>
    void DeserializeField(ProtoBytes<Max>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, infra::BoundedVector<uint8_t>& value);
    template<std::size_t Max>
    void DeserializeField(ProtoBytes<Max>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, infra::ConstByteRange& value);
    template<std::size_t Max>
    void DeserializeField(ProtoString<Max>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, infra::BoundedString& value);
    template<std::size_t Max>
    void DeserializeField(ProtoString<Max>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, infra::BoundedConstString& value);

    class EchoErrorPolicy
    {
    protected:
        ~EchoErrorPolicy() = default;

    public:
        virtual void MessageFormatError() = 0;
        virtual void ServiceNotFound(uint32_t serviceId) = 0;
        virtual void MethodNotFound(uint32_t serviceId, uint32_t methodId) = 0;
    };

    class EchoErrorPolicyAbortOnMessageFormatError
        : public EchoErrorPolicy
    {
    public:
        void MessageFormatError() override;
        void ServiceNotFound(uint32_t serviceId) override;
        void MethodNotFound(uint32_t serviceId, uint32_t methodId) override;
    };

    class EchoErrorPolicyAbort
        : public EchoErrorPolicyAbortOnMessageFormatError
    {
    public:
        void ServiceNotFound(uint32_t serviceId) override;
        void MethodNotFound(uint32_t serviceId, uint32_t methodId) override;
    };

    extern EchoErrorPolicyAbortOnMessageFormatError echoErrorPolicyAbortOnMessageFormatError;
    extern EchoErrorPolicyAbort echoErrorPolicyAbort;

    class Service
        : public infra::Observer<Service, Echo>
    {
    public:
        using infra::Observer<Service, Echo>::Observer;

        virtual bool AcceptsService(uint32_t id) const = 0;

        void MethodDone();
        bool InProgress() const;
        void HandleMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents, EchoErrorPolicy& errorPolicy);

    protected:
        Echo& Rpc();
        virtual void Handle(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents, EchoErrorPolicy& errorPolicy) = 0;

    private:
        bool inProgress = false;
    };

    class Echo
        : public infra::Subject<Service>
    {
    public:
        virtual void RequestSend(ServiceProxy& serviceProxy) = 0;
        virtual infra::StreamWriter& SendStreamWriter() = 0;
        virtual void Send() = 0;
        virtual void ServiceDone(Service& service) = 0;
    };

    class ServiceProxy
        : public infra::IntrusiveList<ServiceProxy>::NodeType
    {
    public:
        ServiceProxy(Echo& echo, uint32_t maxMessageSize);

        Echo& Rpc();
        virtual void RequestSend(infra::Function<void()> onGranted);
        void GrantSend();
        uint32_t MaxMessageSize() const;

    private:
        Echo& echo;
        uint32_t maxMessageSize;
        infra::Function<void()> onGranted;
    };

    template<class TServiceProxy>
    class ServiceProxyResponseQueue : public TServiceProxy
    {
    public:
        using Container = infra::BoundedDeque<infra::Function<void()>>;

        template<std::size_t Max>
        using WithStorage = infra::WithStorage<ServiceProxyResponseQueue, typename Container::template WithMaxSize<Max>>;

        template<class... TArgs>
        explicit ServiceProxyResponseQueue(Container& container, TArgs&&... args);

        void RequestSend(infra::Function<void()> onRequestGranted) override;

    private:
        void ProcessSendQueue();

        Container& container;

        bool responseInProgress{ false };
    };

    class EchoOnStreams
        : public Echo
        , public infra::EnableSharedFromThis<EchoOnStreams>
    {
    public:
        explicit EchoOnStreams(EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);

        // Implementation of Echo
        void RequestSend(ServiceProxy& serviceProxy) override;
        infra::StreamWriter& SendStreamWriter() override;
        void Send() override;
        void ServiceDone(Service& service) override;

    protected:
        virtual void RequestSendStream(std::size_t size) = 0;
        virtual void BusyServiceDone() = 0;

        virtual void ExecuteMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents, infra::StreamReaderWithRewinding& reader);
        virtual void SetStreamWriter(infra::SharedPtr<infra::StreamWriter>&& writer);
        bool ServiceBusy() const;
        bool ProcessMessage(infra::StreamReaderWithRewinding& reader);

    protected:
        EchoErrorPolicy& errorPolicy;

    private:
        infra::SharedPtr<infra::StreamWriter> streamWriter;
        infra::IntrusiveList<ServiceProxy> sendRequesters;
        infra::Optional<uint32_t> serviceBusy;
    };

    ////    Implementation    ////

    inline void SerializeField(ProtoBool, infra::ProtoFormatter& formatter, bool value, uint32_t fieldNumber)
    {
        formatter.PutVarIntField(value, fieldNumber);
    }

    inline void SerializeField(ProtoUInt32, infra::ProtoFormatter& formatter, uint32_t value, uint32_t fieldNumber)
    {
        formatter.PutVarIntField(value, fieldNumber);
    }

    inline void SerializeField(ProtoInt32, infra::ProtoFormatter& formatter, int32_t value, uint32_t fieldNumber)
    {
        formatter.PutVarIntField(value, fieldNumber);
    }

    inline void SerializeField(ProtoUInt64, infra::ProtoFormatter& formatter, uint64_t value, uint32_t fieldNumber)
    {
        formatter.PutVarIntField(value, fieldNumber);
    }

    inline void SerializeField(ProtoInt64, infra::ProtoFormatter& formatter, int64_t value, uint32_t fieldNumber)
    {
        formatter.PutVarIntField(value, fieldNumber);
    }

    inline void SerializeField(ProtoFixed32, infra::ProtoFormatter& formatter, uint32_t value, uint32_t fieldNumber)
    {
        formatter.PutFixed32Field(value, fieldNumber);
    }

    inline void SerializeField(ProtoFixed64, infra::ProtoFormatter& formatter, uint64_t value, uint32_t fieldNumber)
    {
        formatter.PutFixed64Field(value, fieldNumber);
    }

    inline void SerializeField(ProtoSFixed32, infra::ProtoFormatter& formatter, int32_t value, uint32_t fieldNumber)
    {
        formatter.PutFixed32Field(static_cast<uint32_t>(value), fieldNumber);
    }

    inline void SerializeField(ProtoSFixed64, infra::ProtoFormatter& formatter, int64_t value, uint32_t fieldNumber)
    {
        formatter.PutFixed64Field(static_cast<uint64_t>(value), fieldNumber);
    }

    inline void SerializeField(ProtoUnboundedString, infra::ProtoFormatter& formatter, const std::string& value, uint32_t fieldNumber)
    {
        formatter.PutStringField(value, fieldNumber);
    }

    inline void SerializeField(ProtoUnboundedBytes, infra::ProtoFormatter& formatter, const std::vector<uint8_t>& value, uint32_t fieldNumber)
    {
        formatter.PutBytesField(value, fieldNumber);
    }

    template<std::size_t Max, class T, class U>
    void SerializeField(ProtoRepeated<Max, T>, infra::ProtoFormatter& formatter, const infra::BoundedVector<U>& value, uint32_t fieldNumber)
    {
        for (auto& v : value)
            SerializeField(T(), formatter, v, fieldNumber);
    }

    template<class T, class U>
    void SerializeField(ProtoUnboundedRepeated<T>, infra::ProtoFormatter& formatter, const std::vector<U>& value, uint32_t fieldNumber)
    {
        for (auto& v : value)
            SerializeField(T(), formatter, v, fieldNumber);
    }

    template<class T>
    void SerializeField(ProtoUnboundedRepeated<T>, infra::ProtoFormatter& formatter, const std::vector<bool>& value, uint32_t fieldNumber)
    {
        for (auto v : value)
            SerializeField(T(), formatter, v, fieldNumber);
    }

    template<class T, class U>
    void SerializeField(ProtoMessage<T>, infra::ProtoFormatter& formatter, const U& value, uint32_t fieldNumber)
    {
        infra::ProtoLengthDelimitedFormatter nestedMessage(formatter, fieldNumber);
        value.Serialize(formatter);
    }

    template<class T>
    void SerializeField(ProtoEnum<T>, infra::ProtoFormatter& formatter, T value, uint32_t fieldNumber)
    {
        formatter.PutVarIntField(static_cast<uint64_t>(value), fieldNumber);
    }

    template<std::size_t Max>
    void SerializeField(ProtoBytes<Max>, infra::ProtoFormatter& formatter, const infra::BoundedVector<uint8_t>& value, uint32_t fieldNumber)
    {
        formatter.PutBytesField(infra::MakeRange(value), fieldNumber);
    }

    template<std::size_t Max>
    void SerializeField(ProtoString<Max>, infra::ProtoFormatter& formatter, infra::BoundedConstString value, uint32_t fieldNumber)
    {
        formatter.PutStringField(value, fieldNumber);
    }

    inline void DeserializeField(ProtoBool, infra::ProtoParser& parser, infra::ProtoParser::Field& field, bool& value)
    {
        parser.ReportFormatResult(field.first.Is<uint64_t>());
        if (field.first.Is<uint64_t>())
            value = field.first.Get<uint64_t>() != 0;
    }

    inline void DeserializeField(ProtoUInt32, infra::ProtoParser& parser, infra::ProtoParser::Field& field, uint32_t& value)
    {
        parser.ReportFormatResult(field.first.Is<uint64_t>());
        if (field.first.Is<uint64_t>())
            value = static_cast<uint32_t>(field.first.Get<uint64_t>());
    }

    inline void DeserializeField(ProtoInt32, infra::ProtoParser& parser, infra::ProtoParser::Field& field, int32_t& value)
    {
        parser.ReportFormatResult(field.first.Is<uint64_t>());
        if (field.first.Is<uint64_t>())
            value = static_cast<int32_t>(field.first.Get<uint64_t>());
    }

    inline void DeserializeField(ProtoUInt64, infra::ProtoParser& parser, infra::ProtoParser::Field& field, uint64_t& value)
    {
        parser.ReportFormatResult(field.first.Is<uint64_t>());
        if (field.first.Is<uint64_t>())
            value = field.first.Get<uint64_t>();
    }

    inline void DeserializeField(ProtoInt64, infra::ProtoParser& parser, infra::ProtoParser::Field& field, int64_t& value)
    {
        parser.ReportFormatResult(field.first.Is<uint64_t>());
        if (field.first.Is<uint64_t>())
            value = static_cast<int64_t>(field.first.Get<uint64_t>());
    }

    inline void DeserializeField(ProtoFixed32, infra::ProtoParser& parser, infra::ProtoParser::Field& field, uint32_t& value)
    {
        parser.ReportFormatResult(field.first.Is<uint32_t>());
        if (field.first.Is<uint32_t>())
            value = field.first.Get<uint32_t>();
    }

    inline void DeserializeField(ProtoFixed64, infra::ProtoParser& parser, infra::ProtoParser::Field& field, uint64_t& value)
    {
        parser.ReportFormatResult(field.first.Is<uint64_t>());
        if (field.first.Is<uint64_t>())
            value = field.first.Get<uint64_t>();
    }

    inline void DeserializeField(ProtoSFixed32, infra::ProtoParser& parser, infra::ProtoParser::Field& field, int32_t& value)
    {
        parser.ReportFormatResult(field.first.Is<uint32_t>());
        if (field.first.Is<uint32_t>())
            value = static_cast<int32_t>(field.first.Get<uint32_t>());
    }

    inline void DeserializeField(ProtoSFixed64, infra::ProtoParser& parser, infra::ProtoParser::Field& field, int64_t& value)
    {
        parser.ReportFormatResult(field.first.Is<uint64_t>());
        if (field.first.Is<uint64_t>())
            value = static_cast<int64_t>(field.first.Get<uint64_t>());
    }

    inline void DeserializeField(ProtoUnboundedString, infra::ProtoParser& parser, infra::ProtoParser::Field& field, std::string& value)
    {
        parser.ReportFormatResult(field.first.Is<infra::ProtoLengthDelimited>());
        if (field.first.Is<infra::ProtoLengthDelimited>())
            value = field.first.Get<infra::ProtoLengthDelimited>().GetStdString();
    }

    inline void DeserializeField(ProtoUnboundedBytes, infra::ProtoParser& parser, infra::ProtoParser::Field& field, std::vector<uint8_t>& value)
    {
        parser.ReportFormatResult(field.first.Is<infra::ProtoLengthDelimited>());
        if (field.first.Is<infra::ProtoLengthDelimited>())
            value = field.first.Get<infra::ProtoLengthDelimited>().GetUnboundedBytes();
    }

    template<std::size_t Max, class T, class U>
    void DeserializeField(ProtoRepeated<Max, T>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, infra::BoundedVector<U>& value)
    {
        parser.ReportFormatResult(!value.full());
        if (!value.full())
        {
            value.emplace_back();
            DeserializeField(T(), parser, field, value.back());
        }
    }

    template<class T, class U>
    void DeserializeField(ProtoUnboundedRepeated<T>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, std::vector<U>& value)
    {
        value.emplace_back();
        DeserializeField(T(), parser, field, value.back());
    }

    template<class T>
    void DeserializeField(ProtoUnboundedRepeated<T>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, std::vector<bool>& value)
    {
        bool result{};
        DeserializeField(T(), parser, field, result);
        value.push_back(result);
    }

    template<class T, class U>
    void DeserializeField(ProtoMessage<T>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, U& value)
    {
        parser.ReportFormatResult(field.first.Is<infra::ProtoLengthDelimited>());
        if (field.first.Is<infra::ProtoLengthDelimited>())
        {
            infra::ProtoParser nestedParser = field.first.Get<infra::ProtoLengthDelimited>().Parser();
            value.Deserialize(nestedParser);
        }
    }

    template<class T>
    void DeserializeField(ProtoEnum<T>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, T& value)
    {
        parser.ReportFormatResult(field.first.Is<uint64_t>());
        if (field.first.Is<uint64_t>())
            value = static_cast<T>(field.first.Get<uint64_t>());
    }

    template<std::size_t Max>
    void DeserializeField(ProtoBytes<Max>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, infra::BoundedVector<uint8_t>& value)
    {
        parser.ReportFormatResult(field.first.Is<infra::ProtoLengthDelimited>());
        if (field.first.Is<infra::ProtoLengthDelimited>())
            field.first.Get<infra::ProtoLengthDelimited>().GetBytes(value);
    }

    template<std::size_t Max>
    void DeserializeField(ProtoBytes<Max>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, infra::ConstByteRange& value)
    {
        parser.ReportFormatResult(field.first.Is<infra::ProtoLengthDelimited>());
        if (field.first.Is<infra::ProtoLengthDelimited>())
            field.first.Get<infra::ProtoLengthDelimited>().GetBytesReference(value);
    }

    template<std::size_t Max>
    void DeserializeField(ProtoString<Max>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, infra::BoundedString& value)
    {
        parser.ReportFormatResult(field.first.Is<infra::ProtoLengthDelimited>());
        if (field.first.Is<infra::ProtoLengthDelimited>())
            field.first.Get<infra::ProtoLengthDelimited>().GetString(value);
    }

    template<std::size_t Max>
    void DeserializeField(ProtoString<Max>, infra::ProtoParser& parser, infra::ProtoParser::Field& field, infra::BoundedConstString& value)
    {
        parser.ReportFormatResult(field.first.Is<infra::ProtoLengthDelimited>());
        if (field.first.Is<infra::ProtoLengthDelimited>())
            field.first.Get<infra::ProtoLengthDelimited>().GetStringReference(value);
    }

    template<class TServiceProxy>
    template<class... TArgs>
    ServiceProxyResponseQueue<TServiceProxy>::ServiceProxyResponseQueue(Container& container, TArgs&&... args)
        : TServiceProxy{ std::forward<TArgs>(args)... }
        , container{ container }
    {}

    template<class TServiceProxy>
    void ServiceProxyResponseQueue<TServiceProxy>::RequestSend(infra::Function<void()> onRequestGranted)
    {
        if (container.full())
        {
            return;
        }

        container.push_back(onRequestGranted);
        ProcessSendQueue();
    }

    template<class TServiceProxy>
    void ServiceProxyResponseQueue<TServiceProxy>::ProcessSendQueue()
    {
        if (!responseInProgress && !container.empty())
        {
            responseInProgress = true;
            TServiceProxy::RequestSend([this]
                {
                    container.front()();
                    container.pop_front();

                    responseInProgress = false;
                    ProcessSendQueue();
                });
        }
    }
}

#endif
