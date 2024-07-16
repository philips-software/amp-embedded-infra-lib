#ifndef SERVICES_UTIL_ECHO_INSTANTIATIONS
#define SERVICES_UTIL_ECHO_INSTANTIATIONS

#include "infra/util/BoundedVector.hpp"
#include "protobuf/echo/ServiceForwarder.hpp"
#include "services/util/EchoOnSesame.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/SesameWindowed.hpp"
#ifdef EMIL_HAL_WINDOWS
#include "hal/windows/UartWindows.hpp"
#else
#include "hal/unix/UartUnix.hpp"
#endif

namespace main_
{
    template<std::size_t MessageSize>
    struct EchoOnSesame
    {
        EchoOnSesame(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory)
            : cobs(serialCommunication)
            , echo(windowed, serializerFactory)
        {}

        ~EchoOnSesame()
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
        services::EchoOnSesame echo;
    };

    template<std::size_t MessageSize>
    struct EchoOnUartBase
    {
        EchoOnUartBase(infra::BoundedConstString portName)
            : uart(infra::AsStdString(portName))
        {}

#ifdef EMIL_HAL_WINDOWS
        hal::UartWindows uart;
#else
        hal::UartUnix uart;
#endif
        services::MethodSerializerFactory::OnHeap serializerFactory;
        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<MessageSize> bufferedSerial{ uart };
    };

    template<std::size_t MessageSize>
    struct EchoOnUart
        : EchoOnUartBase<MessageSize>
    {
        using EchoOnUartBase<MessageSize>::EchoOnUartBase;

        main_::EchoOnSesame<MessageSize> echoOnSesame{ this->bufferedSerial, this->serializerFactory };

        services::Echo& echo{ echoOnSesame.echo };
    };

    template<std::size_t MessageSize, std::size_t MaxServices>
    class EchoForwarder
    {
    public:
        EchoForwarder(services::Echo& from, services::Echo& to)
            : from(from)
            , to(to)
        {}

        void AddService(uint32_t serviceId, uint32_t responseId)
        {
            AddForwarder(from, serviceId, to);
            AddForwarder(to, responseId, from);
        }

        void AddResponse(uint32_t responseId)
        {
            AddForwarder(to, responseId, from);
        }

    private:
        void AddForwarder(services::Echo& forwardFrom, uint32_t id, services::Echo& forwardTo)
        {
            forwarders.emplace_back(forwardFrom, id, forwardTo);
        }

    private:
        services::Echo& from;
        services::Echo& to;

        typename infra::BoundedVector<services::ServiceForwarder>::template WithMaxSize<MaxServices> forwarders;
    };

    template<std::size_t MessageSize, std::size_t MaxServices>
    struct EchoForwarderToSerial
    {
        EchoForwarderToSerial(services::Echo& from, hal::BufferedSerialCommunication& toSerial, services::MethodSerializerFactory& serializerFactory)
            : to(toSerial, serializerFactory)
            , echoForwarder(from, to)
        {}

        EchoOnSesame<MessageSize> to;
        EchoForwarder<MessageSize, MaxServices> echoForwarder;
    };
}

#endif
