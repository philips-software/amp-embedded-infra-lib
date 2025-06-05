#ifndef SERVICE_TRACER_ON_SEGGER_RTT_SERIAL_COMMUNICATION_INFRASTRUCTURE_HPP
#define SERVICE_TRACER_ON_SEGGER_RTT_SERIAL_COMMUNICATION_INFRASTRUCTURE_HPP

#include "services/tracer/SerialCommunicationOnSeggerRtt.hpp"
#include "services/tracer/StreamWriterOnSerialCommunication.hpp"
#include "services/tracer/TracerWithDateTime.hpp"

namespace main_
{
    struct TracerOnSeggerRttSerialCommunicationInfrastructure
    {
        TracerOnSeggerRttSerialCommunicationInfrastructure(unsigned int bufferIndex = 0, infra::Duration readInterval = std::chrono::milliseconds(20));

        services::SerialCommunicationOnSeggerRtt serialCommunicationOnSegger;

        services::StreamWriterOnSerialCommunication::WithStorage<128> streamWriter{ serialCommunicationOnSegger };
        infra::TextOutputStream::WithErrorPolicy stream{ streamWriter };
        services::TracerWithDateTime tracer{ stream };
    };
}

#endif
