#ifndef SERVICES_CUCUMBER_CONTEXT_HPP 
#define SERVICES_CUCUMBER_CONTEXT_HPP

#include "infra/util/InterfaceConnector.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/BoundedVector.hpp"

namespace services
{
    class CucumberContextValue
    {
    public:
        CucumberContextValue(infra::BoundedConstString key, void *value);

        infra::BoundedConstString::WithStorage<64> key;
        void *value;
    };

    class CucumberContext
        : public infra::InterfaceConnector<CucumberContext>
    {
    public:

        infra::BoundedVector<CucumberContextValue>::WithMaxSize<512> vector;

        template <class T>
        T Get(infra::BoundedConstString key);

        void Add(infra::BoundedConstString key, void *value);
    };

    template<class T>
    inline T CucumberContext::Get(infra::BoundedConstString key)
    {
        T* ptr = nullptr;
        uint8_t nrMatches = 0;
        for (auto node : vector)
            if (node.key == key)
            {
                ptr = static_cast<T*>(node.value);
                nrMatches++;
            }
        if (nrMatches == 1)
            return *ptr;
        else
            return T();
    }

}

#endif