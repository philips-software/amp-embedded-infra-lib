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

class FlashEchoTest
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

TEST_F(FlashEchoTest, Read)
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

TEST_F(FlashEchoTest, Write)
{
    EXPECT_CALL(delegate, WriteBuffer(infra::CheckByteRangeContents(infra::MakeRange(data)), 1234, testing::_)).WillOnce(testing::Invoke([&](infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onWriteDone)
        {
            onDone = onWriteDone;
        }));
    flash.Write(1234, data);

    EXPECT_CALL(flashResult, WriteDone());
    onDone();
}

TEST_F(FlashEchoTest, EraseSectors)
{
    EXPECT_CALL(delegate, EraseSectors(1234, 1238, testing::_)).WillOnce(testing::SaveArg<2>(&onDone));
    flash.EraseSectors(1234, 4);

    EXPECT_CALL(flashResult, EraseSectorsDone());
    onDone();
}

TEST_F(FlashEchoTest, stop_while_idle)
{
    infra::VerifyingFunction<void()> onStopped;
    flash.Stop(onStopped);
}

TEST_F(FlashEchoTest, stop_while_reading)
{
    infra::VerifyingFunction<void()> onStopped;

    EXPECT_CALL(delegate, ReadBuffer(testing::_, 1234, testing::_)).WillOnce(testing::Invoke([&](infra::ByteRange buffer, uint32_t address, infra::Function<void()> onReadDone)
        {
            infra::Copy(infra::MakeRange(data), buffer);
            onDone = onReadDone;
            flash.Stop(onStopped);
        }));
    flash.Read(1234, data.size());

    onDone();
}

TEST_F(FlashEchoTest, stop_while_writing)
{
    infra::VerifyingFunction<void()> onStopped;

    EXPECT_CALL(delegate, WriteBuffer(infra::CheckByteRangeContents(infra::MakeRange(data)), 1234, testing::_)).WillOnce(testing::Invoke([&](infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onWriteDone)
        {
            onDone = onWriteDone;
            flash.Stop(onStopped);
        }));
    flash.Write(1234, data);

    onDone();
}

TEST_F(FlashEchoTest, stop_while_erasing)
{
    infra::VerifyingFunction<void()> onStopped;

    EXPECT_CALL(delegate, EraseSectors(1234, 1238, testing::_)).WillOnce(testing::Invoke([&](uint32_t sector, uint32_t numberOfSectors, infra::Function<void()> onEraseDone)
        {
            onDone = onEraseDone;
            flash.Stop(onStopped);
        }));
    flash.EraseSectors(1234, 4);

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

TEST_F(FlashProxyTest, ReadBuffer_TwoChunks_BothRequestedBeforeDone)
{
    constexpr auto twoChunkSize = flash::WriteRequest::contentsSize + 4;
    std::array<uint8_t, twoChunkSize> buffer{};

    infra::Function<void()> sendReadDone1;
    infra::Function<void()> sendReadDone2;

    EXPECT_CALL(flash, Read(1234, flash::WriteRequest::contentsSize))
        .WillOnce([&](uint32_t address, uint32_t size)
            {
                flash.MethodDone();
                sendReadDone1 = [this, size]()
                {
                    flashResult.RequestSend([this, size]()
                        {
                            std::array<uint8_t, flash::WriteRequest::contentsSize> chunk{};
                            flashResult.ReadDone(infra::Head(infra::MakeRange(chunk), size));
                        });
                };
            });
    EXPECT_CALL(flash, Read(1234 + flash::WriteRequest::contentsSize, 4))
        .WillOnce([&](uint32_t address, uint32_t size)
            {
                flash.MethodDone();
                sendReadDone2 = [this, size]()
                {
                    flashResult.RequestSend([this, size]()
                        {
                            std::array<uint8_t, 4> chunk{};
                            flashResult.ReadDone(infra::Head(infra::MakeRange(chunk), size));
                        });
                };
            });

    infra::VerifyingFunction<void()> onDone;
    flashProxy.ReadBuffer(infra::MakeRange(buffer), 1234, onDone);

    // Both reads were requested before any ReadDone (pipelined)
    EXPECT_TRUE(sendReadDone1);
    EXPECT_TRUE(sendReadDone2);

    sendReadDone1();
    sendReadDone2();
}

TEST_F(FlashProxyTest, WriteBuffer_TwoChunks_BothRequestedBeforeDone)
{
    constexpr auto twoChunkSize = flash::WriteRequest::contentsSize + 4;
    std::array<uint8_t, twoChunkSize> writeData{};

    infra::Function<void()> sendWriteDone1;
    infra::Function<void()> sendWriteDone2;

    EXPECT_CALL(flash, Write(1234, testing::_))
        .WillOnce([&](uint32_t address, infra::ConstByteRange contents)
            {
                flash.MethodDone();
                sendWriteDone1 = [this]()
                {
                    flashResult.RequestSend([this]()
                        {
                            flashResult.WriteDone();
                        });
                };
            });
    EXPECT_CALL(flash, Write(1234 + flash::WriteRequest::contentsSize, testing::_))
        .WillOnce([&](uint32_t address, infra::ConstByteRange contents)
            {
                flash.MethodDone();
                sendWriteDone2 = [this]()
                {
                    flashResult.RequestSend([this]()
                        {
                            flashResult.WriteDone();
                        });
                };
            });

    infra::VerifyingFunction<void()> onDone;
    flashProxy.WriteBuffer(infra::MakeRange(writeData), 1234, onDone);

    // Both writes were requested before any WriteDone (pipelined)
    EXPECT_TRUE(sendWriteDone1);
    EXPECT_TRUE(sendWriteDone2);

    sendWriteDone1();
    sendWriteDone2();
}

class FlashSequentialProxyTest
    : public testing::Test
{
public:
    services::MethodSerializerFactory::OnHeap serializerFactory;
    application::EchoSingleLoopback echo{ serializerFactory };
    const std::array<uint32_t, 4> sectorSizesArray{ 4096, 4096, 4096, 4096 };
    services::FlashEchoSequentialProxy flashProxy{ echo, infra::MakeRange(sectorSizesArray) };
    testing::StrictMock<FlashMock> flash{ echo };
    flash::FlashResultProxy flashResult{ echo };

    const std::array<uint8_t, 4> data{ 5, 8, 2, 3 };
};

TEST_F(FlashSequentialProxyTest, Accessors)
{
    EXPECT_EQ(4, flashProxy.NumberOfSectors());
    EXPECT_EQ(4096, flashProxy.SizeOfSector(0));
    EXPECT_EQ(1, flashProxy.SectorOfAddress(5000));
    EXPECT_EQ(4096, flashProxy.AddressOfSector(1));
}

TEST_F(FlashSequentialProxyTest, ReadBuffer)
{
    EXPECT_CALL(flash, Read(1234, 4)).WillOnce(testing::Invoke([this](uint32_t address, uint32_t size)
        {
            flashResult.RequestSend([this]()
                {
                    flashResult.ReadDone(infra::MakeRange(data));
                    flash.MethodDone();
                });
        }));

    std::array<uint8_t, 4> buffer{};
    infra::VerifyingFunction<void()> onDone;
    flashProxy.ReadBuffer(infra::MakeRange(buffer), 1234, onDone);

    EXPECT_TRUE(infra::ContentsEqual(infra::MakeRange(buffer), infra::MakeRange(data)));
}

TEST_F(FlashSequentialProxyTest, WriteBuffer)
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

TEST_F(FlashSequentialProxyTest, EraseSectors)
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

TEST_F(FlashSequentialProxyTest, ReadBuffer_TwoChunks_SecondOnlyAfterFirstDone)
{
    constexpr auto twoChunkSize = flash::WriteRequest::contentsSize + 4;
    std::array<uint8_t, twoChunkSize> buffer{};

    infra::Function<void()> sendReadDone1;
    infra::Function<void()> sendReadDone2;

    EXPECT_CALL(flash, Read(1234, flash::WriteRequest::contentsSize))
        .WillOnce([&](uint32_t address, uint32_t size)
            {
                flash.MethodDone();
                sendReadDone1 = [this, size]()
                {
                    flashResult.RequestSend([this, size]()
                        {
                            std::array<uint8_t, flash::WriteRequest::contentsSize> chunk{};
                            flashResult.ReadDone(infra::Head(infra::MakeRange(chunk), size));
                        });
                };
            });

    infra::VerifyingFunction<void()> onDone;
    flashProxy.ReadBuffer(infra::MakeRange(buffer), 1234, onDone);

    // Only the first read was requested; the second is waiting for the first ReadDone
    EXPECT_TRUE(sendReadDone1);
    EXPECT_FALSE(sendReadDone2);

    EXPECT_CALL(flash, Read(1234 + flash::WriteRequest::contentsSize, 4))
        .WillOnce([&](uint32_t address, uint32_t size)
            {
                flash.MethodDone();
                sendReadDone2 = [this, size]()
                {
                    flashResult.RequestSend([this, size]()
                        {
                            std::array<uint8_t, 4> chunk{};
                            flashResult.ReadDone(infra::Head(infra::MakeRange(chunk), size));
                        });
                };
            });

    sendReadDone1();

    // Now the second read was requested after the first completed
    ASSERT_TRUE(sendReadDone2);

    sendReadDone2();
}

TEST_F(FlashSequentialProxyTest, WriteBuffer_TwoChunks_SecondOnlyAfterFirstDone)
{
    constexpr auto twoChunkSize = flash::WriteRequest::contentsSize + 4;
    std::array<uint8_t, twoChunkSize> writeData{};

    infra::Function<void()> sendWriteDone1;
    infra::Function<void()> sendWriteDone2;

    EXPECT_CALL(flash, Write(1234, testing::_))
        .WillOnce([&](uint32_t address, infra::ConstByteRange contents)
            {
                flash.MethodDone();
                sendWriteDone1 = [this]()
                {
                    flashResult.RequestSend([this]()
                        {
                            flashResult.WriteDone();
                        });
                };
            });

    infra::VerifyingFunction<void()> onDone;
    flashProxy.WriteBuffer(infra::MakeRange(writeData), 1234, onDone);

    // Only the first write was requested; the second is waiting for the first WriteDone
    EXPECT_TRUE(sendWriteDone1);
    EXPECT_FALSE(sendWriteDone2);

    EXPECT_CALL(flash, Write(1234 + flash::WriteRequest::contentsSize, testing::_))
        .WillOnce([&](uint32_t address, infra::ConstByteRange contents)
            {
                flash.MethodDone();
                sendWriteDone2 = [this]()
                {
                    flashResult.RequestSend([this]()
                        {
                            flashResult.WriteDone();
                        });
                };
            });

    sendWriteDone1();

    // Now the second write was requested after the first completed
    EXPECT_TRUE(sendWriteDone2);

    sendWriteDone2();
}

class FlashEchoDeathTest
    : public testing::Test
{
public:
    services::MethodSerializerFactory::OnHeap serializerFactory;
    application::EchoSingleLoopback echo{ serializerFactory };
    testing::NiceMock<hal::CleanFlashMock> delegate;
    services::FlashEcho flash{ echo, delegate };

    const std::array<uint8_t, 4> data{ 5, 8, 2, 3 };
    infra::Function<void()> onDone;
};

TEST_F(FlashEchoDeathTest, write_while_read_in_progress_aborts)
{
    EXPECT_CALL(delegate, ReadBuffer(testing::_, 1234, testing::_)).WillOnce(testing::SaveArg<2>(&onDone));
    flash.Read(1234, data.size());

    EXPECT_DEATH(flash.Write(1234, data), "");
}

class FlashProxyDeathTest
    : public testing::Test
{
public:
    services::MethodSerializerFactory::OnHeap serializerFactory;
    application::EchoSingleLoopback echo{ serializerFactory };
    services::FlashEchoHomogeneousProxy flashProxy{ echo, 4, 4096 };
    testing::NiceMock<FlashMock> flash{ echo };
    flash::FlashResultProxy flashResult{ echo };

    const std::array<uint8_t, 4> data{ 5, 8, 2, 3 };
};

TEST_F(FlashProxyDeathTest, write_buffer_while_read_buffer_in_progress_aborts)
{
    EXPECT_CALL(flash, Read(1234, 4));

    std::array<uint8_t, 4> buffer{};
    flashProxy.ReadBuffer(infra::MakeRange(buffer), 1234, []() {});

    EXPECT_DEATH(flashProxy.WriteBuffer(infra::MakeRange(data), 1234, []() {}), "");
}

TEST_F(FlashProxyDeathTest, read_done_while_idle_aborts)
{
    EXPECT_DEATH(flashResult.RequestSend([this]()
                     {
                         flashResult.ReadDone(infra::MakeRange(data));
                     }),
        "");
}

TEST_F(FlashProxyDeathTest, write_done_while_idle_aborts)
{
    EXPECT_DEATH(flashResult.RequestSend([this]()
                     {
                         flashResult.WriteDone();
                     }),
        "");
}

TEST_F(FlashProxyDeathTest, erase_sectors_done_while_idle_aborts)
{
    EXPECT_DEATH(flashResult.RequestSend([this]()
                     {
                         flashResult.EraseSectorsDone();
                     }),
        "");
}
