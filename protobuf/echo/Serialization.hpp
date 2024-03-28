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
        explicit MethodDeserializerImpl(const infra::Function<void(Args...)>& method);

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

    class MethodSerializerFactory
    {
    public:
        template<class... Services>
        class ForServices;

        class OnHeap;

        MethodSerializerFactory() = default;
        MethodSerializerFactory(const MethodSerializerFactory& other) = delete;
        MethodSerializerFactory& operator=(const MethodSerializerFactory& other) = delete;
        virtual ~MethodSerializerFactory() = default;

        virtual infra::SharedPtr<infra::ByteRange> SerializerMemory(uint32_t size) = 0;
        virtual infra::SharedPtr<infra::ByteRange> DeserializerMemory(uint32_t size) = 0;

        template<class Message, class... Args>
        infra::SharedPtr<MethodSerializer> MakeSerializer(uint32_t serviceId, uint32_t methodId, Args... args);

        template<class Message, class... Args>
        infra::SharedPtr<MethodDeserializer> MakeDeserializer(const infra::Function<void(Args...)>& method);

        infra::SharedPtr<MethodDeserializer> MakeDummyDeserializer(Echo& echo);
    };

    class EmptyMessage
    {
    public:
        static const uint32_t numberOfFields = 0;
        template<std::size_t fieldIndex>
        using ProtoType = void;
        template<std::size_t fieldIndex>
        using Type = void;
        template<std::size_t fieldIndex>
        static const uint32_t fieldNumber = 0;

    public:
        void Serialize([[maybe_unused]] infra::ProtoFormatter& formatter) const
        {}
    };

    template<class MessageList>
    struct MaxSerializerSize;

    template<>
    struct MaxSerializerSize<infra::List<>>
    {
        static constexpr std::size_t maxSize = sizeof(MethodSerializerImpl<services::EmptyMessage>);
    };

    template<class Front, class... Messages>
    struct MaxSerializerSize<infra::List<Front, Messages...>>
    {
        static constexpr std::size_t maxSize = std::max(sizeof(MethodSerializerImpl<Front>), MaxSerializerSize<infra::List<Messages...>>::maxSize);
    };

    template<class MessageList>
    struct MaxDeserializerSize;

    template<>
    struct MaxDeserializerSize<infra::List<>>
    {
        static constexpr std::size_t maxSize = sizeof(MethodDeserializerImpl<services::EmptyMessage>);
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
        static constexpr std::size_t maxSize = sizeof(MethodDeserializerDummy);
    };

    template<class Front, class... Tail>
    struct MaxServiceSize<Front, Tail...>
    {
        static constexpr std::size_t maxSize = std::max(MaxDeserializerSize<typename Front::MethodTypeList>::maxSize, MaxServiceSize<Tail...>::maxSize);
    };

    template<class... ServiceProxiess>
    struct MaxServiceProxySize;

    template<>
    struct MaxServiceProxySize<>
    {
        static constexpr std::size_t maxSize = sizeof(MethodSerializerImpl<services::EmptyMessage>);
    };

    template<class Front, class... Tail>
    struct MaxServiceProxySize<Front, Tail...>
    {
        static constexpr std::size_t maxSize = std::max(MaxSerializerSize<typename Front::MethodTypeList>::maxSize, MaxServiceProxySize<Tail...>::maxSize);
    };

    template<class... Services>
    class MethodSerializerFactory::ForServices
    {
    public:
        template<class... ServiceProxies>
        class AndProxies;
    };

    template<class... Services>
    template<class... ServiceProxies>
    class MethodSerializerFactory::ForServices<Services...>::AndProxies
        : public MethodSerializerFactory
    {
    public:
        infra::SharedPtr<infra::ByteRange> SerializerMemory(uint32_t size) override;
        infra::SharedPtr<infra::ByteRange> DeserializerMemory(uint32_t size) override;

    private:
        alignas(std::max_align_t) std::array<uint8_t, MaxServiceProxySize<ServiceProxies...>::maxSize> serializerStorage;
        alignas(std::max_align_t) std::array<uint8_t, MaxServiceSize<Services...>::maxSize> deserializerStorage;
        infra::ByteRange serializerMemory;
        infra::ByteRange deserializerMemory;
        infra::AccessedBySharedPtr serializerAccess{ infra::emptyFunction };
        infra::AccessedBySharedPtr deserializerAccess{ infra::emptyFunction };
    };

    class MethodSerializerFactory::OnHeap
        : public MethodSerializerFactory
    {
    public:
        infra::SharedPtr<infra::ByteRange> SerializerMemory(uint32_t size) override;
        infra::SharedPtr<infra::ByteRange> DeserializerMemory(uint32_t size) override;

    private:
        void DeAllocateSerializer();
        void DeAllocateDeserializer();

    private:
        infra::AccessedBySharedPtr serializerAccess{
            [this]()
            {
                DeAllocateSerializer();
            }
        };

        infra::AccessedBySharedPtr deserializerAccess{
            [this]()
            {
                DeAllocateDeserializer();
            }
        };

        infra::ByteRange serializerMemory;
        infra::ByteRange deserializerMemory;
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
        method(receiver.message.GetDecayed(std::integral_constant<uint32_t, I>{})...);
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
            std::array<uint8_t, 16> saveStateStorage; // For writing length fields
            infra::DataOutputStream::WithWriter<infra::CountingStreamWriter> countingStream(saveStateStorage);
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
    infra::SharedPtr<MethodSerializer> MethodSerializerFactory::MakeSerializer(uint32_t serviceId, uint32_t methodId, Args... args)
    {
        using Serializer = MethodSerializerImpl<Message, Args...>;

        auto memory = SerializerMemory(sizeof(Serializer));
        auto serializer = new (memory->begin()) Serializer(serviceId, methodId, args...);
        return infra::MakeContainedSharedObject(*serializer, memory);
    }

    template<class Message, class... Args>
    infra::SharedPtr<MethodDeserializer> MethodSerializerFactory::MakeDeserializer(const infra::Function<void(Args...)>& method)
    {
        using Deserializer = MethodDeserializerImpl<Message, Args...>;

        auto memory = DeserializerMemory(sizeof(Deserializer));
        auto deserializer = new (memory->begin()) Deserializer(method);
        return infra::MakeContainedSharedObject(*deserializer, memory);
    }

    template<class... Services>
    template<class... ServiceProxies>
    infra::SharedPtr<infra::ByteRange> MethodSerializerFactory::ForServices<Services...>::AndProxies<ServiceProxies...>::SerializerMemory(uint32_t size)
    {
        really_assert(size <= serializerStorage.size());
        serializerMemory = infra::Head(infra::MakeRange(serializerStorage), size);
        return serializerAccess.MakeShared(serializerMemory);
    }

    template<class... Services>
    template<class... ServiceProxies>
    infra::SharedPtr<infra::ByteRange> MethodSerializerFactory::ForServices<Services...>::AndProxies<ServiceProxies...>::DeserializerMemory(uint32_t size)
    {
        really_assert(size <= deserializerStorage.size());
        deserializerMemory = infra::Head(infra::MakeRange(deserializerStorage), size);
        return deserializerAccess.MakeShared(deserializerMemory);
    }
}

#endif
