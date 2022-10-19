#ifndef SERVICES_CUCUMBER_CONTEXT_HPP
#define SERVICES_CUCUMBER_CONTEXT_HPP

#include "infra/timer/Timer.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/InterfaceConnector.hpp"
#include "infra/util/Function.hpp"

namespace services
{
    class CucumberContext
        : public infra::InterfaceConnector<CucumberContext>
    {
    public:
        infra::TimerSingleShot& TimeoutTimer();

        infra::Function<void()> onSuccess;
        infra::Function<void(infra::BoundedConstString&)> onFailure;

    private:
        infra::TimerSingleShot timeoutTimer;
    };
}

#endif
