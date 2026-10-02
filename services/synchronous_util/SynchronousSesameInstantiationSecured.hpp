#ifndef SERVICES_SYNCHRONOUS_SESAME_INSTANTIATION_SECURED_HPP
#define SERVICES_SYNCHRONOUS_SESAME_INSTANTIATION_SECURED_HPP

#include "services/synchronous_util/SynchronousSesameSecured.hpp"
#include "services/util/SesameInstantiation.hpp"

namespace main_
{
    struct SynchronousSesameSecured
        : Sesame
    {
    public:
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        SynchronousSesameSecured(CobsStorageBase& storage,
            infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, const services::SynchronousSesameSecured::KeyMaterial& keyMaterial);

        services::SynchronousSesameSecured::WithCryptoMbedTls secured;

        template<std::size_t MessageSize>
        struct SecuredStorage
        {
            infra::BoundedVector<uint8_t>::WithMaxSize<services::SynchronousSesameSecured::encodedMessageSize<MessageSize>> securedSendBuffer;
            infra::BoundedVector<uint8_t>::WithMaxSize<services::SynchronousSesameSecured::encodedMessageSize<MessageSize>> securedReceiveBuffer;
        };
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct SynchronousSesameSecured::WithMessageSize
        : private SynchronousSesameSecured::SecuredStorage<MessageSize>
        , private Sesame::CobsStorage<MessageSize, SplitBuffers>
        , SynchronousSesameSecured
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, const services::SynchronousSesameSecured::KeyMaterial& keyMaterial)
            : SynchronousSesameSecured(static_cast<CobsStorageBase&>(*this), this->securedSendBuffer, this->securedReceiveBuffer, serialCommunication, keyMaterial)
        {}
    };
}

#endif
