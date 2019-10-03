#include "hal/synchronous_interfaces/test_doubles/SynchronousFixedRandomDataGenerator.hpp"

namespace hal
{
    SynchronousFixedRandomDataGenerator::SynchronousFixedRandomDataGenerator(const std::vector<uint8_t>& initialData)
        : initialData(initialData)
    {}

    void SynchronousFixedRandomDataGenerator::GenerateRandomData(infra::ByteRange result)
    {
        std::fill(result.begin(), result.end(), 0);
        std::copy(initialData.begin(), initialData.begin() + std::min(initialData.size(), result.size()), result.begin());
        initialData.resize(initialData.size() - std::min(initialData.size(), result.size()));
    }
}
