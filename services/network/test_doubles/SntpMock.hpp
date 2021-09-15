#ifndef NETWORK_SNTP_MOCK_HPP
#define NETWORK_SNTP_MOCK_HPP

#include "services/network/SntpClient.hpp"
#include "gmock/gmock.h"

namespace services
{
    class SntpResultObserverMock
        : public SntpResultObserver
    {
    public:
        using SntpResultObserver::SntpResultObserver;

        MOCK_METHOD2(TimeAvailable, void(infra::Duration, infra::Duration));
        MOCK_METHOD0(TimeUnavailable, void());
        MOCK_METHOD1(KissOfDeath, void(SntpResultObserver::KissCode));
    };
}

#endif
