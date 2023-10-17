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

        reader = nullptr;
    }

    void MethodDeserializerDummy::ExecuteMethod()
    {
        echo.ServiceDone();
    }

    bool MethodDeserializerDummy::Failed() const
    {
        return false;
    }

    infra::SharedPtr<infra::ByteRange> MethodDeserializerFactory::OnHeap::DeserializerMemory(uint32_t size)
    {
        memory = reinterpret_cast<uint8_t*>(malloc(size));
        return access.MakeShared(infra::ByteRange{ memory, memory + size });
    }

    void MethodDeserializerFactory::OnHeap::DeAllocate()
    {
        free(memory);
        memory = nullptr;
    }
}
