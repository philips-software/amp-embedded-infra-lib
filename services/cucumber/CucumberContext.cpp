#include "services/cucumber/CucumberContext.hpp"

namespace services
{
    CucumberContextValue::CucumberContextValue(infra::BoundedConstString key, void *value)
        : key(key)
        , value(value)
    {}

    infra::TimerSingleShot& CucumberContext::TimeoutTimer()
    {
        static infra::TimerSingleShot timeoutTimer;
        return timeoutTimer;
    }

    void CucumberContext::Add(infra::BoundedConstString key, void *value)
    {
        for (auto& var : varStorage)
            if (var.key == key)
                varStorage.erase(&var);
        varStorage.emplace_back(key, value);
    }

    void CucumberContext::Clear()
    {
        varStorage.clear();
    }

    bool CucumberContext::Empty()
    {
        return varStorage.empty();
    }

    bool CucumberContext::Contains(infra::BoundedConstString key)
    {
        for (auto& var : varStorage)
            if (var.key == key)
                return true;
        return false;
    }
}
