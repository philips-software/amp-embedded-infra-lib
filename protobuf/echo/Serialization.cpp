#include "protobuf/echo/Serialization.hpp"
#include "protobuf/echo/Echo.hpp"

namespace services
{
    MethodDeserializerDummy::MethodDeserializerDummy(Echo& echo)
        : echo(echo)
    {}

    void MethodDeserializerDummy::MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        while (!reader->Empty())
            reader->ExtractContiguousRange(std::numeric_limits<uint32_t>::max());
    }

    void MethodDeserializerDummy::ExecuteMethod()
    {
        echo.ServiceDone();
    }

    bool MethodDeserializerDummy::Failed() const
    {
        return false;
    }

    infra::SharedPtr<MethodDeserializer> MethodSerializerFactory::MakeDummyDeserializer(Echo& echo)
    {
        using Deserializer = MethodDeserializerDummy;

        auto memory = DeserializerMemory(sizeof(Deserializer));
        auto deserializer = new (memory->begin()) Deserializer(echo);
        return infra::MakeContainedSharedObject(*deserializer, memory);
    }

    infra::SharedPtr<infra::ByteRange> MethodSerializerFactory::OnHeap::SerializerMemory(uint32_t size)
    {
        auto m = new uint8_t[size];
        serializerMemory = { m,
            m + size };
        return serializerAccess.MakeShared(serializerMemory);
    }

    infra::SharedPtr<infra::ByteRange> MethodSerializerFactory::OnHeap::DeserializerMemory(uint32_t size)
    {
        auto m = new uint8_t[size];
        deserializerMemory = { m,
            m + size };
        return deserializerAccess.MakeShared(deserializerMemory);
    }

    void MethodSerializerFactory::OnHeap::DeAllocateSerializer()
    {
        delete[] serializerMemory.begin();
        serializerMemory = {};
    }

    void MethodSerializerFactory::OnHeap::DeAllocateDeserializer()
    {
        delete[] deserializerMemory.begin();
        deserializerMemory = {};
    }
}
