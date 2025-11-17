#ifndef SERVICES_GAP_CENTRAL_MOCK_HPP
#define SERVICES_GAP_CENTRAL_MOCK_HPP

#include "services/ble/Gap.hpp"
#include "gmock/gmock.h"

namespace services
{
    class GapCentralMock
        : public GapCentral
    {
    public:
        MOCK_METHOD(void, Connect, (hal::MacAddress macAddress, GapDeviceAddressType addressType, infra::Duration initiatingTimeout));
        MOCK_METHOD(void, CancelConnect, ());
        MOCK_METHOD(void, Disconnect, ());
        MOCK_METHOD(void, SetAddress, (hal::MacAddress macAddress, GapDeviceAddressType addressType));
        MOCK_METHOD(void, StartDeviceDiscovery, ());
        MOCK_METHOD(void, StopDeviceDiscovery, ());
        MOCK_METHOD(std::optional<hal::MacAddress>, ResolvePrivateAddress, (hal::MacAddress address), (const));

        void ChangeState(GapState newState)
        {
            NotifyObservers([newState](auto& observer)
                {
                    observer.StateChanged(newState);
                });
        }
    };
}

#endif
