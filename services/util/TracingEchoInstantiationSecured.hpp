#ifndef SERVICES_TRACING_ECHO_INSTANTIATION_SECURED_HPP
#define SERVICES_TRACING_ECHO_INSTANTIATION_SECURED_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "protobuf/echo/EchoErrorPolicy.hpp"
#include "services/tracer/Tracer.hpp"
#include "services/util/EchoPolicyDiffieHellman.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/SesameInstantiation.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/SesameWindowed.hpp"
#include "services/util/Stoppable.hpp"
#include "services/util/TracingEchoOnSesame.hpp"

namespace main_
{
    struct TracingEchoOnSesameSecured
        : public services::Stoppable
    {
        template<std::size_t MessageSize>
        struct SecuredStorage
        {
            services::SesameSecured::AlignedBuffer<services::SesameSecured::encodedMessageSize<MessageSize>> securedSendBuffer;
            services::SesameSecured::AlignedBuffer<services::SesameSecured::encodedMessageSize<MessageSize>> securedReceiveBuffer;
        };

        TracingEchoOnSesameSecured(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, services::AesGcmEncryption& sendEncryption, services::AesGcmEncryption& receiveEncryption,
            const services::SesameSecured::KeyMaterial& keyMaterial, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted);

        void Reset();

        // Implementation of Stoppable
        void Stop(const infra::Function<void()>& onDone) override;

        services::SesameCobs cobs;
        services::SesameWindowed windowed;
        services::SesameSecured secured;
        services::TracingEchoOnSesame echo;

        infra::AutoResetFunction<void()> onStopDone;
    };

    struct TracingEchoOnSesameSecuredDiffieHellman
        : TracingEchoOnSesameSecured
    {
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        struct Crypto
        {
            services::AesGcmEncryption& sendEncryption;
            services::AesGcmEncryption& receiveEncryption;
            services::EchoPolicyDiffieHellman::Crypto keyEstablishment;
        };

        TracingEchoOnSesameSecuredDiffieHellman(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const Crypto& crypto, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate,
            services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted);

        services::EchoPolicyDiffieHellman policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize
        : private Sesame::CobsStorage<MessageSize, SplitBuffers>
        , private TracingEchoOnSesameSecured::SecuredStorage<MessageSize>
        , TracingEchoOnSesameSecuredDiffieHellman
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const Crypto& crypto, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate,
            services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : TracingEchoOnSesameSecuredDiffieHellman(static_cast<Sesame::CobsStorageBase&>(*this), this->securedSendBuffer, this->securedReceiveBuffer, serialCommunication, serializerFactory, crypto, dsaCertificate, rootCaCertificate, tracer, echoErrorPolicy, initializer)
        {}
    };
}

#endif
