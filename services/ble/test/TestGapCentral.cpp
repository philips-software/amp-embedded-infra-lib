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
        return x.reportType == arg.reportType && x.gapAddress == arg.gapAddress && x.rssi == arg.rssi;
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
        GapAdvertisingReport deviceDiscovered{
            AdvertisingReportType::advInd,
            GapAddress{ hal::MacAddress{ 0, 1, 2, 3, 4, 5 }, GapDeviceAddressType::publicAddress },
            infra::BoundedVector<uint8_t>::WithMaxSize<GapPeripheral::maxAdvertisementDataSize>{},
            -75
        };

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

        EXPECT_CALL(gap, Standby());
        decorator.Standby();

        const services::GapAddress identityAddress{ macAddress, GapDeviceAddressType::publicAddress };
        EXPECT_CALL(gap, SetIdentityAddress(identityAddress));
        decorator.SetIdentityAddress(identityAddress);

        EXPECT_CALL(gap, StartDeviceDiscovery());
        decorator.StartDeviceDiscovery();

        hal::MacAddress mac = { 0x00, 0x1A, 0x7D, 0xDA, 0x71, 0x13 };
        EXPECT_CALL(gap, ResolvePrivateAddress(mac)).WillOnce(testing::Return(std::nullopt));
        EXPECT_EQ(decorator.ResolvePrivateAddress(mac), std::nullopt);

        EXPECT_CALL(gap, ResolvePrivateAddress(mac)).WillOnce(testing::Return(std::make_optional(mac)));
        EXPECT_EQ(decorator.ResolvePrivateAddress(mac), mac);

        EXPECT_CALL(gap, SetDeviceName(infra::BoundedConstString("central")));
        decorator.SetDeviceName("central");
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

    TEST(GapAdvertisingDataParserTest, contains_service_uuid_finds_complete_16bit)
    {
        const std::array<uint8_t, 4> data{ { 0x03, 0x03, 0x34, 0x12 } };
        const std::array<uint8_t, 2> uuid{ { 0x34, 0x12 } };
        services::GapAdvertisingDataParser parser(infra::MakeConstByteRange(data));

        EXPECT_THAT(parser.ContainsServiceUuid(infra::MakeConstByteRange(uuid)), testing::IsTrue());
    }

    TEST(GapAdvertisingDataParserTest, contains_service_uuid_finds_incomplete_16bit)
    {
        const std::array<uint8_t, 4> data{ { 0x03, 0x02, 0x34, 0x12 } };
        const std::array<uint8_t, 2> uuid{ { 0x34, 0x12 } };
        services::GapAdvertisingDataParser parser(infra::MakeConstByteRange(data));

        EXPECT_THAT(parser.ContainsServiceUuid(infra::MakeConstByteRange(uuid)), testing::IsTrue());
    }

    TEST(GapAdvertisingDataParserTest, contains_service_uuid_finds_complete_32bit)
    {
        const std::array<uint8_t, 6> data{ { 0x05, 0x05, 0x01, 0x02, 0x03, 0x04 } };
        const std::array<uint8_t, 4> uuid{ { 0x01, 0x02, 0x03, 0x04 } };
        services::GapAdvertisingDataParser parser(infra::MakeConstByteRange(data));

        EXPECT_THAT(parser.ContainsServiceUuid(infra::MakeConstByteRange(uuid)), testing::IsTrue());
    }

    TEST(GapAdvertisingDataParserTest, contains_service_uuid_finds_incomplete_32bit)
    {
        const std::array<uint8_t, 6> data{ { 0x05, 0x04, 0x01, 0x02, 0x03, 0x04 } };
        const std::array<uint8_t, 4> uuid{ { 0x01, 0x02, 0x03, 0x04 } };
        services::GapAdvertisingDataParser parser(infra::MakeConstByteRange(data));

        EXPECT_THAT(parser.ContainsServiceUuid(infra::MakeConstByteRange(uuid)), testing::IsTrue());
    }

    TEST(GapAdvertisingDataParserTest, contains_service_uuid_finds_complete_128bit)
    {
        const std::array<uint8_t, 18> data{ { 0x11, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f } };
        const std::array<uint8_t, 16> uuid{ { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f } };
        services::GapAdvertisingDataParser parser(infra::MakeConstByteRange(data));

        EXPECT_THAT(parser.ContainsServiceUuid(infra::MakeConstByteRange(uuid)), testing::IsTrue());
    }

    TEST(GapAdvertisingDataParserTest, contains_service_uuid_finds_incomplete_128bit)
    {
        const std::array<uint8_t, 18> data{ { 0x11, 0x06, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f } };
        const std::array<uint8_t, 16> uuid{ { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f } };
        services::GapAdvertisingDataParser parser(infra::MakeConstByteRange(data));

        EXPECT_THAT(parser.ContainsServiceUuid(infra::MakeConstByteRange(uuid)), testing::IsTrue());
    }

    TEST(GapAdvertisingDataParserTest, contains_service_uuid_finds_second_entry_in_list)
    {
        const std::array<uint8_t, 6> data{ { 0x05, 0x03, 0x34, 0x12, 0x78, 0x56 } };
        const std::array<uint8_t, 2> uuid{ { 0x78, 0x56 } };
        services::GapAdvertisingDataParser parser(infra::MakeConstByteRange(data));

        EXPECT_THAT(parser.ContainsServiceUuid(infra::MakeConstByteRange(uuid)), testing::IsTrue());
    }

    TEST(GapAdvertisingDataParserTest, contains_service_uuid_finds_uuid_after_other_ad_structures)
    {
        const std::array<uint8_t, 12> data{ { 0x02, 0x01, 0x06, 0x04, 0x09, 0x61, 0x62, 0x63, 0x03, 0x03, 0x34, 0x12 } };
        const std::array<uint8_t, 2> uuid{ { 0x34, 0x12 } };
        services::GapAdvertisingDataParser parser(infra::MakeConstByteRange(data));

        EXPECT_THAT(parser.ContainsServiceUuid(infra::MakeConstByteRange(uuid)), testing::IsTrue());
    }

    TEST(GapAdvertisingDataParserTest, contains_service_uuid_rejects_missing_uuid)
    {
        const std::array<uint8_t, 6> data{ { 0x05, 0x03, 0x34, 0x12, 0x78, 0x56 } };
        const std::array<uint8_t, 2> uuid{ { 0x12, 0x34 } };
        services::GapAdvertisingDataParser parser(infra::MakeConstByteRange(data));

        EXPECT_THAT(parser.ContainsServiceUuid(infra::MakeConstByteRange(uuid)), testing::IsFalse());
    }

    TEST(GapAdvertisingDataParserTest, contains_service_uuid_rejects_same_bytes_of_other_size)
    {
        const std::array<uint8_t, 6> data{ { 0x05, 0x03, 0x34, 0x12, 0x78, 0x56 } };
        const std::array<uint8_t, 4> uuid{ { 0x34, 0x12, 0x78, 0x56 } };
        services::GapAdvertisingDataParser parser(infra::MakeConstByteRange(data));

        EXPECT_THAT(parser.ContainsServiceUuid(infra::MakeConstByteRange(uuid)), testing::IsFalse());
    }

    TEST(GapAdvertisingDataParserTest, contains_service_uuid_rejects_list_with_invalid_length)
    {
        const std::array<uint8_t, 5> data{ { 0x04, 0x03, 0x34, 0x12, 0x78 } };
        const std::array<uint8_t, 2> uuid{ { 0x34, 0x12 } };
        services::GapAdvertisingDataParser parser(infra::MakeConstByteRange(data));

        EXPECT_THAT(parser.ContainsServiceUuid(infra::MakeConstByteRange(uuid)), testing::IsFalse());
    }

    TEST(GapAdvertisingDataParserTest, contains_service_uuid_rejects_unsupported_uuid_size)
    {
        const std::array<uint8_t, 6> data{ { 0x05, 0x03, 0x34, 0x12, 0x78, 0x56 } };
        const std::array<uint8_t, 3> uuid{ { 0x34, 0x12, 0x78 } };
        services::GapAdvertisingDataParser parser(infra::MakeConstByteRange(data));

        EXPECT_THAT(parser.ContainsServiceUuid(infra::MakeConstByteRange(uuid)), testing::IsFalse());
    }

    TEST(GapAdvertisingDataParserTest, contains_service_uuid_rejects_empty_payload)
    {
        const std::array<uint8_t, 2> uuid{ { 0x34, 0x12 } };
        services::GapAdvertisingDataParser parser{ infra::ConstByteRange() };

        EXPECT_THAT(parser.ContainsServiceUuid(infra::MakeConstByteRange(uuid)), testing::IsFalse());
    }

    namespace
    {
        class GapDeviceDiscoveryFilterTest
            : public testing::Test
        {
        public:
            GapAdvertisingReport Report(infra::ConstByteRange advertisingData = infra::ConstByteRange(), GapDeviceAddressType addressType = GapDeviceAddressType::publicAddress) const
            {
                GapAdvertisingReport report{ AdvertisingReportType::advInd, GapAddress{ reportAddress, addressType }, {}, 0 };
                report.data.insert(report.data.end(), advertisingData.begin(), advertisingData.end());
                return report;
            }

            GapAdvertisingReport ReportWithManufacturerData(uint16_t manufacturerId, infra::ConstByteRange manufacturerData) const
            {
                infra::BoundedVector<uint8_t>::WithMaxSize<GapPeripheral::maxAdvertisementDataSize> payload;
                GapAdvertisementFormatter formatter{ payload };
                formatter.AppendManufacturerData(manufacturerId, manufacturerData);
                return Report(formatter.FormattedAdvertisementData());
            }

            GapAdvertisingReport ReportWithServiceList(GapAdvertisementDataType listType, infra::ConstByteRange uuids) const
            {
                infra::BoundedVector<uint8_t>::WithMaxSize<GapPeripheral::maxAdvertisementDataSize> payload;
                GapAdvertisementFormatter formatter{ payload };
                formatter.AppendListOfServicesUuid(listType, uuids);
                return Report(formatter.FormattedAdvertisementData());
            }

            GapDeviceDiscoveryFilter ServiceUuidFilter(const std::array<uint8_t, 2>& uuid) const
            {
                return GapDeviceDiscoveryFilter(GapDeviceDiscoveryFilter::Type::serviceUuid, infra::MakeConstByteRange(uuid));
            }

            const hal::MacAddress reportAddress{ { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 } };
            const hal::MacAddress otherAddress{ { 0x01, 0x02, 0x03, 0x04, 0x05, 0x07 } };
            const std::array<uint8_t, 2> uuid16{ { 0x34, 0x12 } };
            const std::array<uint8_t, 4> uuid32{ { 0x01, 0x02, 0x03, 0x04 } };
            const std::array<uint8_t, 16> uuid128{ { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f } };
            const std::array<uint8_t, 2> manufacturerIdBytes{ { 0xaa, 0xbb } };
            const std::array<uint8_t, 4> manufacturerFilterData{ { 0xaa, 0xbb, 0xcc, 0xdd } };
            const std::array<uint8_t, 2> manufacturerPayload{ { 0xcc, 0xdd } };
            const std::array<uint8_t, 2> otherPayload{ { 0xcc, 0xde } };
            const uint16_t manufacturerId = 0xbbaa;
        };
    }

    TEST_F(GapDeviceDiscoveryFilterTest, default_filter_passes_every_report)
    {
        GapDeviceDiscoveryFilter filter;

        EXPECT_THAT(filter.Passes(Report(infra::MakeConstByteRange(uuid16))), testing::IsTrue());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, none_with_data_passes_every_report)
    {
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::none, infra::MakeConstByteRange(uuid128));

        EXPECT_THAT(filter.Passes(Report(infra::MakeConstByteRange(uuid16))), testing::IsTrue());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, none_with_oversized_data_does_not_abort)
    {
        const std::array<uint8_t, 40> oversizedData{};
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::none, infra::MakeConstByteRange(oversizedData));

        EXPECT_THAT(filter.Passes(Report()), testing::IsTrue());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, none_with_one_byte_of_data_does_not_abort)
    {
        const std::array<uint8_t, 1> data{ { 0x01 } };
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::none, infra::MakeConstByteRange(data));

        EXPECT_THAT(filter.Passes(Report()), testing::IsTrue());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, address_filter_passes_report_with_same_address)
    {
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::address, infra::MakeConstByteRange(reportAddress));

        EXPECT_THAT(filter.Passes(Report()), testing::IsTrue());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, address_filter_rejects_report_with_other_address)
    {
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::address, infra::MakeConstByteRange(otherAddress));

        EXPECT_THAT(filter.Passes(Report()), testing::IsFalse());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, address_filter_passes_report_with_same_address_of_other_address_type)
    {
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::address, infra::MakeConstByteRange(reportAddress));

        EXPECT_THAT(filter.Passes(Report(infra::ConstByteRange(), GapDeviceAddressType::randomAddress)), testing::IsTrue());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, address_with_data_shorter_than_6_bytes_matches_nothing)
    {
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::address, infra::Head(infra::MakeConstByteRange(reportAddress), 5));

        EXPECT_THAT(filter.Passes(Report()), testing::IsFalse());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, address_with_data_longer_than_6_bytes_matches_nothing)
    {
        const std::array<uint8_t, 7> longerAddress{ { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 } };
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::address, infra::MakeConstByteRange(longerAddress));

        EXPECT_THAT(filter.Passes(Report()), testing::IsFalse());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, address_with_data_of_maximum_size_matches_nothing)
    {
        const std::array<uint8_t, GapPeripheral::maxAdvertisementDataSize> data{};
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::address, infra::MakeConstByteRange(data));

        EXPECT_THAT(filter.Passes(Report()), testing::IsFalse());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, manufacturer_filter_passes_report_with_same_id_and_data)
    {
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::manufacturerSpecificData, infra::MakeConstByteRange(manufacturerFilterData));

        EXPECT_THAT(filter.Passes(ReportWithManufacturerData(manufacturerId, infra::MakeConstByteRange(manufacturerPayload))), testing::IsTrue());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, manufacturer_filter_rejects_report_with_other_id)
    {
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::manufacturerSpecificData, infra::MakeConstByteRange(manufacturerFilterData));

        EXPECT_THAT(filter.Passes(ReportWithManufacturerData(0xaabb, infra::MakeConstByteRange(manufacturerPayload))), testing::IsFalse());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, manufacturer_filter_rejects_report_with_other_data)
    {
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::manufacturerSpecificData, infra::MakeConstByteRange(manufacturerFilterData));

        EXPECT_THAT(filter.Passes(ReportWithManufacturerData(manufacturerId, infra::MakeConstByteRange(otherPayload))), testing::IsFalse());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, manufacturer_filter_rejects_report_without_manufacturer_data)
    {
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::manufacturerSpecificData, infra::MakeConstByteRange(manufacturerFilterData));

        EXPECT_THAT(filter.Passes(ReportWithServiceList(GapAdvertisementDataType::completeListOf16BitUuids, infra::MakeConstByteRange(uuid16))), testing::IsFalse());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, manufacturer_filter_with_id_only_passes_report_without_payload)
    {
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::manufacturerSpecificData, infra::MakeConstByteRange(manufacturerIdBytes));

        EXPECT_THAT(filter.Passes(ReportWithManufacturerData(manufacturerId, infra::ConstByteRange())), testing::IsTrue());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, manufacturer_filter_with_id_only_rejects_report_with_payload)
    {
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::manufacturerSpecificData, infra::MakeConstByteRange(manufacturerIdBytes));

        EXPECT_THAT(filter.Passes(ReportWithManufacturerData(manufacturerId, infra::MakeConstByteRange(manufacturerPayload))), testing::IsFalse());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, service_uuid_filter_passes_report_with_incomplete_16bit_list)
    {
        EXPECT_THAT(ServiceUuidFilter(uuid16).Passes(ReportWithServiceList(GapAdvertisementDataType::incompleteListOf16BitUuids, infra::MakeConstByteRange(uuid16))), testing::IsTrue());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, service_uuid_filter_passes_report_with_complete_16bit_list)
    {
        EXPECT_THAT(ServiceUuidFilter(uuid16).Passes(ReportWithServiceList(GapAdvertisementDataType::completeListOf16BitUuids, infra::MakeConstByteRange(uuid16))), testing::IsTrue());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, service_uuid_filter_passes_report_with_incomplete_32bit_list)
    {
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::serviceUuid, infra::MakeConstByteRange(uuid32));

        EXPECT_THAT(filter.Passes(ReportWithServiceList(GapAdvertisementDataType::incompleteListOf32BitUuids, infra::MakeConstByteRange(uuid32))), testing::IsTrue());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, service_uuid_filter_passes_report_with_complete_32bit_list)
    {
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::serviceUuid, infra::MakeConstByteRange(uuid32));

        EXPECT_THAT(filter.Passes(ReportWithServiceList(GapAdvertisementDataType::completeListOf32BitUuids, infra::MakeConstByteRange(uuid32))), testing::IsTrue());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, service_uuid_filter_passes_report_with_incomplete_128bit_list)
    {
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::serviceUuid, infra::MakeConstByteRange(uuid128));

        EXPECT_THAT(filter.Passes(ReportWithServiceList(GapAdvertisementDataType::incompleteListOf128BitUuids, infra::MakeConstByteRange(uuid128))), testing::IsTrue());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, service_uuid_filter_passes_report_with_complete_128bit_list)
    {
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::serviceUuid, infra::MakeConstByteRange(uuid128));

        EXPECT_THAT(filter.Passes(ReportWithServiceList(GapAdvertisementDataType::completeListOf128BitUuids, infra::MakeConstByteRange(uuid128))), testing::IsTrue());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, service_uuid_filter_rejects_report_with_other_uuid)
    {
        const std::array<uint8_t, 2> otherUuid{ { 0x12, 0x34 } };

        EXPECT_THAT(ServiceUuidFilter(otherUuid).Passes(ReportWithServiceList(GapAdvertisementDataType::completeListOf16BitUuids, infra::MakeConstByteRange(uuid16))), testing::IsFalse());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, service_uuid_filter_rejects_report_with_same_bytes_in_list_of_other_size)
    {
        const std::array<uint8_t, 4> uuid16Twice{ { 0x34, 0x12, 0x34, 0x12 } };
        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::serviceUuid, infra::MakeConstByteRange(uuid16Twice));

        EXPECT_THAT(filter.Passes(ReportWithServiceList(GapAdvertisementDataType::completeListOf16BitUuids, infra::MakeConstByteRange(uuid16Twice))), testing::IsFalse());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, service_uuid_filter_rejects_report_without_service_list)
    {
        EXPECT_THAT(ServiceUuidFilter(uuid16).Passes(ReportWithManufacturerData(manufacturerId, infra::MakeConstByteRange(manufacturerPayload))), testing::IsFalse());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, copied_filter_keeps_type_and_data)
    {
        GapDeviceDiscoveryFilter filter;
        filter = ServiceUuidFilter(uuid16);

        EXPECT_THAT(filter.Passes(ReportWithServiceList(GapAdvertisementDataType::completeListOf16BitUuids, infra::MakeConstByteRange(uuid16))), testing::IsTrue());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, assigned_default_filter_passes_every_report)
    {
        GapDeviceDiscoveryFilter filter = ServiceUuidFilter(uuid16);
        filter = GapDeviceDiscoveryFilter();

        EXPECT_THAT(filter.Passes(Report()), testing::IsTrue());
    }

    TEST_F(GapDeviceDiscoveryFilterTest, manufacturer_filter_accepts_data_of_31_bytes)
    {
        const std::array<uint8_t, GapPeripheral::maxAdvertisementDataSize> data{};

        GapDeviceDiscoveryFilter filter(GapDeviceDiscoveryFilter::Type::manufacturerSpecificData, infra::MakeConstByteRange(data));

        EXPECT_THAT(filter.Passes(Report()), testing::IsFalse());
    }

#ifndef EMIL_MUTATION_TESTING
    TEST_F(GapDeviceDiscoveryFilterTest, manufacturer_filter_asserts_on_data_shorter_than_2_bytes)
    {
        const std::array<uint8_t, 1> data{ { 0xaa } };

        EXPECT_DEATH(GapDeviceDiscoveryFilter(GapDeviceDiscoveryFilter::Type::manufacturerSpecificData, infra::MakeConstByteRange(data)), "");
    }

    TEST_F(GapDeviceDiscoveryFilterTest, manufacturer_filter_asserts_on_empty_data)
    {
        EXPECT_DEATH(GapDeviceDiscoveryFilter(GapDeviceDiscoveryFilter::Type::manufacturerSpecificData, infra::ConstByteRange()), "");
    }

    TEST_F(GapDeviceDiscoveryFilterTest, manufacturer_filter_asserts_on_data_longer_than_31_bytes)
    {
        const std::array<uint8_t, GapPeripheral::maxAdvertisementDataSize + 1> data{};

        EXPECT_DEATH(GapDeviceDiscoveryFilter(GapDeviceDiscoveryFilter::Type::manufacturerSpecificData, infra::MakeConstByteRange(data)), "");
    }

    TEST_F(GapDeviceDiscoveryFilterTest, address_filter_asserts_on_data_longer_than_31_bytes)
    {
        const std::array<uint8_t, GapPeripheral::maxAdvertisementDataSize + 1> data{};

        EXPECT_DEATH(GapDeviceDiscoveryFilter(GapDeviceDiscoveryFilter::Type::address, infra::MakeConstByteRange(data)), "");
    }

    TEST_F(GapDeviceDiscoveryFilterTest, service_uuid_filter_asserts_on_empty_data)
    {
        EXPECT_DEATH(GapDeviceDiscoveryFilter(GapDeviceDiscoveryFilter::Type::serviceUuid, infra::ConstByteRange()), "");
    }

    TEST_F(GapDeviceDiscoveryFilterTest, service_uuid_filter_asserts_on_1_byte_of_data)
    {
        const std::array<uint8_t, 1> data{};

        EXPECT_DEATH(GapDeviceDiscoveryFilter(GapDeviceDiscoveryFilter::Type::serviceUuid, infra::MakeConstByteRange(data)), "");
    }

    TEST_F(GapDeviceDiscoveryFilterTest, service_uuid_filter_asserts_on_3_bytes_of_data)
    {
        const std::array<uint8_t, 3> data{};

        EXPECT_DEATH(GapDeviceDiscoveryFilter(GapDeviceDiscoveryFilter::Type::serviceUuid, infra::MakeConstByteRange(data)), "");
    }

    TEST_F(GapDeviceDiscoveryFilterTest, service_uuid_filter_asserts_on_17_bytes_of_data)
    {
        const std::array<uint8_t, 17> data{};

        EXPECT_DEATH(GapDeviceDiscoveryFilter(GapDeviceDiscoveryFilter::Type::serviceUuid, infra::MakeConstByteRange(data)), "");
    }

    TEST_F(GapDeviceDiscoveryFilterTest, service_uuid_filter_asserts_on_data_longer_than_31_bytes)
    {
        const std::array<uint8_t, GapPeripheral::maxAdvertisementDataSize + 1> data{};

        EXPECT_DEATH(GapDeviceDiscoveryFilter(GapDeviceDiscoveryFilter::Type::serviceUuid, infra::MakeConstByteRange(data)), "");
    }
#endif

    TEST(GapInsertionOperatorEventTypeTest, event_type_overload_operator)
    {
        infra::StringOutputStream::WithStorage<128> stream;

        auto eventTypeAdvInd = services::AdvertisingReportType::advInd;
        auto eventTypeAdvDirectInd = services::AdvertisingReportType::advDirectInd;
        auto eventTypeAdvScanInd = services::AdvertisingReportType::advScanInd;
        auto eventTypeAdvNonconnInd = services::AdvertisingReportType::advNonconnInd;
        auto eventTypeScanResponse = services::AdvertisingReportType::scanResponse;

        stream << eventTypeAdvInd << " " << eventTypeAdvDirectInd << " " << eventTypeAdvScanInd << " " << eventTypeAdvNonconnInd << " " << eventTypeScanResponse;

        EXPECT_EQ("ADV_IND ADV_DIRECT_IND ADV_SCAN_IND ADV_NONCONN_IND SCAN_RESPONSE", stream.Storage());
    }

    TEST(GapInsertionOperatorEventAddressTypeTest, address_event_type_overload_operator)
    {
        infra::StringOutputStream::WithStorage<128> stream;

        auto eventAddressTypePublicDevice = services::GapDeviceAddressType::publicAddress;
        auto eventAddressTypeRandomDevice = services::GapDeviceAddressType::randomAddress;
        stream << eventAddressTypePublicDevice << " " << eventAddressTypeRandomDevice;

        EXPECT_EQ("Public Random", stream.Storage());
    }

    TEST(GapInsertionOperatorStateTest, state_overload_operator)
    {
        infra::StringOutputStream::WithStorage<128> stream;

        stream << services::GapState::standby << " " << services::GapState::scanning << " " << services::GapState::advertising << " " << services::GapState::connected << " " << services::GapState::initiating;

        EXPECT_EQ("Standby Scanning Advertising Connected Initiating", stream.Storage());
    }
}
