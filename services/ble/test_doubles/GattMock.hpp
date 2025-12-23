#ifndef SERVICES_GATT_MOCK_HPP
#define SERVICES_GATT_MOCK_HPP

#include "services/ble/Gatt.hpp"
#include "gmock/gmock.h"

namespace services
{
    class AttMtuExchangeMock
        : public AttMtuExchange
    {
    public:
        MOCK_METHOD(uint16_t, EffectiveAttMtuSize, (), (const override));
    };

    class AttMtuExchangeObserverMock
        : public AttMtuExchangeObserver
    {
    public:
        using AttMtuExchangeObserver::AttMtuExchangeObserver;

        MOCK_METHOD(void, ExchangedAttMtuSize, (), (override));
    };
}

#endif
