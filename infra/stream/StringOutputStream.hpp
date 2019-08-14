#ifndef INFRA_STRING_OUTPUT_STREAM_HPP
#define INFRA_STRING_OUTPUT_STREAM_HPP

#include "infra/stream/StreamManipulators.hpp"
#include "infra/stream/OutputStream.hpp"
#include "infra/util/BoundedString.hpp"
#include <cstdint>

namespace infra
{
    class StringOutputStreamWriter
        : public StreamWriter
    {
    public:
        explicit StringOutputStreamWriter(BoundedString& string);

        template<class T>
            ReservedProxy<T> Reserve(StreamErrorPolicy& errorPolicy);

        void Reset();
        void Reset(BoundedString& newString);

    private:
        virtual void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;
        virtual std::size_t Available() const override;

        virtual std::size_t ConstructSaveMarker() const override;
        virtual std::size_t GetProcessedBytesSince(std::size_t marker) const override;
        virtual infra::ByteRange SaveState(std::size_t marker) override;
        virtual void RestoreState(infra::ByteRange range) override;
        virtual infra::ByteRange Overwrite(std::size_t marker) override;

    private:
        BoundedString* string;
    };

    class StringOutputStream
        : public TextOutputStream::WithWriter<StringOutputStreamWriter>
    {
    public:
        template<std::size_t Max>
            using WithStorage = infra::WithStorage<TextOutputStream::WithWriter<StringOutputStreamWriter>, BoundedString::WithStorage<Max>>;

        StringOutputStream(BoundedString& storage);
        StringOutputStream(BoundedString& storage, const SoftFail&);
        StringOutputStream(BoundedString& storage, const NoFail&);
    };

    ////    Implementation    ////

    template<class T>
    ReservedProxy<T> StringOutputStreamWriter::Reserve(StreamErrorPolicy& errorPolicy)
    {
        ByteRange range(ReinterpretCastByteRange(MemoryRange<char>(string->end(), string->end() + sizeof(T))));
        std::size_t spaceLeft = string->max_size() - string->size();
        bool spaceOk = range.size() <= spaceLeft;
        errorPolicy.ReportResult(spaceOk);
        if (!spaceOk)
            range.shrink_from_back_to(spaceLeft);

        string->append(range.size(), 0);

        return ReservedProxy<T>(range);
    }
}

#endif
