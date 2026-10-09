#ifndef SERVICES_UTIL_SESAME_INSTANTIATION_SECURED_HPP
#define SERVICES_UTIL_SESAME_INSTANTIATION_SECURED_HPP

#include "services/util/SesameInstantiation.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/Stoppable.hpp"
#ifdef EMIL_USE_MBEDTLS
#include "services/util/SesameCryptoMbedTls.hpp"
#endif

namespace main_
{
    template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
    struct SesameInstantiationSecured
        : public services::Stoppable
    {
        SesameInstantiationSecured(services::AesGcmEncryptors& encryptors, hal::BufferedSerialCommunication& serialCommunication,
            const services::SesameSecured::KeyMaterial& keyMaterial = {}, services::SesameInitializer& initializer = services::immediatelyGranted)
            : sesame(serialCommunication, initializer)
            , secured(encryptors, sesame.windowed, keyMaterial)
        {}

        void Stop(const infra::Function<void()>& onDone) override
        {
            sesame.Stop(onDone);
        }

        Sesame::WithMessageSize<MessageSize, SplitBuffers> sesame;
        services::SesameSecured::WithMessageSize<MessageSize> secured;

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif
    };

#ifdef EMIL_USE_MBEDTLS
    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct SesameInstantiationSecured<MessageSize, SplitBuffers>::WithCryptoMbedTls
        : private services::SesameSecuredMbedTlsEncryptors
        , public SesameInstantiationSecured<MessageSize, SplitBuffers>
    {
        explicit WithCryptoMbedTls(hal::BufferedSerialCommunication& serialCommunication, const services::SesameSecured::KeyMaterial& keyMaterial = {}, services::SesameInitializer& initializer = services::immediatelyGranted)
            : SesameInstantiationSecured<MessageSize, SplitBuffers>(*this, serialCommunication, keyMaterial, initializer)
        {}
    };
#endif
}

#endif
