#include "services/util/SesameInstantiationSecured.hpp"

namespace main_
{
    SesameSecured::SesameSecured(CobsStorageBase& storage,
        infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, const services::SesameSecured::KeyMaterial& keyMaterial)
        : Sesame(storage, serialCommunication)
        , secured(securedSendBuffer, securedReceiveBuffer, windowed, keyMaterial)
    {}
}
