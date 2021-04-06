#ifndef SERVICES_CUCUMBER_CONTEXT_HPP 
#define SERVICES_CUCUMBER_CONTEXT_HPP

#include "infra/util/BoundedString.hpp"

namespace services
{
    class CucumberContext
    {
    public:

        infra::BoundedString::WithStorage<256> detectedNetwork;
        
        template <class T>
        T Get(uint8_t id);

    };

    template<class T>
    inline T CucumberContext::Get(uint8_t id)
    {
        return T();
    }
}

#endif