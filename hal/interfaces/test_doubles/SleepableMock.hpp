#include "hal/interfaces/Sleepable.hpp"
#include "infra/util/Function.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class SleepableMock
        : public Sleepable
    {
    public:
        MOCK_METHOD(void, Sleep, (const infra::Function<void()>&));
        MOCK_METHOD(void, Wake, (const infra::Function<void()>&));
    };
}
