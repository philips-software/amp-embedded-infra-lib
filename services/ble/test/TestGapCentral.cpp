#include "infra/stream/StringOutputStream.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "services/ble/test_doubles/GapCentralMock.hpp"
#include "services/ble/test_doubles/GapCentralObserverMock.hpp"
#include "services/ble/test_doubles/GapPeripheralMock.hpp"
#include "services/ble/test_doubles/GapPeripheralObserverMock.hpp"
#include "gmock/gmock.h"

namespace services
{
    namespace
    {
        class GapCentralDecoratorTest
            : public testing::Test
        {
        public:
            GapCentralMock gap;
            GapCentralDecorator decorator{ gap };
            GapCentralObserverMock gapObserver{ decorator };
        };
    }

    MATCHER_P(MacAddressContentsEqual, x, negation ? "Contents not equal" : "Contents are equal")
    {
        return infra::ContentsEqual(infra::MakeRange(x), infra::MakeRange(arg));
    }

    MATCHER_P(ObjectContentsEqual, x, negation ? "Contents not equal" : "Contents are equal")
    {
        return x.eventType == arg.eventType && x.addressType == arg.addressType && x.address == arg.address && x.rssi == arg.rssi;
    }

    TEST_F(GapCentralDecoratorTest, forward_all_state_changed_events_to_observers)
    {
        EXPECT_CALL(gapObserver, StateChanged(GapState::connected));
        EXPECT_CALL(gapObserver, StateChanged(GapState::initiating));
        EXPECT_CALL(gapObserver, StateChanged(GapState::scanning));
        EXPECT_CALL(gapObserver, StateChanged(GapState::standby));

        gap.NotifyObservers([](GapCentralObserver& obs)
            {
                obs.StateChanged(GapState::connected);
                obs.StateChanged(GapState::initiating);
                obs.StateChanged(GapState::scanning);
                obs.StateChanged(GapState::standby);
            });
    }

    TEST_F(GapCentralDecoratorTest, forward_device_discovered_event_to_observers)
    {
        GapAdvertisingReport deviceDiscovered{ GapAdvertisingEventType::advInd, GapAdvertisingEventAddressType::publicDeviceAddress, hal::MacAddress{ 0, 1, 2, 3, 4, 5 }, infra::ConstByteRange(), -75 };

        EXPECT_CALL(gapObserver, DeviceDiscovered(ObjectContentsEqual(deviceDiscovered)));

        gap.NotifyObservers([&deviceDiscovered](GapCentralObserver& obs)
            {
                obs.DeviceDiscovered(deviceDiscovered);
            });
    }

    TEST_F(GapCentralDecoratorTest, forward_all_calls_to_subject)
    {
        hal::MacAddress macAddress{ 0, 1, 2, 3, 4, 5 };

        EXPECT_CALL(gap, Connect(MacAddressContentsEqual(macAddress), services::GapDeviceAddressType::publicAddress));
        decorator.Connect(macAddress, services::GapDeviceAddressType::publicAddress);

        EXPECT_CALL(gap, Disconnect());
        decorator.Disconnect();

        EXPECT_CALL(gap, SetAddress(MacAddressContentsEqual(macAddress), GapDeviceAddressType::publicAddress));
        decorator.SetAddress(macAddress, GapDeviceAddressType::publicAddress);

        EXPECT_CALL(gap, StartDeviceDiscovery());
        decorator.StartDeviceDiscovery();

        EXPECT_CALL(gap, StopDeviceDiscovery());
        decorator.StopDeviceDiscovery();
    }

    TEST(GapAdvertisingDataParserTest, payload_too_small)
    {
        std::array<uint8_t, 1> data{ { 0x00 } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));

        EXPECT_EQ(infra::ConstByteRange(), gapAdvertisingDataParser.LocalName());
        EXPECT_EQ(infra::ConstByteRange(), gapAdvertisingDataParser.ManufacturerSpecificData());
    }

    TEST(GapAdvertisingDataParserTest, payload_does_not_contain_valid_info)
    {
        std::array<uint8_t, 2> data{ { 0x03, 0x02 } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));

        EXPECT_EQ(infra::ConstByteRange(), gapAdvertisingDataParser.LocalName());
        EXPECT_EQ(infra::ConstByteRange(), gapAdvertisingDataParser.ManufacturerSpecificData());
    }

    TEST(GapAdvertisingDataParserTest, payload_does_not_contain_valid_length)
    {
        std::array<uint8_t, 14> data{ { 0x05, 0xff, 0xaa, 0xbb, 0xcc, 0xdd, 0xaa, 0x09, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67 } };
        std::array<uint8_t, 4> payloadParser{ { 0xaa, 0xbb, 0xcc, 0xdd } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));

        EXPECT_EQ(infra::ConstByteRange(), gapAdvertisingDataParser.LocalName());
        EXPECT_TRUE(infra::ContentsEqual(infra::MakeConstByteRange(payloadParser), gapAdvertisingDataParser.ManufacturerSpecificData()));
    }

    TEST(GapAdvertisingDataParserTest, get_local_name_using_type_shortenedLocalName)
    {
        std::array<uint8_t, 5> data{ { 0x04, 0x08, 0x73, 0x74, 0x72 } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));

        EXPECT_EQ("str", ByteRangeAsStdString(gapAdvertisingDataParser.LocalName()));
        EXPECT_EQ(infra::ConstByteRange(), gapAdvertisingDataParser.ManufacturerSpecificData());
    }

    TEST(GapAdvertisingDataParserTest, get_local_name_using_type_completeLocalName)
    {
        std::array<uint8_t, 8> data{ { 0x07, 0x09, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67 } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));

        EXPECT_EQ("string", ByteRangeAsStdString(gapAdvertisingDataParser.LocalName()));
        EXPECT_EQ(infra::ConstByteRange(), gapAdvertisingDataParser.ManufacturerSpecificData());
    }

    TEST(GapAdvertisingDataParserTest, get_manufacturer_specific_data)
    {
        std::array<uint8_t, 6> data{ { 0x05, 0xff, 0xaa, 0xbb, 0xcc, 0xdd } };
        std::array<uint8_t, 4> payloadParser{ { 0xaa, 0xbb, 0xcc, 0xdd } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));

        EXPECT_EQ(infra::ConstByteRange(), gapAdvertisingDataParser.LocalName());
        EXPECT_TRUE(infra::ContentsEqual(infra::MakeConstByteRange(payloadParser), gapAdvertisingDataParser.ManufacturerSpecificData()));
    }

    TEST(GapAdvertisingDataParserTest, useful_info_after_first_ad_structure)
    {
        std::array<uint8_t, 21> data{ { 0x02, 0x01, 0x06, 0x08, 0x09, 0x70, 0x68, 0x69, 0x6C, 0x69, 0x70, 0x73, 0x02, 0x0a, 0x08, 0x05, 0xff, 0xaa, 0xbb, 0xcc, 0xdd } };
        std::array<uint8_t, 4> payloadParser{ { 0xaa, 0xbb, 0xcc, 0xdd } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));

        EXPECT_EQ("philips", ByteRangeAsStdString(gapAdvertisingDataParser.LocalName()));
        EXPECT_TRUE(infra::ContentsEqual(infra::MakeConstByteRange(payloadParser), gapAdvertisingDataParser.ManufacturerSpecificData()));
    }

    TEST(GapInsertionOperatorEventTypeTest, event_type_overload_operator)
    {
        infra::StringOutputStream::WithStorage<128> stream;

        services::GapAdvertisingEventType eventTypeAdvInd = services::GapAdvertisingEventType::advInd;
        services::GapAdvertisingEventType eventTypeAdvDirectInd = services::GapAdvertisingEventType::advDirectInd;
        services::GapAdvertisingEventType eventTypeAdvScanInd = services::GapAdvertisingEventType::advScanInd;
        services::GapAdvertisingEventType eventTypeAdvNonconnInd = services::GapAdvertisingEventType::advNonconnInd;
        services::GapAdvertisingEventType eventTypeScanResponse = services::GapAdvertisingEventType::scanResponse;

        stream << eventTypeAdvInd << " " << eventTypeAdvDirectInd << " " << eventTypeAdvScanInd << " " << eventTypeAdvNonconnInd << " " << eventTypeScanResponse;

        EXPECT_EQ("ADV_IND ADV_DIRECT_IND ADV_SCAN_IND ADV_NONCONN_IND SCAN_RESPONSE", stream.Storage());
    }

    TEST(GapInsertionOperatorEventAddressTypeTest, address_event_type_overload_operator)
    {
        infra::StringOutputStream::WithStorage<128> stream;

        services::GapAdvertisingEventAddressType eventAddressTypePublicDevice = services::GapAdvertisingEventAddressType::publicDeviceAddress;
        services::GapAdvertisingEventAddressType eventAddressTypeRandomDevice = services::GapAdvertisingEventAddressType::randomDeviceAddress;
        services::GapAdvertisingEventAddressType eventAddressTypePublicIdentity = services::GapAdvertisingEventAddressType::publicIdentityAddress;
        services::GapAdvertisingEventAddressType eventAddressTypeRandomIdentity = services::GapAdvertisingEventAddressType::randomIdentityAddress;

        stream << eventAddressTypePublicDevice << " " << eventAddressTypeRandomDevice << " " << eventAddressTypePublicIdentity << " " << eventAddressTypeRandomIdentity;

        EXPECT_EQ("Public Device Address Random Device Address Public Identity Address Random Identity Address", stream.Storage());
    }

    TEST(GapInsertionOperatorStateTest, state_overload_operator)
    {
        infra::StringOutputStream::WithStorage<128> stream;

        stream << services::GapState::standby << " " << services::GapState::scanning << " " << services::GapState::advertising << " " << services::GapState::connected << " " << services::GapState::initiating;

        EXPECT_EQ("Standby Scanning Advertising Connected Initiating", stream.Storage());
    }
}
