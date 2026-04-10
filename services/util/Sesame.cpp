#include "services/util/Sesame.hpp"

namespace services
{
    SesameInitializer immediatelyGranted;

    void SesameInitializer::InitializationRequested(const infra::Function<void()>& onGranted)
    {
        onGranted();
    }
}
