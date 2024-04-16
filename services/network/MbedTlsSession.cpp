#include "services/network/MbedTlsSession.hpp"
#include "services/tracer/GlobalTracer.hpp"

namespace services
{
    void MbedTlsSession::Serialize(MbedTlsSession& in, network::MbedTlsPersistedSession& out)
    {
        size_t outputLen;
        out.address = services::ConvertToUint32(in.address.Get<IPv4Address>());
        out.hostname = in.hostname;
        out.clientSessionObtained = in.clientSessionObtained;
        out.identifier = in.identifier;
        out.serializedSession.resize(out.serializedSession.max_size());
        auto result = mbedtls_ssl_session_save(&in.session, out.serializedSession.begin(), out.serializedSession.max_size(), &outputLen);
        out.serializedSession.resize(outputLen);
        really_assert(result == 0);

        // if (result != 0)
        //     services::GlobalTracer().Trace() << "<<<<<< SSL Session Save Resulted in Failure >>>>>>" << outputLen;
        // // really_assert(outputLen < persistedSession->serializedSession.max_size());
        // if (outputLen > persistedSession->serializedSession.max_size())
        //     services::GlobalTracer().Trace() << "<<<<<< SSL Session Save Does not fit into buffer >>>>>>" << outputLen;
    }

    void MbedTlsSession::Deserialize(network::MbedTlsPersistedSession& in, MbedTlsSession& out)
    {
        out = *new (&out) MbedTlsSession(in);
    }

    MbedTlsSession::MbedTlsSession(uint32_t identifier)
        : session({})
        , identifier(identifier)
    {
        mbedtls_ssl_session_init(&session);
    }

    MbedTlsSession::MbedTlsSession(uint32_t identifier, infra::BoundedConstString hostname)
        : hostname(hostname)
        , session({})
        , identifier(identifier)
    {
        mbedtls_ssl_session_init(&session);
    }

    MbedTlsSession::MbedTlsSession(uint32_t identifier, IPAddress address)
        : address(address)
        , session({})
        , identifier(identifier)
    {
        mbedtls_ssl_session_init(&session);
    }

    MbedTlsSession::MbedTlsSession(network::MbedTlsPersistedSession& reference)
        : address(services::ConvertFromUint32(reference.address))
        , hostname(reference.hostname)
        , session({})
        , clientSessionObtained(reference.clientSessionObtained)
        , identifier(reference.identifier)
    {
        mbedtls_ssl_session_init(&session);
        mbedtls_ssl_session_load(&session, reference.serializedSession.begin(), reference.serializedSession.size());
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

    IPAddress& MbedTlsSession::Address()
    {
        return address;
    }

    infra::BoundedConstString MbedTlsSession::Hostname() const
    {
        return hostname;
    }

    uint32_t MbedTlsSession::Identifier() const
    {
        return identifier;
    }

    MbedTlsSessionWithCallback::MbedTlsSessionWithCallback(uint32_t identifier, const infra::Function<void(MbedTlsSession*)>& onObtained)
        : MbedTlsSession(identifier)
        , onObtained(onObtained)
    {}

    MbedTlsSessionWithCallback::MbedTlsSessionWithCallback(uint32_t identifier, infra::BoundedConstString hostname, const infra::Function<void(MbedTlsSession*)>& onObtained)
        : MbedTlsSession(identifier, hostname)
        , onObtained(onObtained)
    {}

    MbedTlsSessionWithCallback::MbedTlsSessionWithCallback(uint32_t identifier, IPAddress address, const infra::Function<void(MbedTlsSession*)>& onObtained)
        : MbedTlsSession(identifier, address)
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

    MbedTlsSessionStorageRam::MbedTlsSessionStorageRam(infra::BoundedList<MbedTlsSession>& storage)
        : storage(storage)
    {}

    MbedTlsSession* MbedTlsSessionStorageRam::NewSession(infra::BoundedConstString hostname)
    {
        storage.emplace_back(0, hostname);
        return &storage.back();
    }

    MbedTlsSession* MbedTlsSessionStorageRam::NewSession(IPAddress address)
    {
        storage.emplace_back(0, address);
        return &storage.back();
    }

    MbedTlsSession* MbedTlsSessionStorageRam::GetSession(infra::BoundedConstString hostname)
    {
        for (auto& session : storage)
        {
            if (session.Hostname() == hostname)
                return &session;
        }
        return nullptr;
    }

    MbedTlsSession* MbedTlsSessionStorageRam::GetSession(IPAddress address)
    {
        for (auto& session : storage)
        {
            if (session.Address() == address)
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

    MbedTlsSessionStoragePersistent::MbedTlsSessionStoragePersistent(infra::BoundedList<MbedTlsSessionWithCallback>& storage, services::ConfigurationStoreAccess<infra::BoundedVector<network::MbedTlsPersistedSession>>& nvm)
        : nvm(nvm)
        , storage(storage)
    {
        LoadSessions();
    }

    MbedTlsSession* MbedTlsSessionStoragePersistent::NewSession(infra::BoundedConstString hostname)
    {
        storage.emplace_back(NextIdentifier(), hostname, [this](MbedTlsSession* session)
            {
                SessionUpdated(session);
            });
        return &storage.back();
    }

    MbedTlsSession* MbedTlsSessionStoragePersistent::NewSession(IPAddress address)
    {
        storage.emplace_back(NextIdentifier(), address, [this](MbedTlsSession* session)
            {
                SessionUpdated(session);
            });
        return &storage.back();
    }

    MbedTlsSession* MbedTlsSessionStoragePersistent::GetSession(infra::BoundedConstString hostname)
    {
        for (auto& session : storage)
        {
            // remove
            services::GlobalTracer().Trace() << "Searching: " << hostname << " == " << session.Hostname();
            if (session.Hostname() == hostname)
                return &session;
        }
        return nullptr;
    }

    MbedTlsSession* MbedTlsSessionStoragePersistent::GetSession(IPAddress address)
    {
        for (auto& session : storage)
        {
            if (session.Address() == address)
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
        return nvm->full();
    }

    void MbedTlsSessionStoragePersistent::Clear()
    {
        storage.clear();
        nvm->clear();
        nvm.Write();
    }

    uint32_t MbedTlsSessionStoragePersistent::NextIdentifier()
    {
        return nvm->size();
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

    network::MbedTlsPersistedSession* MbedTlsSessionStoragePersistent::FindPersistedSession(uint32_t identifier)
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
