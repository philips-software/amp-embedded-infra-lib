#include "services/util/ConfigurationStore.hpp"

namespace services
{
    ConfigurationBlobFlash::ConfigurationBlobFlash(infra::ByteRange blob, infra::ByteRange verificationBuffer, hal::Flash& flash, services::Sha256& sha256)
        : blob(blob)
        , verificationBuffer(verificationBuffer)
        , flash(flash)
        , sha256(sha256)
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

    void ConfigurationBlobFlash::Write(uint32_t size, const infra::Function<void()>& onDone)
    {
        currentSize = size;
        this->onDone = onDone;
        PrepareBlobForWriting();
        flash.WriteBuffer(blob, 0, [this]() { Verify(); });
    }

    void ConfigurationBlobFlash::Erase(const infra::Function<void()>& onDone)
    {
        flash.EraseAll(onDone);
    }

    infra::ByteRange ConfigurationBlobFlash::Blob()
    {
        return blob;
    }

    infra::ByteRange ConfigurationBlobFlash::VerificationBuffer()
    {
        return verificationBuffer;
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

        infra::MemoryRange input = infra::MemoryRange(blob.begin() + sizeof(header.hash), blob.begin() + std::min<std::size_t>(header.size + sizeof(header.size), blob.size() - sizeof(header.hash)));

        auto messageHash = sha256.Calculate(infra::MakeConstByteRange(input));

        return infra::Head(infra::MakeRange(messageHash), sizeof(header.hash)) == header.hash;
    }

    void ConfigurationBlobFlash::PrepareBlobForWriting()
    {
        Header header;
        header.size = currentSize;
        infra::Copy(infra::MakeByteRange(header), infra::Head(blob, sizeof(header)));

        infra::MemoryRange input = infra::MemoryRange(blob.begin() + sizeof(header.hash), blob.begin() + currentSize + sizeof(header.size));

        auto messageHash = sha256.Calculate(infra::MakeConstByteRange(input));

        infra::Copy(infra::Head(infra::MakeRange(messageHash), sizeof(header.hash)), infra::MakeRange(header.hash));
        infra::Copy(infra::MakeByteRange(header), infra::Head(blob, sizeof(header)));
    }

    void ConfigurationBlobFlash::Verify()
    {
        currentVerificationIndex = 0;

        VerifyBlock();
    }

    void ConfigurationBlobFlash::VerifyBlock()
    {
        if (currentVerificationIndex != blob.size())
            flash.ReadBuffer(infra::Head(verificationBuffer, blob.size() - currentVerificationIndex), currentVerificationIndex, [this]()
            {
                auto verificationBlock = infra::Head(verificationBuffer, blob.size() - currentVerificationIndex);
                really_assert(infra::ContentsEqual(verificationBlock, infra::Head(infra::DiscardHead(blob, currentVerificationIndex), verificationBuffer.size())));
                currentVerificationIndex += verificationBlock.size();
                VerifyBlock();
            });
        else
            onDone();
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

    ConfigurationStoreInterface::LockGuard ConfigurationStoreInterface::Lock()
    {
        return LockGuard(*this);
    }

    bool ConfigurationStoreInterface::IsLocked() const
    {
        return lockCount != 0;
    }

    ConfigurationStoreInterface::LockGuard::LockGuard(ConfigurationStoreInterface& store)
        : store(&store)
    {
        ++store.lockCount;
    }

    ConfigurationStoreInterface::LockGuard::LockGuard(const LockGuard& other)
        : store(other.store)
    {
        ++store->lockCount;
    }

    ConfigurationStoreInterface::LockGuard& ConfigurationStoreInterface::LockGuard::operator=(const LockGuard& other)
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

    ConfigurationStoreInterface::LockGuard::~LockGuard()
    {
        --store->lockCount;
        if (store->lockCount == 0)
            store->Unlocked();
    }

    ConfigurationStoreBase::ConfigurationStoreBase(ConfigurationBlob& blob1, ConfigurationBlob& blob2)
        : activeBlob(&blob1)
        , inactiveBlob(&blob2)
    {}

    uint32_t ConfigurationStoreBase::Write()
    {
        uint32_t thisId = operationId;

        writeRequested = true;
        if (!writingBlob && !IsLocked())
        {
            ++operationId;
            writeRequested = false;
            writingBlob = true;
            Serialize(*activeBlob, [this, thisId]() { inactiveBlob->Erase([this, thisId]() { BlobWriteDone(); NotifyObservers([thisId](ConfigurationStoreObserver& observer) { observer.OperationDone(thisId); }); }); });
        }

        return thisId;
    }

    uint32_t ConfigurationStoreBase::Erase()
    {
        uint32_t thisId = operationId;
        ++operationId;

        inactiveBlob->Erase([this, thisId]() { activeBlob->Erase([this, thisId]() { NotifyObservers([thisId](ConfigurationStoreObserver& observer) { observer.OperationDone(thisId); }); }); });

        return thisId;
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

    void ConfigurationStoreBase::Unlocked()
    {
        if (writeRequested)
            Write();
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
                                eraseOperationId = configurationStore.Erase();
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

    uint32_t FactoryDefaultConfigurationStoreBase::Write()
    {
        return configurationStore.Write();
    }

    void FactoryDefaultConfigurationStoreBase::OperationDone(uint32_t id)
    {
        if (onRecovered != nullptr && eraseOperationId <= id)
            onRecovered(true);

        NotifyObservers([id](ConfigurationStoreObserver& observer) { observer.OperationDone(id); });
    }
}
