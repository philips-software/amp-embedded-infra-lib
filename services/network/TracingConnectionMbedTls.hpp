#ifndef SERVICES_TRACING_CONNECTION_MBED_TLS_HPP
#define SERVICES_TRACING_CONNECTION_MBED_TLS_HPP

#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/network/ConnectionMbedTls.hpp"
#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracingConnectionMbedTls
        : public ConnectionMbedTls
    {
    public:
        TracingConnectionMbedTls(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, const ConnectionMbedTls::ParametersWorkaround& parameters, Tracer& tracer);
        ~TracingConnectionMbedTls();

        void TlsInitFailure(int reason) override;
        void TlsReadFailure(int reason) override;
        void TlsWriteFailure(int reason) override;
        void TlsLog(int level, const char* file, int line, const char* message) override;

    private:
        void LogFailure(const char* what, int reason);

    private:
        Tracer& tracer;
    };

    using AllocatorTracingConnectionMbedTls = infra::SharedObjectAllocator<TracingConnectionMbedTls,
        void(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, const ConnectionMbedTls::ParametersWorkaround& parameters, Tracer& tracer)>;

    class AllocatorTracingConnectionMbedTlsAdapter
        : public AllocatorConnectionMbedTls
    {
    public:
        AllocatorTracingConnectionMbedTlsAdapter(AllocatorTracingConnectionMbedTls& allocator, Tracer& tracer);

        infra::SharedPtr<ConnectionMbedTls> Allocate(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver,
            CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, const ConnectionMbedTls::ParametersWorkaround& parameters) override;
        void OnAllocatable(infra::AutoResetFunction<void()>&& callback) override;

    private:
        AllocatorTracingConnectionMbedTls& allocator;
        Tracer& tracer;
    };

    class TracingConnectionFactoryMbedTls
        : public ConnectionFactoryMbedTls
    {
    public:
        enum DebugLevel
        {
            NoDebug,
            Error,
            StateChange,
            Informational,
            Verbose
        };

    public:
        template<std::size_t MaxConnections, std::size_t MaxListeners, std::size_t MaxConnectors>
        using WithMaxConnectionsListenersAndConnectors = infra::WithStorage<infra::WithStorage<infra::WithStorage<infra::WithStorage<TracingConnectionFactoryMbedTls, AllocatorTracingConnectionMbedTls::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxConnections>>, AllocatorConnectionMbedTlsListener::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxListeners>>, infra::BoundedList<ConnectionMbedTlsConnector>::WithMaxSize<MaxConnectors>>, MbedTlsSessionStorageRam::SingleSession>;
        template<std::size_t MaxConnections, std::size_t MaxListeners, std::size_t MaxConnectors>
        using CustomSessionStorageWithMaxConnectionsListenersAndConnectors = infra::WithStorage<infra::WithStorage<infra::WithStorage<TracingConnectionFactoryMbedTls, AllocatorTracingConnectionMbedTls::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxConnections>>, AllocatorConnectionMbedTlsListener::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxListeners>>, infra::BoundedList<ConnectionMbedTlsConnector>::WithMaxSize<MaxConnectors>>;

        TracingConnectionFactoryMbedTls(AllocatorTracingConnectionMbedTls& connectionAllocator, AllocatorConnectionMbedTlsListener& listenerAllocator, infra::BoundedList<ConnectionMbedTlsConnector>& connectors, MbedTlsSessionStorage& sessionStorage,
            ConnectionFactory& factory, CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, Tracer& tracer, DebugLevel level, ConnectionMbedTls::CertificateValidation certificateValidation = ConnectionMbedTls::CertificateValidation::Default);

    private:
        AllocatorTracingConnectionMbedTlsAdapter allocatorAdapter;
    };

    class TracingConnectionFactoryWithNameResolverMbedTls
        : public ConnectionFactoryWithNameResolverMbedTls
    {
    public:
        enum DebugLevel
        {
            NoDebug,
            Error,
            StateChange,
            Informational,
            Verbose
        };

    public:
        template<std::size_t MaxConnections, std::size_t MaxConnectors>
        using WithMaxConnectionsAndConnectors = infra::WithStorage<infra::WithStorage<infra::WithStorage<TracingConnectionFactoryWithNameResolverMbedTls, AllocatorTracingConnectionMbedTls::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxConnections>>, infra::BoundedList<ConnectionMbedTlsConnectorWithNameResolver>::WithMaxSize<MaxConnectors>>, MbedTlsSessionStorageRam::SingleSession>;
        template<std::size_t MaxConnections, std::size_t MaxConnectors>
        using CustomSessionStorageWithMaxConnectionsAndConnectors = infra::WithStorage<infra::WithStorage<TracingConnectionFactoryWithNameResolverMbedTls, AllocatorTracingConnectionMbedTls::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxConnections>>, infra::BoundedList<ConnectionMbedTlsConnectorWithNameResolver>::WithMaxSize<MaxConnectors>>;

        TracingConnectionFactoryWithNameResolverMbedTls(AllocatorTracingConnectionMbedTls& connectionAllocator, infra::BoundedList<ConnectionMbedTlsConnectorWithNameResolver>& connectors, MbedTlsSessionStorage& sessionStorage,
            ConnectionFactoryWithNameResolver& factory, CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, Tracer& tracer, DebugLevel level, ConnectionMbedTls::CertificateValidation certificateValidation = ConnectionMbedTls::CertificateValidation::Default);

    private:
        AllocatorTracingConnectionMbedTlsAdapter allocatorAdapter;
    };
}

#endif
