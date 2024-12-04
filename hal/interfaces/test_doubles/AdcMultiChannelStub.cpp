#include "hal/interfaces/test_doubles/AdcMultiChannelStub.hpp"

namespace hal
{
    AdcMultiChannelStub::AdcMultiChannelStub()
    {
        ON_CALL(*this, Measure).WillByDefault([this](auto onDone)
            {
                this->onDone = onDone;
            });
    }

    void AdcMultiChannelStub::MeasurementDone(std::vector<uint16_t> samples)
    {
        if (onDone)
        {
            onDone(Samples{ &*samples.begin(), &*samples.end() });
        }
    }
}
