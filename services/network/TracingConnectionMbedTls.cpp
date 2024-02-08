#include "services/network/TracingConnectionMbedTls.hpp"
#include "mbedtls/debug.h"
#include "mbedtls/error.h"

namespace services
{
    TracingConnectionMbedTls::TracingConnectionMbedTls(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver,
        CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, const ConnectionMbedTls::ParametersWorkaround& parameters, Tracer& tracer)
        : ConnectionMbedTls(std::move(createdObserver), certificates, randomDataGenerator, parameters)
        , tracer(tracer)
    {
        tracer.Trace() << "ConnectionMbedTls::ConnectionMbedTls";
    }

    TracingConnectionMbedTls::~TracingConnectionMbedTls()
    {
        tracer.Trace() << "ConnectionMbedTls::~ConnectionMbedTls";
    }

    void TracingConnectionMbedTls::TlsInitFailure(int reason)
    {
        LogFailure("ConnectionMbedTls::TlsInitFailure", reason);
    }

    void TracingConnectionMbedTls::TlsReadFailure(int reason)
    {
        LogFailure("ConnectionMbedTls::TlsReadFailure", reason);
    }

    void TracingConnectionMbedTls::TlsWriteFailure(int reason)
    {
        LogFailure("ConnectionMbedTls::TlsWriteFailure", reason);
    }

    void TracingConnectionMbedTls::TlsLog(int level, const char* file, int line, const char* message)
    {
        infra::BoundedConstString fileCopy(file);
        infra::BoundedConstString messageCopy(message);

        if (messageCopy.find('\n') != infra::BoundedConstString::npos)
            messageCopy = messageCopy.substr(0, messageCopy.find('\n'));

        tracer.Trace() << "[" << fileCopy.substr(fileCopy.find_last_of('\\') + 1) << ":" << line
                       << "] (" << level << "): " << messageCopy;
    }

    void TracingConnectionMbedTls::LogFailure(const char* what, int reason)
    {
        tracer.Trace() << what << ": ";

#if defined(MBEDTLS_ERROR_C)
        infra::BoundedString::WithStorage<128> description;
        mbedtls_strerror(reason, description.data(), description.max_size());

        tracer.Continue() << description;
#endif

        tracer.Continue() << " (-0x" << infra::hex << infra::Width(4, '0') << std::abs(reason) << infra::resetWidth << ")";
    }

    AllocatorTracingConnectionMbedTlsAdapter::AllocatorTracingConnectionMbedTlsAdapter(AllocatorTracingConnectionMbedTls& allocator, Tracer& tracer)
        : allocator(allocator)
        , tracer(tracer)
    {
        tracer.Trace() << "AllocatorTracingConnectionMbedTlsAdapter::AllocatorTracingConnectionMbedTlsAdapter()";
    }

    infra::SharedPtr<ConnectionMbedTls> AllocatorTracingConnectionMbedTlsAdapter::Allocate(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver,
        CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, const ConnectionMbedTls::ParametersWorkaround& parameters)
    {
        return allocator.Allocate(std::move(createdObserver), certificates, randomDataGenerator, parameters, tracer);
    }

    void AllocatorTracingConnectionMbedTlsAdapter::OnAllocatable(infra::AutoResetFunction<void()>&& callback)
    {
        allocator.OnAllocatable(std::move(callback));
    }

    TracingConnectionFactoryMbedTls::TracingConnectionFactoryMbedTls(AllocatorTracingConnectionMbedTls& connectionAllocator, AllocatorConnectionMbedTlsListener& listenerAllocator, infra::BoundedList<ConnectionMbedTlsConnector>& connectors,
        ConnectionFactory& factory, CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, Tracer& tracer, DebugLevel level, ConnectionMbedTls::CertificateValidation certificateValidation)
        : ConnectionFactoryMbedTls(allocatorAdapter, listenerAllocator, connectors, factory, certificates, randomDataGenerator, certificateValidation)
        , allocatorAdapter(connectionAllocator, tracer)
    {
        tracer.Trace() << "ConnectionFactoryMbedTls::ConnectionFactoryMbedTls()";
#if defined(MBEDTLS_DEBUG_C)
        mbedtls_debug_set_threshold(level);
#endif
    }

    TracingConnectionFactoryWithNameResolverMbedTls::TracingConnectionFactoryWithNameResolverMbedTls(AllocatorTracingConnectionMbedTls& connectionAllocator, infra::BoundedList<ConnectionMbedTlsConnectorWithNameResolver>& connectors,
        ConnectionFactoryWithNameResolver& factory, CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, Tracer& tracer, [[maybe_unused]] DebugLevel level, ConnectionMbedTls::CertificateValidation certificateValidation)
        : ConnectionFactoryWithNameResolverMbedTls(allocatorAdapter, connectors, factory, certificates, randomDataGenerator, certificateValidation)
        , allocatorAdapter(connectionAllocator, tracer)
    {
        tracer.Trace() << "ConnectionFactoryWithNameResolver::ConnectionFactoryWithNameResolver()";
#if defined(MBEDTLS_DEBUG_C)
        mbedtls_debug_set_threshold(level);
#endif
    }
}
