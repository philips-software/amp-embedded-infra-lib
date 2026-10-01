#ifndef SERVICES_UTIL_SESAME_INSTANTIATION_SECURED_HPP
#define SERVICES_UTIL_SESAME_INSTANTIATION_SECURED_HPP

#include "services/util/SesameInstantiation.hpp"
#include "services/util/SesameSecured.hpp"

namespace main_
{
    struct SesameSecured
        : Sesame
    {
    public:
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        SesameSecured(CobsStorageBase& storage,
            infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, const services::SesameSecured::KeyMaterial& keyMaterial);

        services::SesameSecured::WithCryptoMbedTls secured;

        template<std::size_t MessageSize>
        struct SecuredStorage
        {
            infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameSecured::encodedMessageSize<MessageSize>> securedSendBuffer;
            infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameSecured::encodedMessageSize<MessageSize>> securedReceiveBuffer;
        };
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct SesameSecured::WithMessageSize
        : private SesameSecured::SecuredStorage<MessageSize>
        , private Sesame::CobsStorage<MessageSize, SplitBuffers>
        , SesameSecured
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, const services::SesameSecured::KeyMaterial& keyMaterial)
            : SesameSecured(static_cast<CobsStorageBase&>(*this), this->securedSendBuffer, this->securedReceiveBuffer, serialCommunication, keyMaterial)
        {}
    };
}

#endif
