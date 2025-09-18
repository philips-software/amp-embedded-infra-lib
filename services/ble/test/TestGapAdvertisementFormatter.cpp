#include "infra/util/BoundedString.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "services/ble/Gap.hpp"
#include "gmock/gmock.h"

namespace services
{
    namespace
    {
        class GapAdvertisementFormatterTest
            : public testing::Test
        {
        public:
            infra::BoundedVector<uint8_t>::WithMaxSize<services::GapPeripheral::maxScanResponseDataSize> buffer;
            GapAdvertisementFormatter formatter{ buffer };
        };
    }

    TEST_F(GapAdvertisementFormatterTest, initial_state_is_empty)
    {
        EXPECT_THAT(formatter.FormattedAdvertisementData(), testing::IsEmpty());
        EXPECT_EQ(formatter.RemainingSpaceAvailable(), GapPeripheral::maxScanResponseDataSize);
    }

    TEST_F(GapAdvertisementFormatterTest, append_flags)
    {
        formatter.AppendFlags(GapPeripheral::AdvertisementFlags::leGeneralDiscoverableMode);

        auto data = formatter.FormattedAdvertisementData();
        EXPECT_EQ(data.size(), 3);
        EXPECT_EQ(data[0], 2);
        EXPECT_EQ(data[1], 0x01);
        EXPECT_EQ(data[2], static_cast<uint8_t>(GapPeripheral::AdvertisementFlags::leGeneralDiscoverableMode));

        EXPECT_EQ(formatter.RemainingSpaceAvailable(), GapPeripheral::maxScanResponseDataSize - 3);
    }

    TEST_F(GapAdvertisementFormatterTest, append_complete_local_name)
    {
        infra::BoundedConstString name{ "Test" };
        formatter.AppendCompleteLocalName(name);

        auto data = formatter.FormattedAdvertisementData();
        EXPECT_EQ(data.size(), 6);
        EXPECT_EQ(data[0], 5);
        EXPECT_EQ(data[1], 0x09);
        EXPECT_THAT(infra::MakeRange(data.begin() + 2, data.end()), infra::ContentsEqual(name));
    }

    TEST_F(GapAdvertisementFormatterTest, append_shortened_local_name)
    {
        infra::BoundedConstString name{ "Test" };
        formatter.AppendShortenedLocalName(name);

        auto data = formatter.FormattedAdvertisementData();
        EXPECT_EQ(data.size(), 6);
        EXPECT_EQ(data[0], 5);
        EXPECT_EQ(data[1], 0x08);
        EXPECT_THAT(infra::MakeRange(data.begin() + 2, data.end()), infra::ContentsEqual(name));
    }

    TEST_F(GapAdvertisementFormatterTest, append_manufacturer_data)
    {
        uint16_t manufacturerCode = 0x1234;
        std::array<uint8_t, 3> manufacturerData = { 0xAB, 0xCD, 0xEF };

        formatter.AppendManufacturerData(manufacturerCode, infra::MakeByteRange(manufacturerData));

        auto data = formatter.FormattedAdvertisementData();
        EXPECT_EQ(data.size(), 7);
        EXPECT_EQ(data[0], 6);
        EXPECT_EQ(data[1], 0xFF);
        EXPECT_EQ(data[2], 0x34);
        EXPECT_EQ(data[3], 0x12);
        EXPECT_EQ(data[4], 0xAB);
        EXPECT_EQ(data[5], 0xCD);
        EXPECT_EQ(data[6], 0xEF);
    }

    TEST_F(GapAdvertisementFormatterTest, append_list_of_16bit_services)
    {
        std::array<AttAttribute::Uuid16, 2> services = { 0x1234, 0x5678 };

        formatter.AppendListOfServicesUuid(infra::MakeRange(services));

        auto data = formatter.FormattedAdvertisementData();
        EXPECT_EQ(data.size(), 6);
        EXPECT_EQ(data[0], 5);
        EXPECT_EQ(data[1], 0x03);
        EXPECT_EQ(data[2], 0x34);
        EXPECT_EQ(data[3], 0x12);
        EXPECT_EQ(data[4], 0x78);
        EXPECT_EQ(data[5], 0x56);
    }

    TEST_F(GapAdvertisementFormatterTest, append_list_of_128bit_services)
    {
        std::array<uint8_t, 16> uuid128_1 = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
        AttAttribute::Uuid128 service1(uuid128_1);
        std::array<AttAttribute::Uuid128, 1> services = { service1 };

        formatter.AppendListOfServicesUuid(infra::MakeRange(services));

        auto data = formatter.FormattedAdvertisementData();
        EXPECT_EQ(data.size(), 18);
        EXPECT_EQ(data[0], 17);
        EXPECT_EQ(data[1], 0x07);

        std::array<uint8_t, 16> extractedFromBigEndian = service1;
        std::reverse(extractedFromBigEndian.begin(), extractedFromBigEndian.end());
        EXPECT_THAT(infra::MakeRange(data.begin() + 2, data.end()), infra::ContentsEqual(extractedFromBigEndian));
    }

    TEST_F(GapAdvertisementFormatterTest, append_public_target_address)
    {
        hal::MacAddress address({ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 });

        formatter.AppendPublicTargetAddress(address);

        auto data = formatter.FormattedAdvertisementData();
        EXPECT_EQ(data.size(), 8);
        EXPECT_EQ(data[0], 7);
        EXPECT_EQ(data[1], 0x17);
        EXPECT_EQ(data[2], 0x01);
        EXPECT_EQ(data[3], 0x02);
        EXPECT_EQ(data[4], 0x03);
        EXPECT_EQ(data[5], 0x04);
        EXPECT_EQ(data[6], 0x05);
        EXPECT_EQ(data[7], 0x06);
    }

    TEST_F(GapAdvertisementFormatterTest, multiple_append_operations)
    {
        formatter.AppendFlags(GapPeripheral::AdvertisementFlags::leGeneralDiscoverableMode);

        infra::BoundedConstString name{ "Test" };
        formatter.AppendCompleteLocalName(name);

        std::array<AttAttribute::Uuid16, 1> services = { 0x1234 };
        formatter.AppendListOfServicesUuid(infra::MakeRange(services));

        auto data = formatter.FormattedAdvertisementData();
        EXPECT_EQ(data.size(), 13);

        EXPECT_EQ(data[0], 2);
        EXPECT_EQ(data[1], 0x01);
        EXPECT_EQ(data[2], static_cast<uint8_t>(GapPeripheral::AdvertisementFlags::leGeneralDiscoverableMode));

        EXPECT_EQ(data[3], 5);
        EXPECT_EQ(data[4], 0x09);

        EXPECT_EQ(data[9], 3);
        EXPECT_EQ(data[10], 0x03);
    }

    TEST_F(GapAdvertisementFormatterTest, remaining_space_calculation)
    {
        std::size_t initialSpace = formatter.RemainingSpaceAvailable();
        EXPECT_EQ(initialSpace, GapPeripheral::maxScanResponseDataSize);

        formatter.AppendFlags(GapPeripheral::AdvertisementFlags::leGeneralDiscoverableMode);
        EXPECT_EQ(formatter.RemainingSpaceAvailable(), initialSpace - 3);

        infra::BoundedConstString name{ "Test" };
        formatter.AppendCompleteLocalName(name);
        EXPECT_EQ(formatter.RemainingSpaceAvailable(), initialSpace - 3 - 6);
    }

    TEST_F(GapAdvertisementFormatterTest, edge_case_single_byte_manufacturer_data)
    {
        uint16_t manufacturerCode = 0x0001;
        std::array<uint8_t, 1> manufacturerData = { 0xFF };

        formatter.AppendManufacturerData(manufacturerCode, infra::MakeByteRange(manufacturerData));

        auto data = formatter.FormattedAdvertisementData();
        EXPECT_EQ(data.size(), 5);
        EXPECT_EQ(data[0], 4);
        EXPECT_EQ(data[1], 0xFF);
        EXPECT_EQ(data[2], 0x01);
        EXPECT_EQ(data[3], 0x00);
        EXPECT_EQ(data[4], 0xFF);
    }

    TEST_F(GapAdvertisementFormatterTest, append_appearance)
    {
        uint16_t keyboardAppearance = 0x03C1;

        formatter.AppendAppearance(keyboardAppearance);

        auto data = formatter.FormattedAdvertisementData();
        EXPECT_EQ(data.size(), 4);
        EXPECT_EQ(data[0], 3);
        EXPECT_EQ(data[1], 0x19);
        EXPECT_EQ(data[2], 0xC1);
        EXPECT_EQ(data[3], 0x03);

        EXPECT_EQ(formatter.RemainingSpaceAvailable(), GapPeripheral::maxScanResponseDataSize - 4);
    }
}
