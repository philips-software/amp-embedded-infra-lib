#ifndef SERVICES_UTIL_ECHO_INSTANTIATION_SECURED_HPP
#define SERVICES_UTIL_ECHO_INSTANTIATION_SECURED_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/util/BoundedVector.hpp"
#include "protobuf/echo/ServiceForwarder.hpp"
#include "services/util/EchoOnMessageCommunication.hpp"
#include "services/util/EchoOnSesameSymmetricKey.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/SesameWindowed.hpp"

namespace main_
{
    template<std::size_t MessageSize>
    struct EchoOnSesameSecured
    {
        EchoOnSesameSecured(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
            : cobs(serialCommunication)
            , secured(windowed, keyMaterial)
            , echo(secured, serializerFactory, randomDataGenerator)
        {}

        ~EchoOnSesameSecured()
        {
            cobs.Stop();
            windowed.Stop();
        }

        operator services::Echo&()
        {
            return echo;
        }

        void Reset()
        {
            echo.Reset();
        }

        services::SesameCobs::WithMaxMessageSize<MessageSize> cobs;
        services::SesameWindowed windowed{ cobs };
        services::SesameSecured::WithBuffers<MessageSize> secured;
        services::EchoOnSesameSymmetricKey echo;
    };
}

#endif
