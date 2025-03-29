#include "services/util/MbedTlsRandomDataGeneratorWrapper.hpp"
#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"

namespace services
{
    int MbedTlsRandomDataGeneratorWrapper(void* data, unsigned char* output, std::size_t size)
    {
        reinterpret_cast<hal::SynchronousRandomDataGenerator*>(data)->GenerateRandomData(infra::ByteRange(output, output + size));
        return 0;
    }
}
