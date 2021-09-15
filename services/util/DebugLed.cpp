#include "services/util/DebugLed.hpp"

namespace services
{

    DebugLed::DebugLed(hal::GpioPin& pin, infra::Duration onDuration, infra::Duration offDuration, uint32_t timerServiceId)
        : pin(pin)
        , debugLedTimer(
              offDuration, [this]() { this->pin.Set(true); },
              onDuration, [this]() { this->pin.Set(false); }, timerServiceId)
    {}

}
