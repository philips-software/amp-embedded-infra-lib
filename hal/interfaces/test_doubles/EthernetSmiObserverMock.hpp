#ifndef HAL_ETHERNET_SMI_OBSERVER_MOCK_HPP
#define HAL_ETHERNET_SMI_OBSERVER_MOCK_HPP

#include "hal/interfaces/Ethernet.hpp"
#include <gmock/gmock.h>

namespace hal
{
    class EthernetSmiObserverMock
        : public EthernetSmiObserver
    {
    public:
        using EthernetSmiObserver::EthernetSmiObserver;

        MOCK_METHOD(void, LinkUp, (LinkSpeed linkSpeed), (override));
        MOCK_METHOD(void, LinkDown, (), (override));
    };
}

#endif
