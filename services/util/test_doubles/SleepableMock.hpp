#include "infra/util/Function.hpp"
#include "services/util/Sleepable.hpp"
#include "gmock/gmock.h"

namespace services
{
    class SleepableMock : public Sleepable
    {
    public:
        MOCK_METHOD(void, Sleep, (const infra::Function<void()>&));
        MOCK_METHOD(void, Wake, (const infra::Function<void()>&));
    };
}
