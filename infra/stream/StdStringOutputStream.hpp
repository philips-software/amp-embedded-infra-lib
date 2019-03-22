#ifndef INFRA_STD_STRING_OUTPUT_STREAM_HPP
#define INFRA_STD_STRING_OUTPUT_STREAM_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/util/WithStorage.hpp"
#include <cstdint>
#include <string>

namespace infra
{
    class StdStringOutputStreamWriter
        : public StreamWriter
    {
    public:
        explicit StdStringOutputStreamWriter(std::string& string);

    private:
        virtual void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;
        virtual std::size_t Available() const override;

    private:
        std::string& string;
    };

    class StdStringOutputStream
        : public TextOutputStream::WithWriter<StdStringOutputStreamWriter>
    {
    public:
        using WithStorage = infra::WithStorage<TextOutputStream::WithWriter<StdStringOutputStreamWriter>, std::string>;

        using TextOutputStream::WithWriter<StdStringOutputStreamWriter>::WithWriter;
    };
}

#endif
