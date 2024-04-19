#include "services/network/MbedTlsSession.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "services/network/Address.hpp"
#include "services/util/Sha256.hpp"

namespace services
{
    void MbedTlsSession::Serialize(MbedTlsSession& in, network::MbedTlsPersistedSession& out)
    {
        size_t outputLen;
        out.clientSessionObtained = in.clientSessionObtained;
        out.identifier = in.identifier;
        out.serializedSession.resize(out.serializedSession.max_size());
        auto result = mbedtls_ssl_session_save(&in.session, out.serializedSession.begin(), out.serializedSession.max_size(), &outputLen);
        out.serializedSession.resize(outputLen);
        really_assert(result == 0);
    }

    MbedTlsSession::MbedTlsSession(Sha256::Digest identifier)
        : session({})
        , identifier(identifier.begin(), identifier.end())
    {
        mbedtls_ssl_session_init(&session);
    }

    MbedTlsSession::MbedTlsSession(network::MbedTlsPersistedSession& reference)
        : session({})
        , clientSessionObtained(reference.clientSessionObtained)
        , identifier(reference.identifier)
    {
        mbedtls_ssl_session_init(&session);
        really_assert(mbedtls_ssl_session_load(&session, reference.serializedSession.begin(), reference.serializedSession.size()) == 0);
    }

    MbedTlsSession::~MbedTlsSession()
    {
        if (clientSessionObtained)
            mbedtls_ssl_session_free(&session);
    }

    void MbedTlsSession::Reinitialize()
    {
        mbedtls_ssl_session_init(&session);
    }

    void MbedTlsSession::Obtained()
    {
        clientSessionObtained = true;
    }

    bool MbedTlsSession::IsObtained()
    {
        return clientSessionObtained;
    }

    int MbedTlsSession::SetSession(mbedtls_ssl_context* context)
    {
        return mbedtls_ssl_set_session(context, &session);
    }

    int MbedTlsSession::GetSession(mbedtls_ssl_context* context)
    {
        return mbedtls_ssl_get_session(context, &session);
    }

    const infra::BoundedVector<uint8_t>& MbedTlsSession::Identifier() const
    {
        return identifier;
    }

    MbedTlsSessionHasher::MbedTlsSessionHasher(Sha256& hasher)
        : hasher(hasher)
    {}

    Sha256::Digest MbedTlsSessionHasher::HashHostname(infra::BoundedConstString hostname)
    {
        return hasher.Calculate(infra::StringAsByteRange(hostname));
    }

    Sha256::Digest MbedTlsSessionHasher::HashIP(IPAddress address)
    {
        infra::StringOutputStream::WithStorage<48> ipStringStream;
        ipStringStream << address;
        return hasher.Calculate(infra::StringAsByteRange(ipStringStream.Storage()));
    }

    MbedTlsSessionWithCallback::MbedTlsSessionWithCallback(Sha256::Digest identifier, const infra::Function<void(MbedTlsSession*)>& onObtained)
        : MbedTlsSession(identifier)
        , onObtained(onObtained)
    {}

    MbedTlsSessionWithCallback::MbedTlsSessionWithCallback(network::MbedTlsPersistedSession& reference, const infra::Function<void(MbedTlsSession*)>& onObtained)
        : MbedTlsSession(reference)
        , onObtained(onObtained)
    {}

    void MbedTlsSessionWithCallback::Obtained()
    {
        MbedTlsSession::Obtained();
        onObtained(this);
    }

    MbedTlsSessionStorageRam::MbedTlsSessionStorageRam(infra::BoundedList<MbedTlsSession>& storage, TlsSessionHasher& hasher)
        : storage(storage)
        , hasher(hasher)
    {}

    MbedTlsSession* MbedTlsSessionStorageRam::NewSession(infra::BoundedConstString hostname)
    {
        storage.emplace_back(hasher.HashHostname(hostname));
        return &storage.back();
    }

    MbedTlsSession* MbedTlsSessionStorageRam::NewSession(IPAddress address)
    {
        storage.emplace_back(hasher.HashIP(address));
        return &storage.back();
    }

    MbedTlsSession* MbedTlsSessionStorageRam::GetSession(infra::BoundedConstString hostname)
    {
        auto identifier = hasher.HashHostname(hostname);
        for (auto& session : storage)
        {
            if (session.Identifier().range() == identifier)
                return &session;
        }
        return nullptr;
    }

    MbedTlsSession* MbedTlsSessionStorageRam::GetSession(IPAddress address)
    {
        auto identifier = hasher.HashIP(address);
        for (auto& session : storage)
        {
            if (session.Identifier().range() == identifier)
                return &session;
        }
        return nullptr;
    }

    void MbedTlsSessionStorageRam::Invalidate(MbedTlsSession* sessionToInvalidate)
    {
        for (auto& session : storage)
        {
            if (sessionToInvalidate == &session)
                storage.remove(session);
        }
    }

    bool MbedTlsSessionStorageRam::Full() const
    {
        return storage.full();
    }

    void MbedTlsSessionStorageRam::Clear()
    {
        storage.clear();
    }

    MbedTlsSessionStoragePersistent::MbedTlsSessionStoragePersistent(infra::BoundedList<MbedTlsSessionWithCallback>& storage, services::ConfigurationStoreAccess<infra::BoundedVector<network::MbedTlsPersistedSession>>& nvm, TlsSessionHasher& hasher)
        : nvm(nvm)
        , storage(storage)
        , hasher(hasher)
    {
        LoadSessions();
    }

    MbedTlsSession* MbedTlsSessionStoragePersistent::NewSession(infra::BoundedConstString hostname)
    {
        storage.emplace_back(hasher.HashHostname(hostname), [this](MbedTlsSession* session)
            {
                SessionUpdated(session);
            });
        return &storage.back();
    }

    MbedTlsSession* MbedTlsSessionStoragePersistent::NewSession(IPAddress address)
    {
        storage.emplace_back(hasher.HashIP(address), [this](MbedTlsSession* session)
            {
                SessionUpdated(session);
            });
        return &storage.back();
    }

    MbedTlsSession* MbedTlsSessionStoragePersistent::GetSession(infra::BoundedConstString hostname)
    {
        auto digest = hasher.HashHostname(hostname);
        for (auto& session : storage)
        {
            if (session.Identifier().range() == digest)
                return &session;
        }
        return nullptr;
    }

    MbedTlsSession* MbedTlsSessionStoragePersistent::GetSession(IPAddress address)
    {
        auto digest = hasher.HashIP(address);
        for (auto& session : storage)
        {
            if (session.Identifier().range() == digest)
                return &session;
        }
        return nullptr;
    }

    void MbedTlsSessionStoragePersistent::Invalidate(MbedTlsSession* sessionToInvalidate)
    {
        for (auto& session : storage)
        {
            if (sessionToInvalidate == &session)
                storage.remove(session);
        }
    }

    bool MbedTlsSessionStoragePersistent::Full() const
    {
        return storage.full();
    }

    void MbedTlsSessionStoragePersistent::Clear()
    {
        storage.clear();
        nvm->clear();
        nvm.Write();
    }

    void MbedTlsSessionStoragePersistent::SessionUpdated(MbedTlsSession* session)
    {
        network::MbedTlsPersistedSession* persistedSession = FindPersistedSession(session->Identifier());

        if (persistedSession == nullptr)
            persistedSession = &NewPersistedSession();

        MbedTlsSession::Serialize(*session, *persistedSession);

        nvm.Write();
    }

    void MbedTlsSessionStoragePersistent::LoadSessions()
    {
        for (auto& sector : *nvm)
        {
            storage.emplace_back(sector, [this](MbedTlsSession* session)
                {
                    SessionUpdated(session);
                });
        }
    }

    network::MbedTlsPersistedSession* MbedTlsSessionStoragePersistent::FindPersistedSession(const infra::BoundedVector<uint8_t>& identifier)
    {
        for (auto& nvmSession : *nvm)
        {
            if (nvmSession.identifier == identifier)
                return &nvmSession;
        }
        return nullptr;
    }

    network::MbedTlsPersistedSession& MbedTlsSessionStoragePersistent::NewPersistedSession()
    {
        if (nvm->full())
            nvm->erase(nvm->begin());
        nvm->emplace_back();
        return nvm->back();
    }
}
