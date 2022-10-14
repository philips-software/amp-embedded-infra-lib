#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "infra/util/Sequencer.hpp"

class TestSequencer
    : public testing::StrictMock<testing::Test>
{
public:
    MOCK_METHOD0(a, void());
    MOCK_METHOD0(b, void());
    MOCK_METHOD0(c, void());

    MOCK_METHOD1(p1, void(uint32_t));

    MOCK_CONST_METHOD0(condition, bool());

    infra::Sequencer sequencer;
};

TEST_F(TestSequencer, can_load_empty_sequence)
{
    sequencer.Load([this]() {});
}

TEST_F(TestSequencer, executing_empty_sequence_results_in_Finished)
{
    sequencer.Load([this]() {});
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, executing_one_step_results_in_not_Finished)
{
    EXPECT_CALL(*this, a());

    sequencer.Load([this]()
        { sequencer.Step([this]()
              { a(); }); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, executing_Execute_results_in_Finished)
{
    EXPECT_CALL(*this, a());

    sequencer.Load([this]()
        { sequencer.Execute([this]()
              { a(); }); });

    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, two_steps_result_in_first_step_executed)
{
    EXPECT_CALL(*this, a());

    sequencer.Load([this]()
        {
        sequencer.Step([this]() { a(); });
        sequencer.Step([this]() { b(); }); });

    EXPECT_FALSE(sequencer.Finished());
}

TEST_F(TestSequencer, after_continue_second_step_is_executed)
{
    EXPECT_CALL(*this, a());
    EXPECT_CALL(*this, b());

    sequencer.Load([this]()
        {
        sequencer.Step([this]() { a(); });
        sequencer.Step([this]() { b(); }); });

    sequencer.Continue();
    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, nested_steps_are_executed_in_sequence)
{
    EXPECT_CALL(*this, a());

    sequencer.Load([this]()
        { sequencer.Execute([this]()
              {
            sequencer.Step([this]() { a(); });
            sequencer.Step([this]() { b(); }); }); });

    EXPECT_FALSE(sequencer.Finished());
    testing::Mock::VerifyAndClearExpectations(this);

    EXPECT_CALL(*this, b());
    sequencer.Continue();
    EXPECT_FALSE(sequencer.Finished());
    testing::Mock::VerifyAndClearExpectations(this);

    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_unsuccessful_condition_If_does_not_execute_statement)
{
    EXPECT_CALL(*this, condition()).WillOnce(testing::Return(false));
    EXPECT_CALL(*this, b());

    sequencer.Load([this]()
        {
        sequencer.If([this] { return condition(); }); 
            sequencer.Step([this]() { a(); });
        sequencer.EndIf();
        sequencer.Step([this]() { b(); }); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_unsuccessful_condition_If_does_not_execute_multiple_statements)
{
    EXPECT_CALL(*this, condition()).WillOnce(testing::Return(false));
    EXPECT_CALL(*this, c());

    sequencer.Load([this]()
        {
        sequencer.If([this] { return condition(); });
            sequencer.Step([this]() { a(); });
            sequencer.Step([this]() { b(); });
        sequencer.EndIf();
        sequencer.Step([this]() { c(); }); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_successful_condition_If_executes_statement)
{
    EXPECT_CALL(*this, condition()).WillOnce(testing::Return(true));
    EXPECT_CALL(*this, a());

    sequencer.Load([this]()
        {
        sequencer.If([this] { return condition(); });
            sequencer.Step([this]() { a(); });
        sequencer.EndIf();
        sequencer.Step([this]() { b(); }); });

    EXPECT_CALL(*this, b());
    sequencer.Continue();
    EXPECT_FALSE(sequencer.Finished());

    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_successful_condition_If_executes_multiple_statements)
{
    EXPECT_CALL(*this, condition()).WillOnce(testing::Return(true));
    EXPECT_CALL(*this, a());
    EXPECT_CALL(*this, b());
    EXPECT_CALL(*this, c());

    sequencer.Load([this]()
        {
        sequencer.If([this] { return condition(); });
            sequencer.Step([this]() { a(); });
            sequencer.Step([this]() { b(); });
        sequencer.EndIf();
        sequencer.Step([this]() { c(); }); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, nested_If_does_not_release_If)
{
    EXPECT_CALL(*this, condition()).WillOnce(testing::Return(false));
    EXPECT_CALL(*this, b());

    sequencer.Load([this]()
        {
        sequencer.If([this] { return condition(); });
            sequencer.If([this](){ return condition(); });
            sequencer.EndIf();
        sequencer.EndIf();
        sequencer.Step([this]() { b(); }); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_unsuccessful_condition_IfElse_executes_Else_statement)
{
    EXPECT_CALL(*this, condition()).WillOnce(testing::Return(false));
    EXPECT_CALL(*this, b());

    sequencer.Load([this]()
        {
        sequencer.If([this] { return condition(); });
            sequencer.Step([this]() { a(); }); 
        sequencer.Else(); 
            sequencer.Step([this]() { b(); });
        sequencer.EndIf(); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_successful_condition_IfElse_executes_If_statement)
{
    EXPECT_CALL(*this, condition()).WillOnce(testing::Return(true));
    EXPECT_CALL(*this, a());

    sequencer.Load([this]()
        {
        sequencer.If([this] { return condition(); });
            sequencer.Step([this]() { a(); });
        sequencer.Else();
            sequencer.Step([this]() { b(); });
        sequencer.EndIf(); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_successful_second_condition_If_ElseIf_executes_ElseIf_statement)
{
    EXPECT_CALL(*this, condition()).WillOnce(testing::Return(false)).WillOnce(testing::Return(true));
    EXPECT_CALL(*this, b());

    sequencer.Load([this]()
        {
        sequencer.If([this] { return condition(); });
            sequencer.Step([this]() { a(); });
        sequencer.ElseIf([this] { return condition(); });
            sequencer.Step([this]() { b(); });
        sequencer.EndIf(); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_successful_second_condition_If_ElseIf_Else_executes_If_statement)
{
    EXPECT_CALL(*this, condition()).WillOnce(testing::Return(true));
    EXPECT_CALL(*this, a());

    sequencer.Load([this]()
        {
        sequencer.If([this] { return condition(); });
            sequencer.Step([this]() { a(); });
        sequencer.ElseIf([this] { return condition(); });
            sequencer.Step([this]() { b(); });
        sequencer.Else();
            sequencer.Step([this]() { c(); });
        sequencer.EndIf(); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_successful_second_condition_If_ElseIf_Else_executes_ElseIf_statement)
{
    EXPECT_CALL(*this, condition()).WillOnce(testing::Return(false)).WillOnce(testing::Return(true));
    EXPECT_CALL(*this, b());

    sequencer.Load([this]()
        {
        sequencer.If([this] { return condition(); });
            sequencer.Step([this]() { a(); });
        sequencer.ElseIf([this] { return condition(); });
            sequencer.Step([this]() { b(); });
        sequencer.Else();
            sequencer.Step([this]() { c(); });
        sequencer.EndIf(); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_unsuccessful_second_condition_If_ElseIf_executes_Else_statement)
{
    EXPECT_CALL(*this, condition()).WillOnce(testing::Return(false)).WillOnce(testing::Return(false));
    EXPECT_CALL(*this, c());

    sequencer.Load([this]()
        {
        sequencer.If([this] { return condition(); });
            sequencer.Step([this]() { a(); });
        sequencer.ElseIf([this] { return condition(); });
            sequencer.Step([this]() { b(); });
        sequencer.Else();
            sequencer.Step([this]() { c(); });
        sequencer.EndIf(); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_successful_condition_IfElseIfElse_executes_If_statement)
{
    EXPECT_CALL(*this, condition()).WillOnce(testing::Return(true));
    EXPECT_CALL(*this, a());

    sequencer.Load([this]()
        {
        sequencer.If([this] { return condition(); });
            sequencer.Step([this]() { a(); });
        sequencer.Else();
            sequencer.If([this] { return condition(); });
                sequencer.Step([this]() { b(); });
            sequencer.Else();
                sequencer.Step([this]() { c(); }); 
            sequencer.EndIf();
       sequencer.EndIf(); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_successful_condition_IfElseIfElse_executes_ElseIf_statement)
{
    EXPECT_CALL(*this, condition()).Times(2).WillOnce(testing::Return(false)).WillOnce(testing::Return(true));
    EXPECT_CALL(*this, b());

    sequencer.Load([this]()
        {
        sequencer.If([this] { return condition(); });
            sequencer.Step([this]() { a(); });
        sequencer.Else();
            sequencer.If([this] { return condition(); });
                sequencer.Step([this](){ b(); });
            sequencer.Else();
                sequencer.Step([this]() { c(); });
            sequencer.EndIf();
        sequencer.EndIf(); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_successful_condition_IfElseIfElse_executes_ElseElse_statement)
{
    EXPECT_CALL(*this, condition()).Times(2).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*this, c());

    sequencer.Load([this]()
        {
        sequencer.If([this] { return condition(); });
            sequencer.Step([this]() { a(); });
        sequencer.Else();
            sequencer.If([this] { return condition(); });
                sequencer.Step([this](){ b(); });
            sequencer.Else();
                sequencer.Step([this]() { c(); });
            sequencer.EndIf();
        sequencer.EndIf(); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_unsuccessful_condition_While_does_not_execute_statement)
{
    EXPECT_CALL(*this, condition())
        .WillOnce(testing::Return(false));
    EXPECT_CALL(*this, b());

    sequencer.Load([this]()
        {
        sequencer.While([this] { return condition(); });
            sequencer.Step([this]() { a(); });
        sequencer.EndWhile();
        sequencer.Step([this]() { b(); }); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_successful_condition_While_executes_statement_once)
{
    EXPECT_CALL(*this, condition())
        .WillOnce(testing::Return(true))
        .WillOnce(testing::Return(false));
    EXPECT_CALL(*this, a());
    EXPECT_CALL(*this, b());

    sequencer.Load([this]()
        {
        sequencer.While([this] { return condition(); });
            sequencer.Step([this]() { a(); });
        sequencer.EndWhile();
        sequencer.Step([this]() { b(); }); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_twice_successful_condition_While_executes_statement_twice)
{
    EXPECT_CALL(*this, condition())
        .WillOnce(testing::Return(true))
        .WillOnce(testing::Return(true))
        .WillOnce(testing::Return(false));
    EXPECT_CALL(*this, a()).Times(2);
    EXPECT_CALL(*this, b());

    sequencer.Load([this]()
        {
        sequencer.While([this] { return condition(); });
            sequencer.Step([this]() { a(); });
        sequencer.EndWhile();
        sequencer.Step([this]() { b(); }); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_unsuccessful_condition_DoWhile_executes_statement_once)
{
    EXPECT_CALL(*this, condition())
        .WillOnce(testing::Return(false));
    EXPECT_CALL(*this, a());
    EXPECT_CALL(*this, b());

    sequencer.Load([this]()
        {
        sequencer.DoWhile();
            sequencer.Step([this]() { a(); });
        sequencer.EndDoWhile([this] { return condition(); });
        sequencer.Step([this]() { b(); }); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, on_successful_condition_DoWhile_executes_statement_twice)
{
    EXPECT_CALL(*this, condition())
        .WillOnce(testing::Return(true))
        .WillOnce(testing::Return(false));
    EXPECT_CALL(*this, a()).Times(2);
    EXPECT_CALL(*this, b());

    sequencer.Load([this]()
        {
        sequencer.DoWhile();
            sequencer.Step([this]() { a(); });
        sequencer.EndDoWhile([this] { return condition(); });
        sequencer.Step([this]() { b(); }); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}

TEST_F(TestSequencer, ForEach_iterates_twice)
{
    EXPECT_CALL(*this, a()).Times(2);
    EXPECT_CALL(*this, b());

    uint32_t x;
    sequencer.Load([this, &x]()
        {
        sequencer.ForEach(x, 0, 2);
            sequencer.Step([this]() { a(); });
        sequencer.EndForEach(x);
        sequencer.Step([this]() { b(); }); });

    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_FALSE(sequencer.Finished());
    sequencer.Continue();
    EXPECT_TRUE(sequencer.Finished());
}
