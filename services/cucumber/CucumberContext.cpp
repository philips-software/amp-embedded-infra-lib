#include "services/cucumber/CucumberContext.hpp"

namespace services
{
    CucumberContextValue::CucumberContextValue(infra::BoundedConstString key, void* value)
        : key(key)
        , value(value)
    {}

    infra::TimerSingleShot& CucumberContext::TimeoutTimer()
    {
        static infra::TimerSingleShot timeoutTimer;
        return timeoutTimer;
    }

    void CucumberContext::Add(infra::BoundedConstString key, void* value)
    {
        storage.erase(std::remove_if(storage.begin(), storage.end(), [key](auto& var)
                          { return var.key == key; }),
            storage.end());
        storage.emplace_back(key, value);
    }

    void CucumberContext::Clear()
    {
        storage.clear();
    }

    bool CucumberContext::Empty() const
    {
        return storage.empty();
    }

    bool CucumberContext::Contains(infra::BoundedConstString key) const
    {
        return std::any_of(storage.begin(), storage.end(), [key](auto& var)
            { return var.key == key; });
    }
}
