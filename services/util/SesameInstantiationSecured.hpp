#ifndef SERVICES_UTIL_SESAME_INSTANTIATION_SECURED_HPP
#define SERVICES_UTIL_SESAME_INSTANTIATION_SECURED_HPP

#include "services/util/Sesame.hpp"
#include "services/util/SesameInstantiation.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/Stoppable.hpp"
#ifdef EMIL_USE_MBEDTLS
#include "services/util/SesameCryptoMbedTls.hpp"
#endif

namespace main_
{
    struct SesameInstantiationSecured
        : public services::Stoppable
    {
        SesameInstantiationSecured(Sesame::StorageBase& sesameStorage, services::SesameSecured::StorageBase& securedStorage, services::AesGcmEncryptors& encryptors, hal::BufferedSerialCommunication& serialCommunication, const services::SesameSecured::KeyMaterial& keyMaterial = {}, services::SesameInitializer& initializer = services::immediatelyGranted)
            : sesame(sesameStorage, serialCommunication, initializer)
            , secured(securedStorage, encryptors, sesame.windowed, keyMaterial)
        {}

        void Stop(const infra::Function<void()>& onDone) override
        {
            sesame.Stop(onDone);
        }

        Sesame sesame;
        services::SesameSecured secured;

        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct SesameInstantiationSecured::WithMessageSize
        : private Sesame::Storage<MessageSize, SplitBuffers>
        , private services::SesameSecured::Storage<MessageSize>
        , SesameInstantiationSecured
    {
        WithMessageSize(services::AesGcmEncryptors& encryptors, hal::BufferedSerialCommunication& serialCommunication, const services::SesameSecured::KeyMaterial& keyMaterial = {}, services::SesameInitializer& initializer = services::immediatelyGranted)
            : SesameInstantiationSecured(static_cast<Sesame::StorageBase&>(*this), static_cast<services::SesameSecured::StorageBase&>(*this), encryptors, serialCommunication, keyMaterial, initializer)
        {
        }

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif
#ifdef EMIL_USE_CUSTOM_CRYPTO
        struct WithCustomCrypto;
#endif
    };

#ifdef EMIL_USE_MBEDTLS
    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct SesameInstantiationSecured::WithMessageSize<MessageSize, SplitBuffers>::WithCryptoMbedTls
        : private services::SesameSecuredMbedTlsEncryptors
        , public SesameInstantiationSecured::WithMessageSize<MessageSize, SplitBuffers>
    {
        explicit WithCryptoMbedTls(hal::BufferedSerialCommunication& serialCommunication, const services::SesameSecured::KeyMaterial& keyMaterial = {}, services::SesameInitializer& initializer = services::immediatelyGranted)
            : SesameInstantiationSecured::WithMessageSize<MessageSize, SplitBuffers>(*this, serialCommunication, keyMaterial, initializer)
        {}
    };
#endif
}

#endif
