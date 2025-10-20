#include "infra/stream/StringOutputStream.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "services/ble/Gap.hpp"
#include "services/ble/test_doubles/GapCentralMock.hpp"
#include "services/ble/test_doubles/GapCentralObserverMock.hpp"
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
        GapAdvertisingReport deviceDiscovered{ GapAdvertisingEventType::advInd, GapDeviceAddressType::publicAddress, hal::MacAddress{ 0, 1, 2, 3, 4, 5 }, infra::BoundedVector<uint8_t>::WithMaxSize<GapPeripheral::maxAdvertisementDataSize>{}, -75 };

        EXPECT_CALL(gapObserver, DeviceDiscovered(ObjectContentsEqual(deviceDiscovered)));

        gap.NotifyObservers([&deviceDiscovered](GapCentralObserver& obs)
            {
                obs.DeviceDiscovered(deviceDiscovered);
            });
    }

    TEST_F(GapCentralDecoratorTest, forward_all_calls_to_subject)
    {
        hal::MacAddress macAddress{ 0, 1, 2, 3, 4, 5 };

        EXPECT_CALL(gap, Connect(MacAddressContentsEqual(macAddress), services::GapDeviceAddressType::publicAddress, infra::Duration{ 0 }));
        decorator.Connect(macAddress, services::GapDeviceAddressType::publicAddress, std::chrono::seconds(0));

        EXPECT_CALL(gap, CancelConnect());
        decorator.CancelConnect();

        EXPECT_CALL(gap, Disconnect());
        decorator.Disconnect();

        EXPECT_CALL(gap, SetAddress(MacAddressContentsEqual(macAddress), GapDeviceAddressType::publicAddress));
        decorator.SetAddress(macAddress, GapDeviceAddressType::publicAddress);

        EXPECT_CALL(gap, StartDeviceDiscovery());
        decorator.StartDeviceDiscovery();

        EXPECT_CALL(gap, StopDeviceDiscovery());
        decorator.StopDeviceDiscovery();

        hal::MacAddress mac = { 0x00, 0x1A, 0x7D, 0xDA, 0x71, 0x13 };
        EXPECT_CALL(gap, ResolvePrivateAddress(mac)).WillOnce(testing::Return(std::nullopt));
        EXPECT_EQ(decorator.ResolvePrivateAddress(mac), std::nullopt);

        EXPECT_CALL(gap, ResolvePrivateAddress(mac)).WillOnce(testing::Return(std::make_optional(mac)));
        EXPECT_EQ(decorator.ResolvePrivateAddress(mac), mac);
    }

    TEST(GapAdvertisingDataParserTest, payload_too_small)
    {
        std::array<uint8_t, 1> data{ { 0x00 } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));

        EXPECT_EQ(infra::ConstByteRange(), gapAdvertisingDataParser.LocalName());
        EXPECT_FALSE(gapAdvertisingDataParser.ManufacturerSpecificData());
    }

    TEST(GapAdvertisingDataParserTest, payload_does_not_contain_valid_info)
    {
        std::array<uint8_t, 2> data{ { 0x03, 0x02 } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));

        EXPECT_EQ(infra::ConstByteRange(), gapAdvertisingDataParser.LocalName());
        EXPECT_FALSE(gapAdvertisingDataParser.ManufacturerSpecificData());
    }

    TEST(GapAdvertisingDataParserTest, payload_does_not_contain_valid_length)
    {
        const std::array<uint8_t, 14> data{ { 0x05, 0xff, 0xaa, 0xbb, 0xcc, 0xdd, 0xaa, 0x09, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67 } };
        const std::array<uint8_t, 2> payloadParser{ { 0xcc, 0xdd } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));
        auto manufacturerSpecificData = gapAdvertisingDataParser.ManufacturerSpecificData();

        EXPECT_EQ(infra::ConstByteRange(), gapAdvertisingDataParser.LocalName());
        EXPECT_TRUE(manufacturerSpecificData);
        EXPECT_EQ(0xbbaa, manufacturerSpecificData->first);
        EXPECT_TRUE(infra::ContentsEqual(infra::MakeRange(payloadParser), manufacturerSpecificData->second));
    }

    TEST(GapAdvertisingDataParserTest, get_local_name_using_type_shortenedLocalName)
    {
        std::array<uint8_t, 5> data{ { 0x04, 0x08, 0x73, 0x74, 0x72 } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));

        EXPECT_EQ("str", ByteRangeAsStdString(gapAdvertisingDataParser.LocalName()));
        EXPECT_FALSE(gapAdvertisingDataParser.ManufacturerSpecificData());
    }

    TEST(GapAdvertisingDataParserTest, get_local_name_using_type_completeLocalName)
    {
        std::array<uint8_t, 8> data{ { 0x07, 0x09, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67 } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));

        EXPECT_EQ("string", ByteRangeAsStdString(gapAdvertisingDataParser.LocalName()));
        EXPECT_FALSE(gapAdvertisingDataParser.ManufacturerSpecificData());
    }

    TEST(GapAdvertisingDataParserTest, get_manufacturer_specific_data)
    {
        const std::array<uint8_t, 6> data{ { 0x05, 0xff, 0xaa, 0xbb, 0xcc, 0xdd } };
        const std::array<uint8_t, 2> payloadParser{ { 0xcc, 0xdd } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));
        auto manufacturerSpecificData = gapAdvertisingDataParser.ManufacturerSpecificData();

        EXPECT_EQ(infra::ConstByteRange(), gapAdvertisingDataParser.LocalName());
        EXPECT_TRUE(manufacturerSpecificData);
        EXPECT_EQ(0xbbaa, manufacturerSpecificData->first);
        EXPECT_TRUE(infra::ContentsEqual(infra::MakeRange(payloadParser), manufacturerSpecificData->second));
    }

    TEST(GapAdvertisingDataParserTest, get_appearance_value)
    {
        const std::array<uint8_t, 4> data = { 0x03, 0x19, 0xC1, 0x03 };

        services::GapAdvertisingDataParser parser(infra::MakeConstByteRange(data));

        auto appearance = parser.Appearance();

        ASSERT_TRUE(appearance);
        EXPECT_EQ(0x03C1, *appearance);
    }

    TEST(GapAdvertisingDataParserTest, useful_info_after_first_ad_structure)
    {
        const std::array<uint8_t, 21> data{ { 0x02, 0x01, 0x06, 0x08, 0x09, 0x70, 0x68, 0x69, 0x6C, 0x69, 0x70, 0x73, 0x02, 0x0a, 0x08, 0x05, 0xff, 0xaa, 0xbb, 0xcc, 0xdd } };
        const std::array<uint8_t, 2> payloadParser{ { 0xcc, 0xdd } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));
        auto manufacturerSpecificData = gapAdvertisingDataParser.ManufacturerSpecificData();

        EXPECT_EQ("philips", ByteRangeAsStdString(gapAdvertisingDataParser.LocalName()));
        EXPECT_TRUE(manufacturerSpecificData);
        EXPECT_EQ(0xbbaa, manufacturerSpecificData->first);
        EXPECT_TRUE(infra::ContentsEqual(infra::MakeRange(payloadParser), manufacturerSpecificData->second));
    }

    TEST(GapAdvertisingDataParserTest, complete_list_of_16bit_services)
    {
        const std::array<uint8_t, 6> data{ { 0x05, 0x03, 0x34, 0x12, 0x78, 0x56 } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));
        auto services = gapAdvertisingDataParser.CompleteListOf16BitUuids();

        ASSERT_EQ(2u, services.size());
        EXPECT_EQ(0x1234, services[0]);
        EXPECT_EQ(0x5678, services[1]);
    }

    TEST(GapAdvertisingDataParserTest, invalid_list_of_16bit_services)
    {
        const std::array<uint8_t, 5> data{ { 0x04, 0x03, 0x34, 0x12, 0x78 } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));
        auto services = gapAdvertisingDataParser.CompleteListOf16BitUuids();

        EXPECT_TRUE(services.empty());
    }

    TEST(GapAdvertisingDataParserTest, complete_list_of_128bit_services)
    {
        const std::array<uint8_t, 22> data{ { 0x11, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f } };
        const std::array<uint8_t, 16> service1{ { 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeRange(data));
        auto services = gapAdvertisingDataParser.CompleteListOf128BitUuids();

        ASSERT_EQ(1u, services.size());
        std::array<uint8_t, 16> parsedUuid = services[0];
        EXPECT_THAT(parsedUuid, testing::ContainerEq(service1));
    }

    TEST(GapAdvertisingDataParserTest, invalid_list_of_128bit_services)
    {
        const std::array<uint8_t, 21> data{ { 0x10, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeRange(data));
        auto services = gapAdvertisingDataParser.CompleteListOf128BitUuids();

        EXPECT_TRUE(services.empty());
    }

    TEST(GapAdvertisingDataParserTest, flags_not_present)
    {
        const std::array<uint8_t, 2> data{ { 0x00, 0x00 } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));

        EXPECT_FALSE(gapAdvertisingDataParser.Flags());
    }

    TEST(GapAdvertisingDataParserTest, flags_present)
    {
        const std::array<uint8_t, 3> data{ { 0x02, 0x01, 0x06 } };
        services::GapAdvertisingDataParser gapAdvertisingDataParser(infra::MakeConstByteRange(data));

        EXPECT_TRUE(gapAdvertisingDataParser.Flags());
        EXPECT_EQ(GapPeripheral::AdvertisementFlags::leGeneralDiscoverableMode | GapPeripheral::AdvertisementFlags::brEdrNotSupported, *gapAdvertisingDataParser.Flags());
    }

    TEST(GapInsertionOperatorEventTypeTest, event_type_overload_operator)
    {
        infra::StringOutputStream::WithStorage<128> stream;

        auto eventTypeAdvInd = services::GapAdvertisingEventType::advInd;
        auto eventTypeAdvDirectInd = services::GapAdvertisingEventType::advDirectInd;
        auto eventTypeAdvScanInd = services::GapAdvertisingEventType::advScanInd;
        auto eventTypeAdvNonconnInd = services::GapAdvertisingEventType::advNonconnInd;
        auto eventTypeScanResponse = services::GapAdvertisingEventType::scanResponse;

        stream << eventTypeAdvInd << " " << eventTypeAdvDirectInd << " " << eventTypeAdvScanInd << " " << eventTypeAdvNonconnInd << " " << eventTypeScanResponse;

        EXPECT_EQ("ADV_IND ADV_DIRECT_IND ADV_SCAN_IND ADV_NONCONN_IND SCAN_RESPONSE", stream.Storage());
    }

    TEST(GapInsertionOperatorEventAddressTypeTest, address_event_type_overload_operator)
    {
        infra::StringOutputStream::WithStorage<128> stream;

        auto eventAddressTypePublicDevice = services::GapDeviceAddressType::publicAddress;
        auto eventAddressTypeRandomDevice = services::GapDeviceAddressType::randomAddress;
        stream << eventAddressTypePublicDevice << " " << eventAddressTypeRandomDevice;

        EXPECT_EQ("Public Device Address Random Device Address", stream.Storage());
    }

    TEST(GapInsertionOperatorStateTest, state_overload_operator)
    {
        infra::StringOutputStream::WithStorage<128> stream;

        stream << services::GapState::standby << " " << services::GapState::scanning << " " << services::GapState::advertising << " " << services::GapState::connected << " " << services::GapState::initiating;

        EXPECT_EQ("Standby Scanning Advertising Connected Initiating", stream.Storage());
    }
}
