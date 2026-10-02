#include "services/synchronous_util/SynchronousSesameInstantiationSecured.hpp"

namespace main_
{
    SynchronousSesameSecured::SynchronousSesameSecured(CobsStorageBase& storage,
        infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, const services::SynchronousSesameSecured::KeyMaterial& keyMaterial)
        : Sesame(storage, serialCommunication)
        , secured(securedSendBuffer, securedReceiveBuffer, windowed, keyMaterial)
    {}
}
