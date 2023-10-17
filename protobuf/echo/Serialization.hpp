#ifndef PROTOBUF_SERIALIZATION_HPP
#define PROTOBUF_SERIALIZATION_HPP

#include "infra/util/ReallyAssert.hpp"
#include "infra/util/SharedOptional.hpp"
#include "protobuf/echo/ProtoMessageReceiver.hpp"
#include "protobuf/echo/ProtoMessageSender.hpp"

namespace services
{
    class Echo;

    class MethodDeserializer
    {
    public:
        MethodDeserializer() = default;
        MethodDeserializer(const MethodDeserializer& other) = delete;
        MethodDeserializer& operator=(const MethodDeserializer& other) = delete;
        virtual ~MethodDeserializer() = default;

        virtual void MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) = 0;
        virtual void ExecuteMethod() = 0;
        virtual bool Failed() const = 0;
    };

    class MethodSerializer
    {
    public:
        MethodSerializer() = default;
        MethodSerializer(const MethodSerializer& other) = delete;
        MethodSerializer& operator=(const MethodSerializer& other) = delete;
        virtual ~MethodSerializer() = default;

        virtual bool Serialize(infra::SharedPtr<infra::StreamWriter>&& writer) = 0;

        virtual void SerializationDone()
        {}
    };

    class MethodDeserializerDummy
        : public MethodDeserializer
    {
    public:
        explicit MethodDeserializerDummy(Echo& echo);

        void MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;
        void ExecuteMethod() override;
        bool Failed() const override;

    private:
        Echo& echo;
    };

    template<class Message, class... Args>
    class MethodDeserializerImpl
        : public MethodDeserializer
    {
    public:
        MethodDeserializerImpl(const infra::Function<void(Args...)>& method);

        void MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;
        void ExecuteMethod() override;
        bool Failed() const override;

    private:
        template<std::size_t... I>
        void Execute(std::index_sequence<I...>);

    private:
        ProtoMessageReceiver<Message> receiver;
        infra::Function<void(Args...)> method;
    };

    class MethodDeserializerFactory
    {
    public:
        template<class... Services>
        class ForServices;

        class OnHeap;

        MethodDeserializerFactory() = default;
        MethodDeserializerFactory(const MethodDeserializerFactory& other) = delete;
        MethodDeserializerFactory& operator=(const MethodDeserializerFactory& other) = delete;
        virtual ~MethodDeserializerFactory() = default;

        virtual infra::SharedPtr<infra::ByteRange> DeserializerMemory(uint32_t size) = 0;

        template<class Message, class... Args>
        infra::SharedPtr<MethodDeserializer> MakeDeserializer(const infra::Function<void(Args...)>& method);
    };

    template<class MessageList>
    struct MaxDeserializerSize;

    template<>
    struct MaxDeserializerSize<infra::List<>>
    {
        static constexpr std::size_t maxSize = 0;
    };

    template<class Front, class... Messages>
    struct MaxDeserializerSize<infra::List<Front, Messages...>>
    {
        static constexpr std::size_t maxSize = std::max(sizeof(MethodDeserializerImpl<Front>), MaxDeserializerSize<infra::List<Messages...>>::maxSize);
    };

    template<class... Services>
    struct MaxServiceSize;

    template<>
    struct MaxServiceSize<>
    {
        static constexpr std::size_t maxSize = 0;
    };

    template<class Front, class... Tail>
    struct MaxServiceSize<Front, Tail...>
    {
        static constexpr std::size_t maxSize = std::max(MaxDeserializerSize<typename Front::MethodTypeList>::maxSize, MaxServiceSize<Tail...>::maxSize);
    };

    template<class... Services>
    class MethodDeserializerFactory::ForServices
        : public MethodDeserializerFactory
    {
    public:
        infra::SharedPtr<infra::ByteRange> DeserializerMemory(uint32_t size) override;

    private:
        alignas(std::max_align_t) std::array<uint8_t, MaxServiceSize<Services...>::maxSize> storage;
        infra::AccessedBySharedPtr access{ infra::emptyFunction };
    };

    class MethodDeserializerFactory::OnHeap
        : public MethodDeserializerFactory
    {
    public:
        infra::SharedPtr<infra::ByteRange> DeserializerMemory(uint32_t size) override;

    private:
        void DeAllocate();

    private:
        infra::AccessedBySharedPtr access
        {
            [this]()
            {
                DeAllocate();
            }
        };

        infra::ByteRange memory;
    };

    template<class Message, class... Args>
    class MethodSerializerImpl
        : public MethodSerializer
    {
    public:
        MethodSerializerImpl(uint32_t serviceId, uint32_t methodId, Args... args);

        bool Serialize(infra::SharedPtr<infra::StreamWriter>&& writer) override;

    private:
        uint32_t serviceId;
        uint32_t methodId;
        bool headerSent = false;
        Message message;
        ProtoMessageSender<Message> sender{ message };
    };

    ////    Implementation    ////

    template<class Message, class... Args>
    MethodDeserializerImpl<Message, Args...>::MethodDeserializerImpl(const infra::Function<void(Args...)>& method)
        : method(method)
    {}

    template<class Message, class... Args>
    void MethodDeserializerImpl<Message, Args...>::MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        receiver.Feed(*reader);
    }

    template<class Message, class... Args>
    void MethodDeserializerImpl<Message, Args...>::ExecuteMethod()
    {
        Execute(std::make_index_sequence<sizeof...(Args)>{});
    }

    template<class Message, class... Args>
    bool MethodDeserializerImpl<Message, Args...>::Failed() const
    {
        return receiver.Failed();
    }

    template<class Message, class... Args>
    template<std::size_t... I>
    void MethodDeserializerImpl<Message, Args...>::Execute(std::index_sequence<I...>)
    {
        method(receiver.message.Get(std::integral_constant<uint32_t, I>{})...);
    }

    template<class Message, class... Args>
    MethodSerializerImpl<Message, Args...>::MethodSerializerImpl(uint32_t serviceId, uint32_t methodId, Args... args)
        : serviceId(serviceId)
        , methodId(methodId)
        , message(args...)
    {}

    template<class Message, class... Args>
    bool MethodSerializerImpl<Message, Args...>::Serialize(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer, infra::softFail);

        if (!headerSent)
        {
            infra::DataOutputStream::WithWriter<infra::CountingStreamWriter> countingStream;
            infra::ProtoFormatter countingFormatter{ countingStream };
            message.Serialize(countingFormatter);

            infra::ProtoFormatter formatter(stream);
            formatter.PutVarInt(serviceId);
            formatter.PutLengthDelimitedSize(countingStream.Writer().Processed(), methodId);

            headerSent = true;
        }

        sender.Fill(stream);
        writer = nullptr;
        return stream.Failed();
    }

    template<class Message, class... Args>
    infra::SharedPtr<MethodDeserializer> MethodDeserializerFactory::MakeDeserializer(const infra::Function<void(Args...)>& method)
    {
        using Deserializer = MethodDeserializerImpl<Message, Args...>;

        auto memory = DeserializerMemory(sizeof(Deserializer));
        auto deserializer = new (memory->begin()) Deserializer(method);
        return infra::MakeContainedSharedObject(*deserializer, memory);
    }

    template<class... Services>
    infra::SharedPtr<infra::ByteRange> MethodDeserializerFactory::ForServices<Services...>::DeserializerMemory(uint32_t size)
    {
        return access.MakeShared(infra::Head(infra::MakeRange(storage), size));
    }
}

#endif
