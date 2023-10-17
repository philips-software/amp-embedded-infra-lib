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
        auto m = reinterpret_cast<uint8_t*>(malloc(size));
        memory = { m,
            m + size };
        return access.MakeShared(memory);
    }

    void MethodDeserializerFactory::OnHeap::DeAllocate()
    {
        free(memory.begin());
        memory = {};
    }
}
