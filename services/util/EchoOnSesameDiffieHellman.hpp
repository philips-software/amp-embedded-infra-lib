#ifndef SERVICES_ECHO_ON_MESSAGE_COMMUNICATION_SYMMETRIC_KEY_HPP
#define SERVICES_ECHO_ON_MESSAGE_COMMUNICATION_SYMMETRIC_KEY_HPP

#include "generated/echo/SesameSecurity.pb.hpp"
#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "mbedtls/ecdh.h"
#include "mbedtls/ecp.h"
#include "mbedtls/pk.h"
#include "mbedtls/x509_crt.h"
#include "services/util/EchoOnSesame.hpp"
#include "services/util/SesameSecured.hpp"

namespace services
{
    class EchoOnSesameDiffieHellman
        : public EchoOnSesame
        , private sesame_security::DiffieHellmanKeyEstablishment
        , private sesame_security::DiffieHellmanKeyEstablishmentProxy
    {
    public:
        EchoOnSesameDiffieHellman(SesameSecured& secured, infra::BoundedConstString dsaCertificate, infra::BoundedConstString dsaCertificatePrivateKey, infra::BoundedConstString rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);
        ~EchoOnSesameDiffieHellman();

        // Implementation of Echo
        void RequestSend(ServiceProxy& serviceProxy) override;

        // Implementation of SesameObserver
        void Initialized() override;

    private:
        // Implementation of DiffieHellmanKeyEstablishment
        void Exchange(infra::ConstByteRange otherPublicKey, infra::ConstByteRange signatureR, infra::ConstByteRange signatureS) override;
        void PresentCertificate(infra::BoundedConstString dsaCertificate) override;

        void ReQueueWaitingProxies();
        void ExpandMaterial(infra::ConstByteRange sharedSecret, infra::ByteRange expandedMaterial) const;

        static int StaticRng(void*, unsigned char*, std::size_t);

    private:
        SesameSecured& secured;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        infra::BoundedConstString dsaCertificate;

        bool initializingKeys = true;
        bool sentExchange = false;
        infra::Optional<std::pair<std::array<uint8_t, 16>, std::array<uint8_t, 16>>> nextKeyPair;
        infra::IntrusiveList<ServiceProxy> waitingProxies;

        mbedtls_x509_crt rootCertificate;
        mbedtls_ecp_group group;
        mbedtls_mpi dhPrivateKey;
        mbedtls_ecp_point dhPublicKey;
        mbedtls_mpi dsaPrivateKey;
        mbedtls_ecp_point otherDsaPublicKey;

        mbedtls_ecdh_context ecdhContext;
    };
}

#endif
