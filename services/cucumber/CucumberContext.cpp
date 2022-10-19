#include "services/cucumber/CucumberContext.hpp"

namespace services
{
    infra::TimerSingleShot& CucumberContext::TimeoutTimer()
    {
        return timeoutTimer;
    }
}
