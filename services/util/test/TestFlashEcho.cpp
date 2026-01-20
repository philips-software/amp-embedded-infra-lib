#include "hal/interfaces/test_doubles/FlashMock.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "protobuf/echo/test_doubles/EchoMock.hpp"
#include "protobuf/echo/test_doubles/EchoSingleLoopback.hpp"
#include "services/util/FlashEcho.hpp"
#include "gtest/gtest.h"

namespace
{
    class FlashMock
        : public flash::Flash
    {
    public:
        using flash::Flash::Flash;

        MOCK_METHOD(void, Read, (uint32_t address, uint32_t size), (override));
        MOCK_METHOD(void, Write, (uint32_t address, infra::ConstByteRange contents), (override));
        MOCK_METHOD(void, EraseSectors, (uint32_t sector, uint32_t number_of_sectors), (override));
    };

    class FlashResultMock
        : public flash::FlashResult
    {
    public:
        using flash::FlashResult::FlashResult;

        MOCK_METHOD(void, ReadDone, (infra::ConstByteRange contents), (override));
        MOCK_METHOD(void, WriteDone, (), (override));
        MOCK_METHOD(void, EraseSectorsDone, (), (override));
    };
}

class FlashTest
    : public testing::Test
{
public:
    services::MethodSerializerFactory::OnHeap serializerFactory;
    application::EchoSingleLoopback echo{ serializerFactory };
    testing::StrictMock<hal::CleanFlashMock> delegate;
    services::FlashEcho flash{ echo, delegate };
    testing::StrictMock<FlashResultMock> flashResult{ echo };

    const std::array<uint8_t, 4> data{ 5, 8, 2, 3 };
    infra::Function<void()> onDone;
};

TEST_F(FlashTest, Read)
{
    EXPECT_CALL(delegate, ReadBuffer(testing::_, 1234, testing::_)).WillOnce(testing::Invoke([&](infra::ByteRange buffer, uint32_t address, infra::Function<void()> onReadDone)
        {
            infra::Copy(infra::MakeRange(data), buffer);
            onDone = onReadDone;
        }));
    flash.Read(1234, data.size());

    EXPECT_CALL(flashResult, ReadDone(infra::CheckByteRangeContents(infra::MakeRange(data))));
    onDone();
}

TEST_F(FlashTest, Write)
{
    EXPECT_CALL(delegate, WriteBuffer(infra::CheckByteRangeContents(infra::MakeRange(data)), 1234, testing::_)).WillOnce(testing::Invoke([&](infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onReadDone)
        {
            onDone = onReadDone;
        }));
    flash.Write(1234, data);

    EXPECT_CALL(flashResult, WriteDone());
    onDone();
}

TEST_F(FlashTest, EraseSectors)
{
    EXPECT_CALL(delegate, EraseSectors(1234, 1238, testing::_)).WillOnce(testing::SaveArg<2>(&onDone));
    flash.EraseSectors(1234, 4);

    EXPECT_CALL(flashResult, EraseSectorsDone());
    onDone();
}

class FlashProxyTest
    : public testing::Test
{
public:
    services::MethodSerializerFactory::OnHeap serializerFactory;
    application::EchoSingleLoopback echo{ serializerFactory };
    services::FlashEchoHomogeneousProxy flashProxy{ echo, 4, 4096 };
    testing::StrictMock<FlashMock> flash{ echo };
    flash::FlashResultProxy flashResult{ echo };

    const std::array<uint8_t, 4> data{ 5, 8, 2, 3 };
    infra::Function<void()> onDone;
};

TEST_F(FlashProxyTest, Accessors)
{
    EXPECT_EQ(4, flashProxy.NumberOfSectors());
    EXPECT_EQ(4096, flashProxy.SizeOfSector(0));
    EXPECT_EQ(1, flashProxy.SectorOfAddress(5000));
    EXPECT_EQ(4096, flashProxy.AddressOfSector(1));
}

TEST_F(FlashProxyTest, ReadBuffer)
{
    EXPECT_CALL(flash, Read(1234, 4)).WillOnce(testing::Invoke([this](uint32_t address, uint32_t size)
        {
            flashResult.RequestSend([this]()
                {
                    flashResult.ReadDone(infra::MakeRange(data));
                    flash.MethodDone();
                });
        }));

    std::array<uint8_t, 4> buffer{ 5, 8, 2, 3 };
    flashProxy.ReadBuffer(infra::MakeRange(buffer), 1234, [&]()
        {
            EXPECT_TRUE(infra::ContentsEqual(infra::MakeRange(buffer), infra::MakeRange(data)));
        });
}

TEST_F(FlashProxyTest, WriteBuffer)
{
    EXPECT_CALL(flash, Write(1234, infra::CheckByteRangeContents(infra::MakeRange(data)))).WillOnce(testing::Invoke([this](uint32_t address, infra::ConstByteRange contents)
        {
            flashResult.RequestSend([this]()
                {
                    flashResult.WriteDone();
                    flash.MethodDone();
                });
        }));

    infra::VerifyingFunction<void()> onDone;
    flashProxy.WriteBuffer(infra::MakeRange(data), 1234, onDone);
}

TEST_F(FlashProxyTest, EraseSectors)
{
    EXPECT_CALL(flash, EraseSectors(12, 22)).WillOnce(testing::Invoke([this](uint32_t sector, uint32_t numberOfSectors)
        {
            flashResult.RequestSend([this]()
                {
                    flashResult.EraseSectorsDone();
                    flash.MethodDone();
                });
        }));

    infra::VerifyingFunction<void()> onDone;
    flashProxy.EraseSectors(12, 34, onDone);
}
