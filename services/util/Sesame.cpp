#include "services/util/Sesame.hpp"

namespace services
{
    SesameInitializer immediatelyGranted;

    void SesameInitializer::InitializationRequested(const infra::Function<void()>& onGranted)
    {
        onGranted();
    }

    void SesameInitializer::InitInformationReceived(infra::StreamReaderWithRewinding& initInfo)
    {}

    infra::ConstByteRange SesameInitializer::InitInformation() const
    {
        return {};
    }
}
