#include "mbedtls/sha256.h"
#include "services/util/ConfigurationStore.hpp"

namespace services
{
    ConfigurationBlobFlash::ConfigurationBlobFlash(infra::ByteRange blob, hal::Flash& flash)
        : blob(blob)
        , flash(flash)
    {
        really_assert(blob.size() <= flash.TotalSize());
    }

    infra::ConstByteRange ConfigurationBlobFlash::CurrentBlob()
    {
        return infra::Head(infra::DiscardHead(blob, sizeof(Header)), currentSize);
    }

    infra::ByteRange ConfigurationBlobFlash::MaxBlob()
    {
        return infra::DiscardHead(blob, sizeof(Header));
    }

    void ConfigurationBlobFlash::Recover(const infra::Function<void(bool success)>& onRecovered)
    {
        this->onRecovered = onRecovered;
        flash.ReadBuffer(blob, 0, [this]()
        {
            if (BlobIsValid())
            {
                RecoverCurrentSize();
                this->onRecovered(true);
            }
            else
                this->onRecovered(false);
        });
    }

    void ConfigurationBlobFlash::Erase(const infra::Function<void()>& onDone)
    {
        flash.EraseAll(onDone);
    }

    void ConfigurationBlobFlash::Write(uint32_t size, const infra::Function<void()>& onDone)
    {
        currentSize = size;
        PrepareBlobForWriting();
        flash.WriteBuffer(blob, 0, onDone);
    }

    void ConfigurationBlobFlash::RecoverCurrentSize()
    {
        Header header;
        infra::Copy(infra::Head(blob, sizeof(header)), infra::MakeByteRange(header));

        currentSize = header.size;
    }

    bool ConfigurationBlobFlash::BlobIsValid() const
    {
        Header header;
        infra::Copy(infra::Head(blob, sizeof(header)), infra::MakeByteRange(header));

        if (header.size + sizeof(Header) > blob.size())
            return false;

        std::array<uint8_t, 32> messageHash;
        mbedtls_sha256(blob.begin() + sizeof(header.hash), std::min<std::size_t>(header.size + sizeof(header.size), blob.size() - sizeof(header.hash)), messageHash.data(), 0);  //TICS !INT#030

        return infra::Head(infra::MakeRange(messageHash), sizeof(header.hash)) == header.hash;
    }

    void ConfigurationBlobFlash::PrepareBlobForWriting()
    {
        Header header;
        header.size = currentSize;
        infra::Copy(infra::MakeByteRange(header), infra::Head(blob, sizeof(header)));

        std::array<uint8_t, 32> messageHash;
        mbedtls_sha256(blob.begin() + sizeof(header.hash), currentSize + sizeof(header.size), messageHash.data(), 0);  //TICS !INT#030

        infra::Copy(infra::Head(infra::MakeRange(messageHash), sizeof(header.hash)), infra::MakeRange(header.hash));
        infra::Copy(infra::MakeByteRange(header), infra::Head(blob, sizeof(header)));
    }

    ConfigurationBlobReadOnlyMemory::ConfigurationBlobReadOnlyMemory(infra::ConstByteRange data)
        : data(data)
    {}

    infra::ConstByteRange ConfigurationBlobReadOnlyMemory::CurrentBlob()
    {
        Header header;
        infra::Copy(infra::Head(data, sizeof(Header)), infra::MakeByteRange(header));

        return infra::Head(infra::DiscardHead(data, sizeof(Header)), header.size);
    }

    infra::ByteRange ConfigurationBlobReadOnlyMemory::MaxBlob()
    {
        std::abort();
    }

    void ConfigurationBlobReadOnlyMemory::Recover(const infra::Function<void(bool success)>& onRecovered)
    {
        onRecovered(true);
    }

    void ConfigurationBlobReadOnlyMemory::Write(uint32_t size, const infra::Function<void()>& onDone)
    {
        std::abort();
    }

    void ConfigurationBlobReadOnlyMemory::Erase(const infra::Function<void()>& onDone)
    {
        std::abort();
    }

    ConfigurationStoreBase::ConfigurationStoreBase(ConfigurationBlob& blob1, ConfigurationBlob& blob2)
        : activeBlob(&blob1)
        , inactiveBlob(&blob2)
    {}

    void ConfigurationStoreBase::Write(infra::Function<void()> onDone)
    {
        if (onDone)
            onWriteDone = onDone;

        writeRequested = true;
        if (!writingBlob && lockCount == 0)
        {
            writeRequested = false;
            writingBlob = true;
            Serialize(*activeBlob, [this]() { inactiveBlob->Erase([this]() { BlobWriteDone(); }); });
        }
    }

    void ConfigurationStoreBase::Write()
    {
        Write(infra::Function<void()>());
    }

    void ConfigurationStoreBase::Erase(infra::Function<void()> onDone)
    {
        onWriteDone = onDone;
        inactiveBlob->Erase([this]() { activeBlob->Erase([this]() { onWriteDone(); }); });
    }

    ConfigurationStoreBase::LockGuard ConfigurationStoreBase::Lock()
    {
        return LockGuard(*this);
    }

    void ConfigurationStoreBase::Recover(const infra::Function<void(bool success)>& onRecovered)
    {
        this->onRecovered = onRecovered;

        activeBlob->Recover([this](bool success)
        {
            if (success)
            {
                inactiveBlob->Erase([this]() { OnBlobLoaded(true); });
            }
            else
            {
                std::swap(activeBlob, inactiveBlob);
                activeBlob->Recover([this](bool success)
                {
                    inactiveBlob->Erase([this, success]() { OnBlobLoaded(success); });
                });
            }
        });
    }

    void ConfigurationStoreBase::OnBlobLoaded(bool success)
    {
        std::swap(activeBlob, inactiveBlob);

        if (success)
            Deserialize(*inactiveBlob);

        onRecovered(success);
    }

    void ConfigurationStoreBase::BlobWriteDone()
    {
        std::swap(activeBlob, inactiveBlob);
        writingBlob = false;
        if (writeRequested)
            Write();
        else if (onWriteDone)
            onWriteDone();
    }

    void ConfigurationStoreBase::Unlocked()
    {
        if (writeRequested)
            Write();
    }

    ConfigurationStoreBase::LockGuard::LockGuard(ConfigurationStoreBase& store)
        : store(&store)
    {
        ++store.lockCount;
    }

    ConfigurationStoreBase::LockGuard::LockGuard(const LockGuard& other)
        : store(other.store)
    {
        ++store->lockCount;
    }

    ConfigurationStoreBase::LockGuard& ConfigurationStoreBase::LockGuard::operator=(const LockGuard& other)
    {
        if (this != &other)
        {
            --store->lockCount;
            if (store->lockCount == 0)
                store->Unlocked();

            store = other.store;
            ++store->lockCount;
        }

        return *this;
    }

    ConfigurationStoreBase::LockGuard::~LockGuard()
    {
        --store->lockCount;
        if (store->lockCount == 0)
            store->Unlocked();
    }

    FactoryDefaultConfigurationStoreBase::FactoryDefaultConfigurationStoreBase(ConfigurationStoreBase& configurationStore, ConfigurationBlob& factoryDefaultBlob)
        : configurationStore(configurationStore)
        , factoryDefaultBlob(factoryDefaultBlob)
    {}

    void FactoryDefaultConfigurationStoreBase::Recover(const infra::Function<void()>& onLoadFactoryDefault, const infra::Function<void(bool isFactoryDefault)>& onRecovered)
    {
        this->onLoadFactoryDefault = onLoadFactoryDefault;
        this->onRecovered = onRecovered;

        factoryDefaultBlob.Recover([this](bool success)
        {
            if (!success)
            {
                this->onLoadFactoryDefault();

                factoryDefaultBlob.Erase([this]()
                {
                    configurationStore.Serialize(factoryDefaultBlob, [this]()
                    {
                        configurationStore.Recover([this](bool success)
                        {
                            if (success)
                                this->onRecovered(false);
                            else
                                configurationStore.Erase([this]() { this->onRecovered(true); });
                        });
                    });
                });
            }
            else
            {
                configurationStore.Deserialize(factoryDefaultBlob);

                this->onLoadFactoryDefault = nullptr;

                configurationStore.Recover([this](bool success)
                {
                    this->onRecovered(!success);
                });
            }
        });
    }

    void FactoryDefaultConfigurationStoreBase::Write(infra::Function<void()> onDone)
    {
        configurationStore.Write(onDone);
    }

    void FactoryDefaultConfigurationStoreBase::Write()
    {
        Write(infra::Function<void()>());
    }
}
