#ifndef INFRA_BOUNDED_VECTOR_OUTPUT_STREAM_HPP
#define INFRA_BOUNDED_VECTOR_OUTPUT_STREAM_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/util/BoundedVector.hpp"

namespace infra
{
    class BoundedVectorStreamWriter
        : public StreamWriter
    {
    public:
        template<std::size_t Size>
        using WithStorage = infra::WithStorage<BoundedVectorStreamWriter, BoundedVector<uint8_t>::WithMaxSize<Size>>;

        explicit BoundedVectorStreamWriter(BoundedVector<uint8_t>& vector);

        template<class T>
        ReservedProxy<T> Reserve(StreamErrorPolicy& errorPolicy);

        void Reset();
        void Reset(BoundedVector<uint8_t>& newVector);

    private:
        virtual void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;
        virtual std::size_t Available() const override;

        virtual std::size_t ConstructSaveMarker() const override;
        virtual std::size_t GetProcessedBytesSince(std::size_t marker) const override;
        virtual infra::ByteRange SaveState(std::size_t marker) override;
        virtual void RestoreState(infra::ByteRange range) override;
        virtual infra::ByteRange Overwrite(std::size_t marker) override;

    private:
        BoundedVector<uint8_t>* vector;
    };

    class BoundedVectorOutputStream
        : public TextOutputStream::WithWriter<BoundedVectorStreamWriter>
    {
    public:
        template<std::size_t Max>
        using WithStorage = infra::WithStorage<DataOutputStream::WithWriter<BoundedVectorStreamWriter>, BoundedVector<uint8_t>::WithMaxSize<Max>>;

        BoundedVectorOutputStream(BoundedVector<uint8_t>& storage);
        BoundedVectorOutputStream(BoundedVector<uint8_t>& storage, const SoftFail&);
        BoundedVectorOutputStream(BoundedVector<uint8_t>& storage, const NoFail&);
    };

    ////    Implementation    ////

    template<class T>
    ReservedProxy<T> BoundedVectorStreamWriter::Reserve(StreamErrorPolicy& errorPolicy)
    {
        ByteRange range(ByteRange(vector->end(), vector->end() + sizeof(T)));
        std::size_t spaceLeft = vector->max_size() - vector->size();
        bool spaceOk = sizeof(T) <= spaceLeft;
        errorPolicy.ReportResult(spaceOk);

        auto increase = std::min(spaceLeft, sizeof(T));
        vector->insert(vector->end(), increase, 0);

        return ReservedProxy<T>(infra::MakeRange(vector->end() - increase, vector->end()));
    }
}

#endif
