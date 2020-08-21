#ifndef SERVICES_CONFIGURATION_STORE_HPP
#define SERVICES_CONFIGURATION_STORE_HPP

#include "hal/interfaces/Flash.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/syntax/ProtoFormatter.hpp"
#include "infra/syntax/ProtoParser.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/Observer.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "infra/util/WithStorage.hpp"

namespace services
{
    class ConfigurationBlob
    {
    protected:
        ConfigurationBlob() = default;
        ConfigurationBlob(const ConfigurationBlob& other) = delete;
        ConfigurationBlob& operator=(const ConfigurationBlob& other) = delete;
        ~ConfigurationBlob() = default;

    public:
        virtual infra::ConstByteRange CurrentBlob() = 0;
        virtual infra::ByteRange MaxBlob() = 0;
        virtual void Recover(const infra::Function<void(bool success)>& onRecovered) = 0;
        virtual void Write(uint32_t size, const infra::Function<void()>& onDone) = 0;
        virtual void Erase(const infra::Function<void()>& onDone) = 0;
    };

    class ConfigurationBlobFlash
        : public ConfigurationBlob
    {
    private:
        struct Header
        {
            std::array<uint8_t, 8> hash;
            uint32_t size;
        };

    public:
        template<std::size_t Size, std::size_t VerificationSize = 256>
            using WithStorage = infra::WithStorage<infra::WithStorage<ConfigurationBlobFlash,
                std::array<uint8_t, Size + sizeof(Header)>>,
                std::array<uint8_t, VerificationSize>>;

        ConfigurationBlobFlash(infra::ByteRange blob, infra::ByteRange verificationBuffer, hal::Flash& flash);

        virtual infra::ConstByteRange CurrentBlob() override;
        virtual infra::ByteRange MaxBlob() override;
        virtual void Recover(const infra::Function<void(bool success)>& onRecovered) override;
        virtual void Write(uint32_t size, const infra::Function<void()>& onDone) override;
        virtual void Erase(const infra::Function<void()>& onDone) override;

        infra::ByteRange Blob();
        infra::ByteRange VerificationBuffer();

    private:
        void RecoverCurrentSize();
        bool BlobIsValid() const;
        void PrepareBlobForWriting();
        void Verify();
        void VerifyBlock();

    private:
        infra::ByteRange blob;
        infra::ByteRange verificationBuffer;
        hal::Flash& flash;
        uint32_t currentSize = 0;
        uint32_t currentVerificationIndex = 0;
        infra::AutoResetFunction<void(bool success)> onRecovered;
        infra::AutoResetFunction<void()> onDone;
    };

    class ConfigurationBlobReadOnlyMemory
        : public ConfigurationBlob
    {
    private:
        struct Header
        {
            uint32_t size;
        };

    public:
        ConfigurationBlobReadOnlyMemory(infra::ConstByteRange data);

        virtual infra::ConstByteRange CurrentBlob() override;
        virtual infra::ByteRange MaxBlob() override;
        virtual void Recover(const infra::Function<void(bool success)>& onRecovered) override;
        virtual void Write(uint32_t size, const infra::Function<void()>& onDone) override;
        virtual void Erase(const infra::Function<void()>& onDone) override;

    private:
        infra::ConstByteRange data;
    };

    class ConfigurationStoreInterface;

    class ConfigurationStoreObserver
        : public infra::Observer<ConfigurationStoreObserver, ConfigurationStoreInterface>
    {
    public:
        using infra::Observer<ConfigurationStoreObserver, ConfigurationStoreInterface>::Observer;

        virtual void OperationDone(uint32_t id) = 0;
    };

    class ConfigurationStoreInterface
        : public infra::Subject<ConfigurationStoreObserver>
    {
    protected:
        ConfigurationStoreInterface() = default;
        ConfigurationStoreInterface(const ConfigurationStoreInterface& other) = delete;
        ConfigurationStoreInterface& operator=(const ConfigurationStoreInterface& other) = delete;
        ~ConfigurationStoreInterface() = default;

    public:
        virtual uint32_t Write() = 0;
    };

    template<class T>
    class ConfigurationStoreAccess
    {
    public:
        ConfigurationStoreAccess(ConfigurationStoreInterface& configurationStore, T& configuration);
        template<class U>
            ConfigurationStoreAccess(const ConfigurationStoreAccess<U>& other);

        T& operator*();
        const T& operator*() const;
        T* operator->();
        const T* operator->() const;

        uint32_t Write();

        template<class U>
            ConfigurationStoreAccess<U> Configuration(U& member) const;

        ConfigurationStoreInterface& Interface();

    private:
        template<class U>
            friend class ConfigurationStoreAccess;

        ConfigurationStoreInterface& configurationStore;
        T& configuration;
    };

    class ConfigurationStoreBase
        : public ConfigurationStoreInterface
    {
    public:
        ConfigurationStoreBase(ConfigurationBlob& blob1, ConfigurationBlob& blob2);

    public:
        void Recover(const infra::Function<void(bool success)>& onRecovered);
        virtual uint32_t Write() override;
        uint32_t Erase();

        virtual void Serialize(ConfigurationBlob& blob, const infra::Function<void()>& onDone) = 0;
        virtual void Deserialize(ConfigurationBlob& blob) = 0;

        class LockGuard
        {
        public:
            LockGuard(ConfigurationStoreBase& store);
            LockGuard(const LockGuard& other);
            LockGuard& operator=(const LockGuard& other);
            ~LockGuard();

        private:
            ConfigurationStoreBase* store;
        };

        LockGuard Lock();

        template<class T>
            ConfigurationStoreAccess<T> Access(T& configuration);

    private:
        void OnBlobLoaded(bool success);
        void BlobWriteDone();
        void Unlocked();

    private:
        ConfigurationBlob* activeBlob;
        ConfigurationBlob* inactiveBlob;
        infra::AutoResetFunction<void(bool success)> onRecovered;
        uint32_t lockCount = 0;
        uint32_t operationId = 0;
        bool writingBlob = false;
        bool writeRequested = false;
    };

    template<class T>
    class ConfigurationStore
    {
    public:
        virtual const T& Configuration() const = 0;
        virtual T& Configuration() = 0;
        virtual ConfigurationStoreInterface& Interface() = 0;

        virtual uint32_t Write() = 0;
        virtual ConfigurationStoreBase::LockGuard Lock() = 0;
    };

    template<class T>
    class ConfigurationStoreImpl
        : public ConfigurationStoreBase
        , public ConfigurationStore<T>
    {
    public:
        template<std::size_t VerificationSize = 256>
            class WithBlobs;

        ConfigurationStoreImpl(ConfigurationBlob& blob1, ConfigurationBlob& blob2);

        virtual const T& Configuration() const override;
        virtual T& Configuration() override;
        virtual ConfigurationStoreInterface& Interface() override;
        virtual uint32_t Write() override;
        virtual ConfigurationStoreBase::LockGuard Lock() override;

        virtual void Serialize(ConfigurationBlob& blob, const infra::Function<void()>& onDone) override;
        virtual void Deserialize(ConfigurationBlob& blob) override;

    private:
        T configuration;
    };

    template<class T>
    template<std::size_t VerificationSize>
    class ConfigurationStoreImpl<T>::WithBlobs
        : public ConfigurationStoreImpl<T>
    {
    public:
        WithBlobs(hal::Flash& flashFirst, hal::Flash& flashSecond, const infra::Function<void(bool success)>& onRecovered);

    private:
        typename ConfigurationBlobFlash::WithStorage<T::maxMessageSize, VerificationSize> blob1;
        ConfigurationBlobFlash blob2;
    };

    class FactoryDefaultConfigurationStoreBase
        : public ConfigurationStoreInterface
        , private ConfigurationStoreObserver
    {
    public:
        FactoryDefaultConfigurationStoreBase(ConfigurationStoreBase& configurationStore, ConfigurationBlob& factoryDefaultBlob);

        void Recover(const infra::Function<void()>& onLoadFactoryDefault, const infra::Function<void(bool isFactoryDefault)>& onRecovered);
        virtual uint32_t Write() override;

        template<class T>
            ConfigurationStoreAccess<T> Access(T& configuration);

    private:
        virtual void OperationDone(uint32_t id) override;

    private:
        ConfigurationStoreBase& configurationStore;
        ConfigurationBlob& factoryDefaultBlob;
        infra::AutoResetFunction<void()> onLoadFactoryDefault;
        infra::AutoResetFunction<void(bool isFactoryDefault)> onRecovered;
        uint32_t eraseOperationId = 0;
    };

    template<class T>
    class FactoryDefaultConfigurationStore
        : public FactoryDefaultConfigurationStoreBase
        , public ConfigurationStore<T>
    {
    public:
        template<std::size_t VerificationSize = 256>
            class WithBlobs;
        template<std::size_t VerificationSize = 256>
            class WithReadOnlyDefaultAndBlobs;

        FactoryDefaultConfigurationStore(ConfigurationBlob& blobFactoryDefault, ConfigurationBlob& blob1, ConfigurationBlob& blob2);

        virtual const T& Configuration() const override;
        virtual T& Configuration() override;
        virtual ConfigurationStoreInterface& Interface() override;
        virtual uint32_t Write() override;
        virtual ConfigurationStoreBase::LockGuard Lock() override;

    private:
        ConfigurationStoreImpl<T> configurationStore;
    };

    template<class T>
    template<std::size_t VerificationSize>
    class FactoryDefaultConfigurationStore<T>::WithBlobs
        : public FactoryDefaultConfigurationStore<T>
    {
    public:
        WithBlobs(hal::Flash& flashFactoryDefault, hal::Flash& flashFirst, hal::Flash& flashSecond
            , const infra::Function<void()>& onLoadFactoryDefault, const infra::Function<void(bool isFactoryDefault)>& onRecovered);

    private:
        ConfigurationBlobFlash::WithStorage<T::maxMessageSize, VerificationSize> factoryDefaultBlob;
        ConfigurationBlobFlash blob1;
        ConfigurationBlobFlash blob2;
    };

    template<class T>
    template<std::size_t VerificationSize>
    class FactoryDefaultConfigurationStore<T>::WithReadOnlyDefaultAndBlobs
        : public FactoryDefaultConfigurationStore<T>
    {
    public:
        WithReadOnlyDefaultAndBlobs(infra::ConstByteRange factoryDefault, hal::Flash& flashFirst, hal::Flash& flashSecond
            , const infra::Function<void(bool isFactoryDefault)>& onRecovered);

    private:
        ConfigurationBlobReadOnlyMemory factoryDefaultBlob;
        ConfigurationBlobFlash::WithStorage<T::maxMessageSize, VerificationSize> blob1;
        ConfigurationBlobFlash blob2;
    };

    ////    Implementation    ////

    template<class T>
    ConfigurationStoreImpl<T>::ConfigurationStoreImpl(ConfigurationBlob& blob1, ConfigurationBlob& blob2)
        : ConfigurationStoreBase(blob1, blob2)
    {}

    template<class T>
    const T& ConfigurationStoreImpl<T>::Configuration() const
    {
        return configuration;
    }

    template<class T>
    T& ConfigurationStoreImpl<T>::Configuration()
    {
        return configuration;
    }

    template<class T>
    ConfigurationStoreInterface& ConfigurationStoreImpl<T>::Interface()
    {
        return *this;
    }

    template<class T>
    uint32_t ConfigurationStoreImpl<T>::Write()
    {
        return ConfigurationStoreBase::Write();
    }

    template<class T>
    ConfigurationStoreBase::LockGuard ConfigurationStoreImpl<T>::Lock()
    {
        return ConfigurationStoreBase::Lock();
    }

    template<class T>
    void ConfigurationStoreImpl<T>::Serialize(ConfigurationBlob& blob, const infra::Function<void()>& onDone)
    {
        infra::ByteOutputStream stream(blob.MaxBlob());
        infra::ProtoFormatter formatter(stream);
        configuration.Serialize(formatter);
        std::fill(stream.Writer().Remaining().begin(), stream.Writer().Remaining().end(), 0xff);
        blob.Write(stream.Writer().Processed().size(), onDone);
    }

    template<class T>
    void ConfigurationStoreImpl<T>::Deserialize(ConfigurationBlob& blob)
    {
        infra::ByteInputStream stream(blob.CurrentBlob());
        infra::ProtoParser parser(stream);

        infra::ReConstruct(configuration);

        configuration.Deserialize(parser);
    }

    template<class T>
    template<std::size_t VerificationSize>
    ConfigurationStoreImpl<T>::WithBlobs<VerificationSize>::WithBlobs(hal::Flash& flashFirst, hal::Flash& flashSecond, const infra::Function<void(bool success)>& onRecovered)
        : ConfigurationStoreImpl<T>(blob1, blob2)
        , blob1(flashFirst)
        , blob2(blob1.Blob(), blob1.VerificationBuffer(), flashSecond)
    {
        Recover(onRecovered);
    }

    template<class T>
    ConfigurationStoreAccess<T>::ConfigurationStoreAccess(ConfigurationStoreInterface& configurationStore, T& configuration)
        : configurationStore(configurationStore)
        , configuration(configuration)
    {}

    template<class T>
    template<class U>
    ConfigurationStoreAccess<T>::ConfigurationStoreAccess(const ConfigurationStoreAccess<U>& other)
        : configurationStore(other.configurationStore)
        , configuration(other.configuration)
    {}

    template<class T>
    T& ConfigurationStoreAccess<T>::operator*()
    {
        return configuration;
    }

    template<class T>
    const T& ConfigurationStoreAccess<T>::operator*() const
    {
        return configuration;
    }

    template<class T>
    T* ConfigurationStoreAccess<T>::operator->()
    {
        return &configuration;
    }

    template<class T>
    const T* ConfigurationStoreAccess<T>::operator->() const
    {
        return &configuration;
    }

    template<class T>
    uint32_t ConfigurationStoreAccess<T>::Write()
    {
        return configurationStore.Write();
    }

    template<class T>
    ConfigurationStoreInterface& ConfigurationStoreAccess<T>::Interface()
    {
        return configurationStore;
    }

    template<class T>
    template<class U>
    ConfigurationStoreAccess<U> ConfigurationStoreAccess<T>::Configuration(U& member) const
    {
        return ConfigurationStoreAccess<U>(configurationStore, member);
    }

    template<class T>
    ConfigurationStoreAccess<T> ConfigurationStoreBase::Access(T& configuration)
    {
        return ConfigurationStoreAccess<T>(*this, configuration);
    }

    template<class T>
    ConfigurationStoreAccess<T> FactoryDefaultConfigurationStoreBase::Access(T& configuration)
    {
        return ConfigurationStoreAccess<T>(*this, configuration);
    }

    template<class T>
    FactoryDefaultConfigurationStore<T>::FactoryDefaultConfigurationStore(ConfigurationBlob& blobFactoryDefault, ConfigurationBlob& blob1, ConfigurationBlob& blob2)
        : FactoryDefaultConfigurationStoreBase(configurationStore, blobFactoryDefault)
        , configurationStore(blob1, blob2)
    {}

    template<class T>
    const T& FactoryDefaultConfigurationStore<T>::Configuration() const
    {
        return configurationStore.Configuration();
    }

    template<class T>
    T& FactoryDefaultConfigurationStore<T>::Configuration()
    {
        return configurationStore.Configuration();
    }

    template<class T>
    ConfigurationStoreInterface& FactoryDefaultConfigurationStore<T>::Interface()
    {
        return configurationStore;  // Identifiers returned by Write() are maintained by configurationStore, so that is the object that observers must attach to
    }

    template<class T>
    uint32_t FactoryDefaultConfigurationStore<T>::Write()
    {
        return FactoryDefaultConfigurationStoreBase::Write();
    }

    template<class T>
    ConfigurationStoreBase::LockGuard FactoryDefaultConfigurationStore<T>::Lock()
    {
        return configurationStore.Lock();
    }

    template<class T>
    template<std::size_t VerificationSize>
    FactoryDefaultConfigurationStore<T>::WithBlobs<VerificationSize>::WithBlobs(hal::Flash& flashFactoryDefault, hal::Flash& flashFirst, hal::Flash& flashSecond
        , const infra::Function<void()>& onLoadFactoryDefault, const infra::Function<void(bool isFactoryDefault)>& onRecovered)
        : FactoryDefaultConfigurationStore(factoryDefaultBlob, blob1, blob2)
        , factoryDefaultBlob(flashFactoryDefault)
        , blob1(factoryDefaultBlob.Blob(), factoryDefaultBlob.VerificationBuffer(), flashFirst)
        , blob2(factoryDefaultBlob.Blob(), factoryDefaultBlob.VerificationBuffer(), flashSecond)
    {
        Recover(onLoadFactoryDefault, onRecovered);
    }

    template<class T>
    template<std::size_t VerificationSize>
    FactoryDefaultConfigurationStore<T>::WithReadOnlyDefaultAndBlobs<VerificationSize>::WithReadOnlyDefaultAndBlobs(infra::ConstByteRange factoryDefault, hal::Flash& flashFirst, hal::Flash& flashSecond
        , const infra::Function<void(bool isFactoryDefault)>& onRecovered)
        : FactoryDefaultConfigurationStore(factoryDefaultBlob, blob1, blob2)
        , factoryDefaultBlob(factoryDefault)
        , blob1(flashFirst)
        , blob2(blob1.Blob(), blob1.VerificationBuffer(), flashSecond)
    {
        Recover([]() { std::abort(); }, onRecovered);
    }
}

#endif
