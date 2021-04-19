#include "services/cucumber/CucumberContext.hpp"

namespace services
{
    CucumberContextValue::CucumberContextValue(infra::BoundedConstString key, void *value)
        : key(key)
        , value(value)
    {}

    void CucumberContext::Add(infra::BoundedConstString key, void *value)
    {
        vector.push_back(CucumberContextValue(key, value));
    }

    void CucumberContext::Clear()
    {
        vector.clear();
    }

    bool CucumberContext::Contains(infra::BoundedConstString key)
    {
        for (auto var : vector)
            if (var.key == key)
                return true;
    }
}