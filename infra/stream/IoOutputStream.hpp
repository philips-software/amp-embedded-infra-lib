#ifndef INFRA_IO_OUTPUT_STREAM_HPP
#define INFRA_IO_OUTPUT_STREAM_HPP

#include "infra/stream/StreamManipulators.hpp"
#include "infra/stream/OutputStream.hpp"
#include "infra/util/BoundedString.hpp"
#include <cstdint>

namespace infra
{
    class IoOutputStreamWriter
        : public StreamWriter
    {
    private:
        virtual void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;
        virtual std::size_t Available() const override;
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
