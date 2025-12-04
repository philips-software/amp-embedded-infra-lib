#ifndef SERVICES_GATT_MOCK_HPP
#define SERVICES_GATT_MOCK_HPP

#include "services/ble/Gatt.hpp"
#include "gmock/gmock.h"

namespace services
{
    class AttMtuExchangeReceiverMock
        : public AttMtuExchangeReceiver
    {
    public:
        MOCK_METHOD(uint16_t, EffectiveMaxAttMtuSize, (), (const override));
    };

    class AttMtuExchangeReceiverObserverMock
        : public AttMtuExchangeReceiverObserver
    {
    public:
        using AttMtuExchangeReceiverObserver::AttMtuExchangeReceiverObserver;

        MOCK_METHOD(void, ExchangedMaxAttMtuSize, (), (override));
    };
}

#endif
