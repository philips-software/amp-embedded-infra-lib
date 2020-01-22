#ifndef SERVICES_MEMORY_MAPPED_STATIC_CONFIGURATION_HPP
#define SERVICES_MEMORY_MAPPED_STATIC_CONFIGURATION_HPP

#include "hal/interfaces/Flash.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/syntax/ProtoFormatter.hpp"
#include "infra/syntax/ProtoParser.hpp"
#include "mbedtls/sha256.h"
#include <memory>
#include "infra/event/EventDispatcher.hpp"

namespace services
{
    template<class T>
    class WritableConfigurationWriter
    {
    public:
        virtual void Write(const T& newValue, const infra::Function<void()>& onDone) = 0;
    };

    template<class TRef>
    class WritableConfigurationReader
    {
    public:
        virtual bool Valid() const = 0;
        virtual const TRef& Get() const = 0;
        virtual void Read(const infra::Function<void()>& onDone) = 0;
    };

    template<class T, class TRef>
    class MemoryMappedWritableConfiguration
        : public WritableConfigurationWriter<T>
        , public WritableConfigurationReader<TRef>
    {
    public:
        MemoryMappedWritableConfiguration(hal::Flash& flash, infra::ConstByteRange flashRegion);

        virtual bool Valid() const override;
        virtual const TRef& Get() const override;
        virtual void Read(const infra::Function<void()>& onDone) override;

        // Warning: Allocates memory on the heap!
        virtual void Write(const T& newValue, const infra::Function<void()>& onDone) override;

    protected:
        void LoadConfiguration();

    protected:
        struct Header
        {
            std::array<uint8_t, 8> hash;
            uint32_t size;
        };

    protected:
        hal::Flash& flash;
        infra::ConstByteRange flashRegion;
        TRef value;
        bool valid = false;
        std::shared_ptr<infra::DataOutputStream::WithWriter<infra::ByteOutputStreamWriter>> stream;
        infra::Function<void()> onDone;
    };

    template<class T, class TRef>
    class WritableConfiguration
        : public MemoryMappedWritableConfiguration<T, TRef>
    {
    public:
        WritableConfiguration(hal::Flash& flash, infra::ByteRange flashRegion);

        virtual void Read(const infra::Function<void()>& onDone) override;
    };

    ////    Implementation    ////

    template<class T, class TRef>
    WritableConfiguration<T, TRef>::WritableConfiguration(hal::Flash& flash, infra::ByteRange flashRegion)
        : MemoryMappedWritableConfiguration<T, TRef>(flash, flashRegion)
    {}

    template<class T, class TRef>
    void WritableConfiguration<T, TRef>::Read(const infra::Function<void()>& onDone)
    {
        this->onDone = onDone;
        flash.ReadBuffer(infra::ConstCastByteRange(flashRegion), 0, [this]()
        {
            LoadConfiguration();
            this->onDone();
        });
    }

    template<class T, class TRef>
    MemoryMappedWritableConfiguration<T, TRef>::MemoryMappedWritableConfiguration(hal::Flash& flash, infra::ConstByteRange flashRegion)
        : flash(flash)
        , flashRegion(flashRegion)
    {
        LoadConfiguration();
    }

    template<class T, class TRef>
    bool MemoryMappedWritableConfiguration<T, TRef>::Valid() const
    {
        return valid;
    }

    template<class T, class TRef>
    const TRef& MemoryMappedWritableConfiguration<T, TRef>::Get() const
    {
        return value;
    }

    template<class T, class TRef>
    void MemoryMappedWritableConfiguration<T, TRef>::Write(const T& newValue, const infra::Function<void()>& onDone)
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
        flash.EraseAll([this]() { flash.WriteBuffer(this->stream->Writer().Processed(), 0, [this]() { this->stream = nullptr; Read(this->onDone); }); });
    }

    template<class T, class TRef>
    void MemoryMappedWritableConfiguration<T, TRef>::Read(const infra::Function<void()>& onDone)
    {
        this->onDone = onDone;
        infra::EventDispatcher::Instance().Schedule([this]()
        {
            LoadConfiguration();
            this->onDone();
        });
    }

    template <class T, class TRef>
    void MemoryMappedWritableConfiguration<T, TRef>::LoadConfiguration()
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
