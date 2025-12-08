#ifndef SERVICES_GATT_MOCK_HPP
#define SERVICES_GATT_MOCK_HPP

#include "services/ble/Gatt.hpp"
#include "gmock/gmock.h"

namespace services
{
    class AttMtuExchangeReceiverMock
        : public AttMtuExchange
    {
    public:
        MOCK_METHOD(uint16_t, EffectiveMaxAttMtuSize, (), (const override));
    };

    class AttMtuExchangeReceiverObserverMock
        : public AttMtuExchangeObserver
    {
    public:
        using AttMtuExchangeObserver::AttMtuExchangeReceiverObserver;

        MOCK_METHOD(void, ExchangedMaxAttMtuSize, (), (override));
    };
}

#endif
