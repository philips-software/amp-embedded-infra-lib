#ifndef INFRA_BYTE_OUTPUT_STREAM_HPP
#define INFRA_BYTE_OUTPUT_STREAM_HPP

// With a ByteOutputStreamWriter, you can easily place all sorts of objects into a block of memory.
// A ByteOutputStreamWriter is created with a MemoryRange as argument, objects streamed out of the
// ByteOutputStreamWriter are taken from that range.
//
// Example:
//
// uint16_t myData = 0x1234;
// uint16_t myChecksum = 0x5678;
//
// std::array<uint8_t, 20> memory;
// infra::ByteOutputStreamWriter writeStream(memory);
// writeStream << uint8_t(4) << myData << myChecksum;
//
// Now memory contains the bytes 0x04, 0x12, 0x34, 0x56, 0x78.

#include "infra/stream/OutputStream.hpp"
#include "infra/util/WithStorage.hpp"

namespace infra
{
    class ByteOutputStreamWriter
        : public StreamWriter
    {
    public:
        template<std::size_t Max>
        using WithStorage = infra::WithStorage<ByteOutputStreamWriter, std::array<uint8_t, Max>>;

        explicit ByteOutputStreamWriter(ByteRange range);

        ByteRange Processed() const; // Invariant: Processed() ++ Remaining() == range
        ByteRange Remaining() const;

        void Reset();
        void Reset(ByteRange range);

        template<class T>
        ReservedProxy<T> Reserve(StreamErrorPolicy& errorPolicy);

    protected:
        virtual void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;

    private:
        std::size_t Available() const override;

        virtual std::size_t ConstructSaveMarker() const override;
        virtual std::size_t GetProcessedBytesSince(std::size_t marker) const override;
        virtual infra::ByteRange SaveState(std::size_t marker) override;
        virtual void RestoreState(infra::ByteRange range) override;
        virtual infra::ByteRange Overwrite(std::size_t marker) override;

    private:
        ByteRange streamRange;
        std::size_t offset = 0;
    };

    class ByteOutputStream
        : public DataOutputStream::WithWriter<ByteOutputStreamWriter>
    {
    public:
        template<std::size_t Max>
        using WithStorage = infra::WithStorage<ByteOutputStream::WithWriter<ByteOutputStreamWriter>, std::array<uint8_t, Max>>;

        ByteOutputStream(ByteRange storage);
        ByteOutputStream(ByteRange storage, const SoftFail&);
        ByteOutputStream(ByteRange storage, const NoFail&);
    };

    ////    Implementation    ////

    template<class T>
    ReservedProxy<T> ByteOutputStreamWriter::Reserve(StreamErrorPolicy& errorPolicy)
    {
        ByteRange reservedRange(streamRange.begin() + offset, streamRange.begin() + offset + sizeof(T));
        std::size_t spaceLeft = streamRange.size() - offset;
        bool spaceOk = reservedRange.size() <= spaceLeft;
        errorPolicy.ReportResult(spaceOk);
        if (!spaceOk)
            reservedRange.shrink_from_back_to(spaceLeft);

        offset += reservedRange.size();

        return ReservedProxy<T>(reservedRange);
    }
}

#endif
