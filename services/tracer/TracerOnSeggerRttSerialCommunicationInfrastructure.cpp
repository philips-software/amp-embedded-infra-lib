#include "services/tracer/TracerOnSeggerRttSerialCommunicationInfrastructure.hpp"
#include "services/tracer/GlobalTracer.hpp"

namespace main_
{
    TracerOnSeggerRttSerialCommunicationInfrastructure::TracerOnSeggerRttSerialCommunicationInfrastructure(unsigned int bufferIndex, infra::Duration readInterval)
        : serialCommunicationOnSegger{ bufferIndex, readInterval }
    {
        services::SetGlobalTracerInstance(tracer);
    }
}
