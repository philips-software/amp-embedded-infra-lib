#ifndef SERVICES_MBED_TLS_SESSION_HPP
#define SERVICES_MBED_TLS_SESSION_HPP

#include "generated/echo/Network.pb.hpp"
#include "infra/util/BoundedList.hpp"
#include "infra/util/Function.hpp"
#include "mbedtls/ssl.h"
#include "services/network/Address.hpp"
#include "services/util/ConfigurationStore.hpp"
#include <memory>

namespace services
{
    class MbedTlsSessionStorageRam;
    class MbedTlsSessionStoragePersistent;
    class MbedTlsSessionStorageRam;

    class MbedTlsSession
    {
    public:
        explicit MbedTlsSession(uint32_t identifier);
        MbedTlsSession(uint32_t identifier, infra::BoundedConstString hostname);
        MbedTlsSession(uint32_t identifier, IPAddress address);
        explicit MbedTlsSession(network::MbedTlsPersistedSession&);
        virtual ~MbedTlsSession();

        static void Serialize(MbedTlsSession& in, network::MbedTlsPersistedSession& out);
        static void Deserialize(network::MbedTlsPersistedSession& in, MbedTlsSession& out);

        virtual void Reinitialize();
        virtual void Obtained();
        virtual bool IsObtained();
        virtual int SetSession(mbedtls_ssl_context* context);
        virtual int GetSession(mbedtls_ssl_context* context);
        virtual IPAddress& Address();
        virtual infra::BoundedConstString Hostname() const;
        virtual uint32_t Identifier() const;

    private:
        friend class MbedTlsSessionStorageRam;
        friend class MbedTlsSessionStoragePersistent;

        IPAddress address;
        infra::BoundedConstString hostname;
        mbedtls_ssl_session session;
        bool clientSessionObtained = false;
        uint32_t identifier;
    };

    class MbedTlsSessionWithCallback
        : public MbedTlsSession
    {
    public:
        MbedTlsSessionWithCallback(uint32_t identifier, const infra::Function<void(MbedTlsSession*)>& onObtained);
        MbedTlsSessionWithCallback(uint32_t identifier, infra::BoundedConstString hostname, const infra::Function<void(MbedTlsSession*)>& onObtained);
        MbedTlsSessionWithCallback(uint32_t identifier, IPAddress address, const infra::Function<void(MbedTlsSession*)>& onObtained);
        MbedTlsSessionWithCallback(network::MbedTlsPersistedSession&, const infra::Function<void(MbedTlsSession*)>& onObtained);

        // Implementation of MbedTlsSession
        void Obtained() override;

    private:
        friend class MbedTlsSessionStoragePersistent;

        infra::Function<void(MbedTlsSession*)> onObtained;
    };

    class MbedTlsSessionStorage
    {
    public:
        MbedTlsSessionStorage() = default;
        MbedTlsSessionStorage(const MbedTlsSessionStorage& other) = delete;
        MbedTlsSessionStorage& operator=(const MbedTlsSessionStorage& other) = delete;
        virtual ~MbedTlsSessionStorage() = default;

        virtual MbedTlsSession* NewSession(infra::BoundedConstString hostname) = 0;
        virtual MbedTlsSession* NewSession(IPAddress address) = 0;
        virtual MbedTlsSession* GetSession(infra::BoundedConstString hostname) = 0;
        virtual MbedTlsSession* GetSession(IPAddress address) = 0;
        virtual bool Full() const = 0;
        virtual void Clear() = 0;
        virtual void Invalidate(MbedTlsSession* session) = 0;
    };

    class MbedTlsSessionStorageRam
        : public MbedTlsSessionStorage
    {
    public:
        template<std::size_t Max>
        using WithMaxSize = infra::WithStorage<MbedTlsSessionStorageRam, infra::BoundedList<MbedTlsSession>::WithMaxSize<Max>>;

        explicit MbedTlsSessionStorageRam(infra::BoundedList<MbedTlsSession>& storage);

        MbedTlsSession* NewSession(infra::BoundedConstString hostname) override;
        MbedTlsSession* NewSession(IPAddress address) override;
        MbedTlsSession* GetSession(infra::BoundedConstString hostname) override;
        MbedTlsSession* GetSession(IPAddress address) override;
        void Invalidate(MbedTlsSession* sessionToInvalidate) override;
        bool Full() const override;
        void Clear() override;

    private:
        infra::BoundedList<MbedTlsSession>& storage;
    };

    class MbedTlsSessionStoragePersistent
        : public MbedTlsSessionStorage
    {
    public:
        template<std::size_t MaxRamSession>
        using WithMaxSize = infra::WithStorage<MbedTlsSessionStoragePersistent, infra::BoundedList<MbedTlsSessionWithCallback>::WithMaxSize<MaxRamSession>>;

        MbedTlsSessionStoragePersistent(infra::BoundedList<MbedTlsSessionWithCallback>& storage, services::ConfigurationStoreAccess<infra::BoundedVector<network::MbedTlsPersistedSession>>& nvm);

        MbedTlsSession* NewSession(infra::BoundedConstString hostname) override;
        MbedTlsSession* NewSession(IPAddress address) override;
        MbedTlsSession* GetSession(infra::BoundedConstString hostname) override;
        MbedTlsSession* GetSession(IPAddress address) override;
        void Invalidate(MbedTlsSession* sessionToInvalidate) override;
        bool Full() const override;
        void Clear() override;

    private:
        uint32_t NextIdentifier();
        void SessionUpdated(MbedTlsSession* session);
        void LoadSessions();
        network::MbedTlsPersistedSession* FindPersistedSession(uint32_t identifier);
        network::MbedTlsPersistedSession& NewPersistedSession();

        services::ConfigurationStoreAccess<infra::BoundedVector<network::MbedTlsPersistedSession>>& nvm;
        infra::BoundedList<MbedTlsSessionWithCallback>& storage;
    };

}

#endif
