#ifndef SERVICES_CUCUMBER_CONTEXT_HPP
#define SERVICES_CUCUMBER_CONTEXT_HPP

#include "infra/timer/Timer.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/InterfaceConnector.hpp"

namespace services
{
    class CucumberContextValue
    {
    public:
        CucumberContextValue(infra::BoundedConstString key, void* value);
        CucumberContextValue& operator=(const CucumberContextValue& other) = default;
        CucumberContextValue(CucumberContextValue& other) = delete;
        virtual ~CucumberContextValue() = default;

        infra::BoundedConstString key;
        void* value;
    };

    class CucumberContext
        : public infra::InterfaceConnector<CucumberContext>
    {
    public:
        CucumberContext() = default;
        CucumberContext& operator=(const CucumberContext& other) = delete;
        CucumberContext(CucumberContext& other) = delete;
        virtual ~CucumberContext() = default;

        template<class T>
        T& Get(infra::BoundedConstString key);
        void Add(infra::BoundedConstString key, void* value);
        void Clear();
        bool Empty() const;
        bool Contains(infra::BoundedConstString key) const;

        infra::TimerSingleShot& TimeoutTimer();

    private:
        infra::BoundedVector<CucumberContextValue>::WithMaxSize<32> storage;
    };

    template<class T>
    inline T& CucumberContext::Get(infra::BoundedConstString key)
    {
        for (auto& node : storage)
            if (node.key == key)
                return *(static_cast<T*>(node.value));
        std::abort();
    }
}

#endif
