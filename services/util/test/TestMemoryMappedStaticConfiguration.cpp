#include "gtest/gtest.h"
#include "hal/interfaces/test_doubles/FlashStub.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/InterfaceConnector.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/MemoryMappedStaticConfiguration.hpp"

namespace
{
    struct Data
        : public infra::InterfaceConnector<Data>
    {
        MOCK_CONST_METHOD1(Serialize, void(infra::ProtoFormatter& formatter));
        MOCK_METHOD1(Deserialize, void(infra::ProtoParser& parser));

    public:
        static const uint32_t maxMessageSize = 16;
    };

    struct DataProxy
    {
        DataProxy() = default;
        DataProxy(infra::ProtoParser& parser)
        {
            Deserialize(parser);
        }

        void Serialize(infra::ProtoFormatter& formatter) const
        {
            Data::Instance().Serialize(formatter);
        }

        void Deserialize(infra::ProtoParser& parser)
        {
            Data::Instance().Deserialize(parser);
        }

    public:
        static const uint32_t maxMessageSize = Data::maxMessageSize;
    };
}

class MemoryMappedStaticConfigurationTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    void ConstructConfiguration()
    {
        configuration.Emplace(flash, infra::MakeRange(flash.sectors[0]));
    }

    hal::FlashStub flash{ 1, 16 };
    infra::Optional<services::MemoryMappedStaticConfiguration<DataProxy, DataProxy>> configuration;

    testing::StrictMock<Data> data;
};

TEST_F(MemoryMappedStaticConfigurationTest, write_data_in_empty_flash)
{
    ConstructConfiguration();
    EXPECT_FALSE(configuration->Valid());

    DataProxy newValue;
    EXPECT_CALL(data, Serialize(testing::_)).WillOnce(testing::Invoke([](infra::ProtoFormatter& formatter)
    {
        formatter.PutVarInt(15);
    }));

    infra::VerifyingFunctionMock<void()> callback;
    configuration->Write(newValue, [&callback]() { callback.callback(); });
    EXPECT_CALL(data, Deserialize(testing::_)).WillOnce(testing::Invoke([](infra::ProtoParser& parser)
    {
        EXPECT_EQ(15, parser.GetVarInt());
    }));
    ExecuteAllActions();

    EXPECT_EQ((std::vector<uint8_t>{ 0xee, 0x1d, 0xef, 0x7e, 0x34, 0x12, 0x66, 0x4d, 0x01, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff }), flash.sectors[0]);
}

TEST_F(MemoryMappedStaticConfigurationTest, recover_data_from_flash)
{
    flash.sectors[0] = std::vector<uint8_t>{ 0xee, 0x1d, 0xef, 0x7e, 0x34, 0x12, 0x66, 0x4d, 0x01, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff };
    EXPECT_CALL(data, Deserialize(testing::_)).WillOnce(testing::Invoke([](infra::ProtoParser& parser)
    {
        EXPECT_EQ(15, parser.GetVarInt());
    }));
    ConstructConfiguration();
    EXPECT_TRUE(configuration->Valid());
}
