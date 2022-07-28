#include <algorithm>
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"

namespace hal
{
    void SynchronousRandomDataGeneratorGeneric::GenerateRandomData(infra::ByteRange result)
    {
        std::generate(result.begin(), result.end(), [this] { return distribution(randomDevice); });
    }
}
