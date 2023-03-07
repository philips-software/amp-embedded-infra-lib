#ifndef SERVICES_CUCUMBER_CONTEXT_HPP
#define SERVICES_CUCUMBER_CONTEXT_HPP

#include "infra/timer/Timer.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/InterfaceConnector.hpp"

namespace services
{
    class CucumberContext
        : public infra::InterfaceConnector<CucumberContext>
    {
    public:
        infra::TimerSingleShot& TimeoutTimer();

        infra::AutoResetFunction<void()> onSuccess;
        infra::AutoResetFunction<void(infra::BoundedConstString&)> onFailure;

    private:
        infra::TimerSingleShot timeoutTimer;
    };
}

#endif
