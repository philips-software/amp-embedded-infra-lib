#ifndef INFRA_IO_OUTPUT_STREAM_HPP
#define INFRA_IO_OUTPUT_STREAM_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/stream/StreamManipulators.hpp"
#include "infra/util/BoundedString.hpp"
#include <cstdint>
#include <iostream>

namespace infra
{
    class IoOutputStreamWriter
        : public StreamWriter
    {
    public:
        IoOutputStreamWriter() = default;
        IoOutputStreamWriter(std::ostream& stream);

        virtual void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;
        virtual std::size_t Available() const override;

    private:
        std::ostream& stream{ std::cout };
    };

    class IoOutputStream
        : public TextOutputStream::WithWriter<IoOutputStreamWriter>
    {
    public:
        template<std::size_t Max>
        using WithStorage = infra::WithStorage<TextOutputStream::WithWriter<IoOutputStreamWriter>, BoundedString::WithStorage<Max>>;

        using TextOutputStream::WithWriter<IoOutputStreamWriter>::WithWriter;
    };
}

#endif
