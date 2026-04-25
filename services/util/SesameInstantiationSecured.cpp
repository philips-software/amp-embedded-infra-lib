#include "services/util/SesameInstantiationSecured.hpp"

namespace main_
{
    SesameSecured::SesameSecured(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedDeque<uint8_t>& windowedReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, const services::SesameSecured::KeyMaterial& keyMaterial)
        : Sesame(cobsSendStorage, cobsReceivedMessage, windowedReceivedMessage, serialCommunication)
        , secured(securedSendBuffer, securedReceiveBuffer, windowed, keyMaterial)
    {}
}
