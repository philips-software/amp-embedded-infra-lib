#include "infra/timer/Waiting.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace infra
{
    bool WaitUntilDone(const infra::Function<void(const infra::Function<void()>&)>& action, infra::Duration timeout)
    {
        bool done{ false };
        bool timedOut{ false };

        infra::Function<void()> onDone = [&done]()
        {
            done = true;
        };

        action(onDone);

        infra::TimerSingleShot timeoutTimer{ timeout, [&timedOut]()
            {
                timedOut = true;
            } };

        infra::EventDispatcher::Instance().ExecuteUntil([&done, &timedOut]()
            {
                return done || timedOut;
            });

        return !timedOut;
    }

    bool WaitFor(const infra::Function<bool()>& pred, infra::Duration timeout)
    {
        bool timedOut{ false };

        infra::TimerSingleShot timeoutTimer{ timeout, [&timedOut]
            {
                timedOut = true;
            } };

        infra::EventDispatcher::Instance().ExecuteUntil([&pred, &timedOut]()
            {
                return pred() || timedOut;
            });

        return !timedOut;
    }

    namespace host
    {
        bool WaitUntilDone(const std::function<void(const std::function<void()>&)>& action, infra::Duration timeout)
        {
            bool done{ false };
            bool timedOut{ false };

            std::function<void()> onDone = [&done]()
            {
                done = true;
            };

            action(onDone);

            infra::TimerSingleShot timeoutTimer{ timeout, [&timedOut]()
                {
                    timedOut = true;
                } };

            infra::EventDispatcher::Instance().ExecuteUntil([&done, &timedOut]()
                {
                    return done || timedOut;
                });

            return !timedOut;
        }

        bool WaitFor(const std::function<bool()>& pred, infra::Duration timeout)
        {
            bool timedOut{ false };

            infra::TimerSingleShot timeoutTimer{ timeout, [&timedOut]
                {
                    timedOut = true;
                } };

            infra::EventDispatcher::Instance().ExecuteUntil([&pred, &timedOut]()
                {
                    return pred() || timedOut;
                });

            return !timedOut;
        }
    }
}
