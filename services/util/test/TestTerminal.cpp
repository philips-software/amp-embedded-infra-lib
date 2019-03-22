#include "gmock/gmock.h"
#include "hal/interfaces/test_doubles/SerialCommunicationMock.hpp"
#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/util/Terminal.hpp"
#include <vector>

class TerminalTest
    : public testing::Test
    , public infra::EventDispatcherWithWeakPtrFixture
{
public:
    TerminalTest()
    {
        EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

        terminal.Emplace(communication);
    }

protected:
    testing::StrictMock<hal::SerialCommunicationMock> communication;
    infra::Optional<services::Terminal> terminal;
    testing::InSequence s;
};

class TerminalNavigationTest
    : public TerminalTest
{
public:
    TerminalNavigationTest()
    {
        EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'a' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
        EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'b' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
        EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'c' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
        EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'd' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

        communication.dataReceived(std::vector<uint8_t>{ 'a', 'b', 'c', 'd' });
    }
};

class TerminalHistoryTest
    : public TerminalTest
{
public:
    TerminalHistoryTest()
    {
        EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'a' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
        EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'b' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
        EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'c' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
        EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'd' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
        EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\r', '\n' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
        EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

        communication.dataReceived(std::vector<uint8_t>{ 'a', 'b', 'c', 'd', '\r' });

        EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'e' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
        EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'f' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
        EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'g' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
        EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\r', '\n' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
        EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

        communication.dataReceived(std::vector<uint8_t>{ 'e', 'f', 'g', '\r' });
    }
};

TEST_F(TerminalTest, echo_printable_characters)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'A' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'Z' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '~' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ ' ', 'A', 'Z', '~' });

    ExecuteAllActions();
}

TEST_F(TerminalTest, echo_bell_for_nonprintable_characters)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\a' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\a' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 31, 128 });

    ExecuteAllActions();
}

TEST_F(TerminalTest, ignore_newline_character)
{
    communication.dataReceived(std::vector<uint8_t>{ '\n' });

    ExecuteAllActions();
}

TEST_F(TerminalTest, echo_carriage_return_line_feed_and_prompt_on_carriage_return)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\r', '\n' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ '\r' });

    ExecuteAllActions();
}

TEST_F(TerminalTest, ignore_unparsed_escaped_data)
{
    communication.dataReceived(std::vector<uint8_t>{ 27, ';', '[', 'O', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' });
    
    ExecuteAllActions();
}

TEST_F(TerminalTest, bell_on_invalid_escape_sequence)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\a' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 27, 'Z' });

    ExecuteAllActions();
}

TEST_F(TerminalTest, navigation_with_empty_buffer)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\r' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\r' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 1, 1, 5, 5 });

    ExecuteAllActions();
}

TEST_F(TerminalTest, clear_buffer_prints_prompt)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\r' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 3 });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, move_cursor_home)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\r' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\r' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 1, 1 });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, move_cursor_home_escape)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\r' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 27, 'H' });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, move_cursor_end)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\r' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'a', 'b', 'c', 'd' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 1, 5, 5 });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, move_cursor_end_escape)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\r' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'a', 'b', 'c', 'd' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 1, 27, 'F' });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, move_cursor_left)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\b' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 2 });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, move_cursor_left_escape)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\b' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 27, 'D' });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, bell_on_move_past_beginning)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\b' })).Times(4).WillRepeatedly(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\a' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 2, 2, 2, 2, 2 });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, move_cursor_right)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\b' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'd' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 2, 6 });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, move_cursor_right_escape)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\b' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'd' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 2, 27, 'C' });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, bell_on_move_past_end)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\a' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 6 });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, remove_last_character_on_backspace)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\b' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ ' ', '\b' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ '\b' });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, remove_first_character_on_backspace)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\b' })).Times(4).WillRepeatedly(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'b', 'c', 'd' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ ' ', '\b' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\b' })).Times(3).WillRepeatedly(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 2, 2, 2, '\b' });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, remove_character_with_delete)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\b' })).Times(3).WillRepeatedly(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'c', 'd' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ ' ', '\b' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\b' })).Times(2).WillRepeatedly(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 2, 2, 2, 4 });

    ExecuteAllActions();
}

TEST_F(TerminalTest, move_backward_in_history_with_empty_history)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\r' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 14 });

    ExecuteAllActions();
}

TEST_F(TerminalTest, move_forward_in_history_with_empty_history)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\a' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 16 });

    ExecuteAllActions();
}

TEST_F(TerminalHistoryTest, move_backward_in_history)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\r' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'e', 'f', 'g' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\r' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 'a', 'b', 'c', 'd' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\a' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 16, 16, 16 });

    ExecuteAllActions();
}

TEST_F(TerminalHistoryTest, move_forward_past_end_prints_prompt)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\r' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '\r' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ '>', ' ' })).WillOnce(infra::Lambda([this](std::vector<uint8_t>) { communication.actionOnCompletion(); }));

    communication.dataReceived(std::vector<uint8_t>{ 14, 14 });

    ExecuteAllActions();
}
