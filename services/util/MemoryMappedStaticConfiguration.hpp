#ifndef SERVICES_MEMORY_MAPPED_STATIC_CONFIGURATION_HPP
#define SERVICES_MEMORY_MAPPED_STATIC_CONFIGURATION_HPP

#include "hal/interfaces/Flash.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/syntax/ProtoFormatter.hpp"
#include "infra/syntax/ProtoParser.hpp"
#include "mbedtls/sha256.h"
#include <memory>

namespace services
{
    template<class T, class TRef>
    class MemoryMappedStaticConfiguration
    {
    public:
        MemoryMappedStaticConfiguration(hal::Flash& flash, infra::ConstByteRange flashRegion);

        bool Valid() const;
        const TRef& Get() const;

        // Warning: Allocates memory on the heap!
        void Write(const T& newValue, const infra::Function<void()>& onDone);

    private:
        void Read();

    private:
        struct Header
        {
            std::array<uint8_t, 8> hash;
            uint32_t size;
        };

    private:
        hal::Flash& flash;
        infra::ConstByteRange flashRegion;
        TRef value;
        bool valid = false;
        std::shared_ptr<infra::DataOutputStream::WithWriter<infra::ByteOutputStreamWriter>> stream;
        infra::Function<void()> onDone;
    };

    ////    Implementation    ////

    template<class T, class TRef>
    MemoryMappedStaticConfiguration<T, TRef>::MemoryMappedStaticConfiguration(hal::Flash& flash, infra::ConstByteRange flashRegion)
        : flash(flash)
        , flashRegion(flashRegion)
    {
        Read();
    }

    template<class T, class TRef>
    bool MemoryMappedStaticConfiguration<T, TRef>::Valid() const
    {
        return valid;
    }

    template<class T, class TRef>
    const TRef& MemoryMappedStaticConfiguration<T, TRef>::Get() const
    {
        return value;
    }

    template<class T, class TRef>
    void MemoryMappedStaticConfiguration<T, TRef>::Write(const T& newValue, const infra::Function<void()>& onDone)
    {
        this->onDone = onDone;
        auto stream = std::make_shared<infra::ByteOutputStream::WithStorage<T::maxMessageSize + sizeof(Header)>>();
        auto headerProxy = stream->Writer().template Reserve<Header>(stream->ErrorPolicy());
        auto marker = stream->SaveMarker();
        infra::ProtoFormatter formatter(*stream);
        newValue.Serialize(formatter);

        Header header;
        header.size = stream->SaveMarker() - marker;
        headerProxy = header;
        std::array<uint8_t, 32> messageHash;
        mbedtls_sha256(stream->Storage().data() + sizeof(Header::hash), std::min<std::size_t>(header.size + sizeof(Header::size), stream->Storage().size() - sizeof(Header::hash)), messageHash.data(), 0);
        infra::Copy(infra::Head(infra::MakeRange(messageHash), sizeof(Header::hash)), infra::MakeRange(header.hash));
        headerProxy = header;

        this->stream = stream;
        flash.EraseAll([this]() { flash.WriteBuffer(this->stream->Writer().Processed(), 0, [this]() { this->stream = nullptr; Read(); this->onDone(); }); });
    }

    template<class T, class TRef>
    void MemoryMappedStaticConfiguration<T, TRef>::Read()
    {
        infra::ByteInputStream::WithReader stream(flashRegion, infra::noFail);
        auto header = stream.Extract<Header>();

        valid = false;
        if (header.size + sizeof(Header) <= flashRegion.size())
        {
            std::array<uint8_t, 32> messageHash;
            mbedtls_sha256(flashRegion.begin() + sizeof(header.hash), std::min<std::size_t>(header.size + sizeof(header.size), flashRegion.size() - sizeof(header.hash)), messageHash.data(), 0);

            if (infra::Head(infra::MakeRange(messageHash), sizeof(header.hash)) == header.hash)
            {
                infra::LimitedStreamReaderWithRewinding limitedReader(stream.Reader(), header.size);
                infra::DataInputStream limitedStream(limitedReader, stream.ErrorPolicy());
                infra::ProtoParser parser(limitedStream);

                value.~TRef();
                new (&value) TRef(parser);

                valid = !stream.Failed();
            }
        }
    }
}

#endif
