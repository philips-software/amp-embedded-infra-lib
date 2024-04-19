#ifndef SERVICES_MBED_TLS_SESSION_HPP
#define SERVICES_MBED_TLS_SESSION_HPP

#include "generated/echo/Network.pb.hpp"
#include "infra/util/BoundedList.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/WithStorage.hpp"
#include "mbedtls/ssl.h"
#include "services/network/Address.hpp"
#include "services/util/ConfigurationStore.hpp"
#include "services/util/Sha256.hpp"
#include "services/util/Sha256MbedTls.hpp"
#include <memory>

namespace services
{
    class MbedTlsSessionStorageRam;
    class MbedTlsSessionStoragePersistent;

    class MbedTlsSession
    {
    public:
        explicit MbedTlsSession(Sha256::Digest identifier);
        explicit MbedTlsSession(network::MbedTlsPersistedSession&);
        virtual ~MbedTlsSession();

        static void Serialize(MbedTlsSession& in, network::MbedTlsPersistedSession& out);
        static void Deserialize(network::MbedTlsPersistedSession& in, MbedTlsSession& out);

        virtual void Reinitialize();
        virtual void Obtained();
        virtual bool IsObtained();
        virtual int SetSession(mbedtls_ssl_context* context);
        virtual int GetSession(mbedtls_ssl_context* context);
        virtual const infra::BoundedVector<uint8_t>& Identifier() const;

    private:
        friend class MbedTlsSessionStorageRam;
        friend class MbedTlsSessionStoragePersistent;

        mbedtls_ssl_session session;
        bool clientSessionObtained = false;
        infra::BoundedVector<uint8_t>::WithMaxSize<32> identifier;
    };

    class TlsSessionHasher
    {
    public:
        TlsSessionHasher() = default;
        TlsSessionHasher(const TlsSessionHasher& other) = delete;
        TlsSessionHasher& operator=(const TlsSessionHasher& other) = delete;
        ~TlsSessionHasher() = default;

        virtual Sha256::Digest HashHostname(infra::BoundedConstString hostname) = 0;
        virtual Sha256::Digest HashIP(IPAddress address) = 0;
    };

    class MbedTlsSessionHasher
        : public TlsSessionHasher
    {
    public:
        using WithMbedTlsHasher = infra::WithStorage<MbedTlsSessionHasher, Sha256MbedTls>;

        explicit MbedTlsSessionHasher(Sha256& hasher);

        Sha256::Digest HashHostname(infra::BoundedConstString hostname) override;
        Sha256::Digest HashIP(IPAddress address) override;

    private:
        Sha256& hasher;
    };

    class MbedTlsSessionWithCallback
        : public MbedTlsSession

    {
    public:
        MbedTlsSessionWithCallback(Sha256::Digest identifier, const infra::Function<void(MbedTlsSession*)>& onObtained);
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
        using SingleSession = infra::WithStorage<infra::WithStorage<MbedTlsSessionStorageRam, infra::BoundedList<MbedTlsSession>::WithMaxSize<1>>, MbedTlsSessionHasher::WithMbedTlsHasher>;

        explicit MbedTlsSessionStorageRam(infra::BoundedList<MbedTlsSession>& storage, TlsSessionHasher& hasher);

        MbedTlsSession* NewSession(infra::BoundedConstString hostname) override;
        MbedTlsSession* NewSession(IPAddress address) override;
        MbedTlsSession* GetSession(infra::BoundedConstString hostname) override;
        MbedTlsSession* GetSession(IPAddress address) override;
        void Invalidate(MbedTlsSession* sessionToInvalidate) override;
        bool Full() const override;
        void Clear() override;

    private:
        infra::BoundedList<MbedTlsSession>& storage;
        TlsSessionHasher& hasher;
    };

    class MbedTlsSessionStoragePersistent
        : public MbedTlsSessionStorage
    {
    public:
        template<std::size_t MaxRamSession>
        using WithMaxSize = infra::WithStorage<MbedTlsSessionStoragePersistent, infra::BoundedList<MbedTlsSessionWithCallback>::WithMaxSize<MaxRamSession>>;

        MbedTlsSessionStoragePersistent(infra::BoundedList<MbedTlsSessionWithCallback>& storage, services::ConfigurationStoreAccess<infra::BoundedVector<network::MbedTlsPersistedSession>>& nvm, TlsSessionHasher& hasher);

        MbedTlsSession* NewSession(infra::BoundedConstString hostname) override;
        MbedTlsSession* NewSession(IPAddress address) override;
        MbedTlsSession* GetSession(infra::BoundedConstString hostname) override;
        MbedTlsSession* GetSession(IPAddress address) override;
        void Invalidate(MbedTlsSession* sessionToInvalidate) override;
        bool Full() const override;
        void Clear() override;

    private:
        void SessionUpdated(MbedTlsSession* session);
        void LoadSessions();
        network::MbedTlsPersistedSession* FindPersistedSession(const infra::BoundedVector<uint8_t>& identifier);
        network::MbedTlsPersistedSession& NewPersistedSession();

        services::ConfigurationStoreAccess<infra::BoundedVector<network::MbedTlsPersistedSession>>& nvm;
        infra::BoundedList<MbedTlsSessionWithCallback>& storage;
        TlsSessionHasher& hasher;
    };

}

#endif
