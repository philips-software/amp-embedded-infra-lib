#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/ConstructBin.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/SesameWindowed.hpp"
#include "services/util/test_doubles/SesameMock.hpp"
#include "gmock/gmock.h"
#include <deque>

template<uint8_t SplitBuffers>
class SesameWindowedTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    SesameWindowedTest()
    {
        SetEncodingSizeExpectations();
    }

    void SetEncodingSizeExpectations()
    {
        EXPECT_CALL(base, WorstCaseEncodedMessageSize(testing::_)).WillRepeatedly(testing::Invoke([](std::size_t size)
            {
                return size + size / 254 + 2;
            }));
        EXPECT_CALL(base, WorstCaseDecodedMessageSize(testing::_)).WillRepeatedly(testing::Invoke([](std::size_t size)
            {
                return (size - 2) - (size - 2) / 254;
            }));
    }

    void SendMessageStreamAvailableWithWriter(const std::vector<uint8_t>& expected)
    {
        assert(sentData.empty());
        assert(expectedMessage.empty());
        expectedMessage = expected;
        writer.OnAllocatable([this]()
            {
                auto expectedMessageCopy = expectedMessage;
                expectedMessage.clear();
                auto sentDataCopy = sentData;
                sentData.clear();
                base.GetObserver().MessageSent(sentDataCopy.size() + sentDataCopy.size() / 254 + 2);
                EXPECT_EQ(expectedMessageCopy, sentDataCopy);
            });
        base.GetObserver().SendMessageStreamAvailable(writer.Emplace(sentData));
    }

    void ReceivePacket(const std::vector<uint8_t>& data)
    {
        infra::StdVectorInputStreamReader::WithStorage reader(std::in_place, data);
        base.GetObserver().ReceivedMessage(reader, data.size() + data.size() / 254 + 2);
    }

    void ReceiveInitRequest(uint16_t availableWindow)
    {
        EXPECT_CALL(observer, Initialized());
        EXPECT_CALL(initializer, InitInformationReceived(testing::_)).WillOnce([this](infra::StreamReaderWithRewinding& initInfoReader)
            {
                infra::DataInputStream::WithErrorPolicy stream(initInfoReader);
                std::vector<uint8_t> data(stream.Available());
                stream >> infra::MakeRange(data);
                EXPECT_THAT(data, testing::ElementsAreArray(initInfo));
            });
        ReceivePacket(infra::ConstructBin().Value<uint8_t>(1).Value<infra::LittleEndian<uint16_t>>(availableWindow)(initInfo).Vector());
    }

    void ReceiveInitResponseWithoutInitialized(uint16_t availableWindow)
    {
        EXPECT_CALL(initializer, InitInformationReceived(testing::_)).WillOnce([this](infra::StreamReaderWithRewinding& initInfoReader)
            {
                infra::DataInputStream::WithErrorPolicy stream(initInfoReader);
                std::vector<uint8_t> data(stream.Available());
                stream >> infra::MakeRange(data);
                EXPECT_THAT(data, testing::ElementsAreArray(initInfo));
            });
        ReceivePacket(infra::ConstructBin().Value<uint8_t>(2).Value<infra::LittleEndian<uint16_t>>(availableWindow)(initInfo).Vector());
    }

    void ReceiveInitResponse(uint16_t availableWindow)
    {
        EXPECT_CALL(observer, Initialized());
        ReceiveInitResponseWithoutInitialized(availableWindow);
    }

    void ReceiveReleaseWindow(uint16_t availableWindow)
    {
        ReceivePacket(infra::ConstructBin().Value<uint8_t>(3).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector());
    }

    void ReceiveMessageOnChannel(services::SesameChannel channel, const std::string& text)
    {
        EXPECT_CALL(base, MessageSize(testing::_)).WillOnce([](infra::StreamReader&& reader)
            {
                return reader.Available() + reader.Available() / 254 + 2;
            });
        ReceivePacket(infra::ConstructBin()(channel == services::SesameChannel::red ? 4 : 5)(text).Vector());
    }

    void ReceiveMessage(const std::string& text)
    {
        ReceiveMessageOnChannel(services::SesameChannel::red, text);
    }

    void PretendReceiveMessage(const std::string& text)
    {
        ReceivePacket(infra::ConstructBin()(4)(text).Vector());
    }

    void ExpectRequestSendMessageForInit(uint16_t availableWindow)
    {
        EXPECT_CALL(base, RequestSendMessage(3 + initInfo.size())).WillOnce([this, availableWindow](uint16_t size)
            {
                SendMessageStreamAvailableWithWriter(infra::ConstructBin().Value<uint8_t>(1).Value<infra::LittleEndian<uint16_t>>(availableWindow)(initInfo).Vector());
            });
    }

    void ExpectRequestSendMessageForInitResponse(uint16_t availableWindow)
    {
        EXPECT_CALL(base, RequestSendMessage(3 + initInfo.size())).WillOnce([this, availableWindow](uint16_t size)
            {
                SendMessageStreamAvailableWithWriter(infra::ConstructBin().Value<uint8_t>(2).Value<infra::LittleEndian<uint16_t>>(availableWindow)(initInfo).Vector());
            });
    }

    void ExpectRequestSendMessageForReleaseWindow(uint16_t releasedSize)
    {
        EXPECT_CALL(base, RequestSendMessage(3)).WillOnce([this, releasedSize](uint16_t size)
            {
                SendMessageStreamAvailableWithWriter(infra::ConstructBin().Value<uint8_t>(3).Value<infra::LittleEndian<uint16_t>>(releasedSize).Vector());
            });
    }

    void ExpectRequestSendMessageForMessage(uint16_t size, const std::vector<uint8_t>& expected, services::SesameChannel channel = services::SesameChannel::red)
    {
        EXPECT_CALL(base, RequestSendMessage(size)).WillOnce([this, expected, channel](uint16_t size)
            {
                SendMessageStreamAvailableWithWriter(infra::ConstructBin().Value<uint8_t>(channel == services::SesameChannel::red ? 4 : 5)(expected).Vector());
            });
    }

    void ExpectSendMessageStreamAvailable(const std::vector<uint8_t>& data)
    {
        EXPECT_CALL(observer, SendMessageStreamAvailable(testing::_, services::SesameChannel::red)).WillOnce([data](infra::SharedPtr<infra::StreamWriter>&& writer, [[maybe_unused]] services::SesameChannel channel)
            {
                        infra::DataOutputStream::WithErrorPolicy stream(*writer);
                stream << infra::MakeRange(data);
            });
    }

    void ExpectSendMessageStreamAvailableAndSaveWriter()
    {
        EXPECT_CALL(observer, SendMessageStreamAvailable(testing::_, services::SesameChannel::red)).WillOnce(testing::SaveArg<0>(&savedWriter));
    }

    void ExpectReceivedMessage(const std::string& expected)
    {
        EXPECT_CALL(observer, ReceivedMessage(testing::_, services::SesameChannel::red)).WillOnce([expected](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, [[maybe_unused]] services::SesameChannel channel)
            {
                        infra::DataInputStream::WithErrorPolicy stream(*reader);
                std::string text(stream.Available(), 0);
                stream >> infra::ByteRange(reinterpret_cast<uint8_t*>(text.data()), reinterpret_cast<uint8_t*>(text.data() + text.size()));

                EXPECT_EQ(expected, text);
            });
    }

    void ExpectReceivedMessageAndSaveReader(const std::string& expected)
    {
        EXPECT_CALL(observer, ReceivedMessage(testing::_, services::SesameChannel::red)).WillOnce(testing::SaveArg<0>(&savedReader));
    }

    testing::StrictMock<services::SesameEncodedMock> base;
    std::vector<uint8_t> expectedMessage;
    std::vector<uint8_t> sentData;
    infra::NotifyingSharedOptional<infra::StdVectorOutputStreamWriter> writer;
    std::array<uint8_t, 2> initInfo{ 6, 7 };
    testing::StrictMock<services::SesameInitializerMock> initializer;
    infra::Execute execute{ [this]()
        {
            EXPECT_CALL(base, MaxSendMessageSize()).WillRepeatedly(testing::Return(8 + 8 * SplitBuffers));
            EXPECT_CALL(base, WorstCaseEncodedMessageSize(3)).WillOnce(testing::Return(5));
            EXPECT_CALL(initializer, InitInformation()).WillRepeatedly(testing::Return(infra::MakeRange(initInfo)));
            ExpectRequestSendMessageForInit(8 + 8 * SplitBuffers);
        } };
    services::SesameWindowed::WithMaxMessageSize<6, SplitBuffers> communicationInstance{ base, initializer };
    services::SesameWindowed* communication = &communicationInstance;
    testing::StrictMock<services::SesameObserverMock> observer{ *communication };
    infra::SharedPtr<infra::StreamWriter> savedWriter;
    infra::SharedPtr<infra::StreamReaderWithRewinding> savedReader;
};

using SesameWindowedTestDouble = SesameWindowedTest<2>;
using SesameWindowedTestTriple = SesameWindowedTest<3>;

TEST_F(SesameWindowedTestDouble, MaxSendMessageSize)
{
    ReceiveInitResponse(24);
    // 6 bytes in a message expands to 1 (cobs) + 1 (operation) + 6 (message) + 1 (delimiter) = 9
    EXPECT_EQ(6, communication->MaxSendMessageSize());
    EXPECT_EQ(9, (services::SesameWindowed::bufferSizeForMessage<6, services::SesameCobs::EncodedMessageSize>));
}

TEST_F(SesameWindowedTestDouble, send_message_after_initialized)
{
    ReceiveInitResponse(24);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    communication->RequestSendMessage(4);
}

TEST_F(SesameWindowedTestDouble, send_blue_message_after_initialized)
{
    ReceiveInitResponse(24);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 }, services::SesameChannel::blue);
    EXPECT_CALL(observer, SendMessageStreamAvailable(testing::_, services::SesameChannel::blue)).WillOnce([](infra::SharedPtr<infra::StreamWriter>&& writer, [[maybe_unused]] services::SesameChannel channel)
        {
                infra::DataOutputStream::WithErrorPolicy stream(*writer);
            const std::vector<uint8_t>& data = { 1, 2, 3, 4 };
            stream << infra::MakeRange(data);
        });
    communication->RequestSendMessage(4, services::SesameChannel::blue);
}

TEST_F(SesameWindowedTestDouble, message_waits_until_window_is_freed)
{
    ReceiveInitResponse(6);

    communication->RequestSendMessage(4);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    ExpectRequestSendMessageForReleaseWindow(12);
    ReceiveReleaseWindow(6);
}

TEST_F(SesameWindowedTestDouble, long_message_waits_until_window_is_freed_taking_into_account_cobs_overhead)
{
    EXPECT_CALL(base, MaxSendMessageSize()).WillRepeatedly(testing::Return(2000));
    ReceiveInitResponse(519);

    auto fillWindow = std::vector<uint8_t>(251, 1);
    ExpectRequestSendMessageForMessage(fillWindow.size() + 1, fillWindow);
    ExpectSendMessageStreamAvailable(fillWindow);
    communication->RequestSendMessage(fillWindow.size());

    auto fillWindow2 = std::vector<uint8_t>(1, 1);
    ExpectRequestSendMessageForMessage(fillWindow2.size() + 1, fillWindow2);
    ExpectSendMessageStreamAvailable(fillWindow2);
    communication->RequestSendMessage(fillWindow2.size());

    // A message of size 253 plus one byte for the operation may consist of 254 zeros, and therefore need one extra COBS overhead byte.
    // Total window available needs therefore to be 254 + 1 (one extra COBS byte) + 2 (normal COBS byte and closing 0) + 5 (safeguard for another window release)
    communication->RequestSendMessage(253);

    ExpectRequestSendMessageForMessage(254, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    ExpectRequestSendMessageForReleaseWindow(12);
    ReceiveReleaseWindow(1);
}

TEST_F(SesameWindowedTestDouble, exact_used_window_size_is_consumed_by_message)
{
    auto longMessage = infra::ConstructBin().Repeat(253 * 3, 0).Vector();

    EXPECT_CALL(base, MaxSendMessageSize()).WillRepeatedly(testing::Return(4000));
    // Exactly enough for any message of size longMessage plus safeguard for a window release
    ReceiveInitResponse(254 * 6 + 1 + 7 + 2 + 5);

    // Sending longMessage, for which no extra COBS overhead bytes were necessary, results in 7 + 5 window still available
    ExpectRequestSendMessageForMessage(longMessage.size() + 1, longMessage);
    ExpectSendMessageStreamAvailable(longMessage);
    communication->RequestSendMessage(longMessage.size());

    // Now prove that enough window is still available by sending another message
    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    communication->RequestSendMessage(4);
}

TEST_F(SesameWindowedTestDouble, request_sending_new_message_while_previous_is_still_processing)
{
    ReceiveInitResponse(28);

    EXPECT_CALL(base, RequestSendMessage(5));
    communication->RequestSendMessage(4);

    EXPECT_CALL(observer, SendMessageStreamAvailable(testing::_, services::SesameChannel::red)).WillOnce([this](infra::SharedPtr<infra::StreamWriter>&& writer, [[maybe_unused]] services::SesameChannel channel)
        {
                infra::DataOutputStream::WithErrorPolicy stream(*writer);
            const std::vector<uint8_t>& data = { 1, 2, 3, 4 };
            stream << infra::MakeRange(data);

            writer = nullptr;
            communication->RequestSendMessage(2);
        });

    EXPECT_CALL(base, RequestSendMessage(3));
    SendMessageStreamAvailableWithWriter(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());

    ExpectSendMessageStreamAvailable({ 5, 6 });
    SendMessageStreamAvailableWithWriter(infra::ConstructBin().Value<uint8_t>(4)({ 5, 6 }).Vector());
}

TEST_F(SesameWindowedTestDouble, receive_message_after_initialized)
{
    ReceiveInitResponse(26);

    ExpectReceivedMessage("abcd");
    ExpectRequestSendMessageForReleaseWindow(14);
    ReceiveMessage("abcd");
}

TEST_F(SesameWindowedTestDouble, receive_blue_message_after_initialized)
{
    ReceiveInitResponse(26);

    EXPECT_CALL(observer, ReceivedMessage(testing::_, services::SesameChannel::blue)).WillOnce([](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, [[maybe_unused]] services::SesameChannel channel)
        {
                infra::DataInputStream::WithErrorPolicy stream(*reader);
            std::string text(stream.Available(), 0);
            stream >> infra::ByteRange(reinterpret_cast<uint8_t*>(text.data()), reinterpret_cast<uint8_t*>(text.data() + text.size()));
            EXPECT_THAT(text, testing::Eq("abcd"));
        });
    ExpectRequestSendMessageForReleaseWindow(14);
    ReceiveMessageOnChannel(services::SesameChannel::blue, "abcd");
}

TEST_F(SesameWindowedTestDouble, release_window_packet_waits_for_window_available)
{
    ReceiveInitResponse(0);

    ExpectReceivedMessage("abcd");
    ReceiveMessage("abcd");

    ExpectRequestSendMessageForReleaseWindow(19);
    ReceiveReleaseWindow(5);
}

TEST_F(SesameWindowedTestDouble, received_message_before_initialized_is_discarded)
{
    PretendReceiveMessage("abcd");

    ReceiveInitResponse(24);
}

TEST_F(SesameWindowedTestDouble, handle_init_request_after_initialization)
{
    ReceiveInitResponse(8);

    ExpectRequestSendMessageForInitResponse(24);
    EXPECT_CALL(initializer, InitializationRequested(testing::_)).WillOnce(testing::InvokeArgument<0>());
    ReceiveInitRequest(8);
}

TEST_F(SesameWindowedTestDouble, init_response_consumes_window)
{
    ReceiveInitResponse(24);

    ExpectRequestSendMessageForInitResponse(24);
    EXPECT_CALL(initializer, InitializationRequested(testing::_)).WillOnce(testing::InvokeArgument<0>());
    ReceiveInitRequest(5 + 9 + 9); // window for two messages and saving for releaseWindow

    ExpectRequestSendMessageForMessage(7, { 1, 2, 3, 4, 5, 6 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4, 5, 6 });
    communication->RequestSendMessage(6);

    communication->RequestSendMessage(6);

    ExpectRequestSendMessageForMessage(7, { 1, 2, 3, 4, 5, 6 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4, 5, 6 });
    ReceiveReleaseWindow(7); // release window consumed by initResponse
}

TEST_F(SesameWindowedTestDouble, received_init_request_while_sending_message_finishes_message_then_sends_init_response)
{
    // build
    ReceiveInitResponse(24);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailableAndSaveWriter();

    communication->RequestSendMessage(4);

    // operate
    EXPECT_CALL(initializer, InitializationRequested(testing::_)).WillOnce(testing::InvokeArgument<0>());
    ReceiveInitRequest(24);

    ExpectRequestSendMessageForInitResponse(24);

    {
        std::vector<uint8_t> data{ 1, 2, 3, 4 };
        infra::DataOutputStream::WithErrorPolicy stream(*savedWriter);
        stream << infra::MakeRange(data);
        savedWriter = nullptr;
    };
}

TEST_F(SesameWindowedTestDouble, increase_window_while_sending)
{
    // build
    ReceiveInitResponse(24);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    communication->RequestSendMessage(4);

    // operate
    ExpectRequestSendMessageForReleaseWindow(12);
    ReceiveReleaseWindow(17);

    // Send a new message that uses the newly announced window
    ExpectRequestSendMessageForMessage(3, { 5, 6 });
    ExpectSendMessageStreamAvailable({ 5, 6 });
    communication->RequestSendMessage(2);
}

TEST_F(SesameWindowedTestDouble, init_response_while_sending)
{
    // build
    ReceiveInitResponse(24);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });

    communication->RequestSendMessage(4);

    // operate
    ReceiveInitResponse(10);

    // Send a new message that uses the newly announced window
    ExpectRequestSendMessageForMessage(3, { 5, 6 });
    ExpectSendMessageStreamAvailable({ 5, 6 });
    communication->RequestSendMessage(2);
}

TEST_F(SesameWindowedTestDouble, received_init_request_while_sending_init)
{
    ExpectRequestSendMessageForInitResponse(24);
    EXPECT_CALL(initializer, InitializationRequested(testing::_)).WillOnce(testing::InvokeArgument<0>());
    ReceiveInitRequest(8);

    ReceiveInitResponseWithoutInitialized(24);
}

TEST_F(SesameWindowedTestDouble, init_response_while_sending_init_is_ignored)
{
    ReceiveInitResponse(8);

    ReceiveInitResponse(24);
}

TEST_F(SesameWindowedTestDouble, release_window_while_sending_init_is_ignored)
{
    ReceiveReleaseWindow(4);

    ReceiveInitResponse(24);
}

TEST_F(SesameWindowedTestDouble, requesting_message_while_sending_init_response)
{
    // build
    ExpectRequestSendMessageForInitResponse(24);
    EXPECT_CALL(initializer, InitializationRequested(testing::_)).WillOnce(testing::InvokeArgument<0>());
    ReceiveInitRequest(8);

    ReceiveInitResponseWithoutInitialized(24);

    // operate
    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    communication->RequestSendMessage(4);
}

TEST_F(SesameWindowedTestDouble, init_request_while_sending_init_response_results_in_new_init_response)
{
    // build
    ExpectRequestSendMessageForInitResponse(24);
    EXPECT_CALL(initializer, InitializationRequested(testing::_)).WillOnce(testing::InvokeArgument<0>());
    ReceiveInitRequest(8);

    ReceiveInitResponseWithoutInitialized(24);

    // operate
    ExpectRequestSendMessageForInitResponse(24);
    EXPECT_CALL(initializer, InitializationRequested(testing::_)).WillOnce(testing::InvokeArgument<0>());
    ReceiveInitRequest(8);
}

TEST_F(SesameWindowedTestDouble, release_window_while_sending_init_response_is_ignored)
{
    // build
    ExpectRequestSendMessageForInitResponse(24);
    EXPECT_CALL(initializer, InitializationRequested(testing::_)).WillOnce(testing::InvokeArgument<0>());
    ReceiveInitRequest(8);

    ReceiveInitResponseWithoutInitialized(24);

    // operate
    ExpectRequestSendMessageForReleaseWindow(12);
    ReceiveReleaseWindow(4);
}

TEST_F(SesameWindowedTestDouble, no_window_release_after_small_message)
{
    // build
    ReceiveInitResponse(24);
    ExpectReceivedMessage("ab");
    ExpectRequestSendMessageForReleaseWindow(12);
    ReceiveMessage("ab");

    // operate
    ExpectReceivedMessage("ab");
    // ExpectRequestSendMessageForReleaseWindow(12);
    ReceiveMessage("ab");
}

TEST_F(SesameWindowedTestDouble, no_window_release_after_just_release_window)
{
    // build
    ReceiveInitResponse(24);
    ExpectReceivedMessage("ab");
    ExpectRequestSendMessageForReleaseWindow(12);
    ReceiveMessage("ab");

    // operate
    // ExpectRequestSendMessageForReleaseWindow(5);
    ReceiveReleaseWindow(50);
}

TEST_F(SesameWindowedTestDouble, window_release_after_second_release_window)
{
    // build
    ReceiveInitResponse(24);
    ExpectReceivedMessage("ab");
    ExpectRequestSendMessageForReleaseWindow(12);
    ReceiveMessage("ab");

    // operate
    ExpectRequestSendMessageForReleaseWindow(10);
    ReceiveReleaseWindow(50);
    ReceiveReleaseWindow(50);
}

TEST_F(SesameWindowedTestDouble, window_is_released_after_message_has_been_processed)
{
    ReceiveInitResponse(24);

    ExpectReceivedMessageAndSaveReader("abcd");
    ReceiveMessage("abcd");

    ExpectRequestSendMessageForReleaseWindow(14);
    savedReader = nullptr;
}

TEST_F(SesameWindowedTestDouble, no_new_message_after_ResetReading)
{
    ReceiveInitResponse(24);

    ExpectReceivedMessageAndSaveReader("abcd");
    ReceiveMessage("abcd");

    // ExpectRequestSendMessageForReleaseWindow(14);
    communication->ResetReading();
    savedReader = nullptr;

    ExecuteAllActions();
}

TEST_F(SesameWindowedTestDouble, Reset_forwards_to_cobs_and_requests_initialize)
{
    ReceiveInitResponse(24);

    EXPECT_CALL(base, Reset());
    communication->Reset();

    ExpectRequestSendMessageForInitResponse(24);
    EXPECT_CALL(initializer, InitializationRequested(testing::_)).WillOnce(testing::InvokeArgument<0>());
    ReceiveInitRequest(8);

    ReceiveInitResponseWithoutInitialized(24);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    communication->RequestSendMessage(4);
}

TEST_F(SesameWindowedTestDouble, hold_initialization_until_initializer_grants)
{
    observer.Detach();
    EXPECT_CALL(base, WorstCaseEncodedMessageSize(3)).WillOnce(testing::Return(5));
    ExpectRequestSendMessageForInit(24);
    infra::ReConstruct(static_cast<decltype(communicationInstance)&>(*communication), base, initializer);
    observer.Attach(*communication);

    infra::Function<void()> onGranted;
    EXPECT_CALL(initializer, InitializationRequested(testing::_)).WillOnce(testing::SaveArg<0>(&onGranted));
    ReceiveInitRequest(24);

    // A message received before sending InitResponse is discarded
    ReceivePacket(infra::ConstructBin()(4)("abcd").Vector());

    ExpectRequestSendMessageForInitResponse(24);
    onGranted();
}

TEST_F(SesameWindowedTestTriple, MaxSendMessageSize_for_3_way_buffer)
{
    ReceiveInitResponse(32);
    // 6 bytes in a message expands to 1 (cobs) + 1 (operation) + 6 (message) + 1 (delimiter) = 9
    EXPECT_EQ(6, communication->MaxSendMessageSize());
    EXPECT_EQ(9, (services::SesameWindowed::bufferSizeForMessage<6, services::SesameCobs::EncodedMessageSize>));

    observer.Detach();
}
