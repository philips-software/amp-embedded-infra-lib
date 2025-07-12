#include "services/cucumber/CucumberStepStorage.hpp"
#include "gmock/gmock.h"

namespace
{
    class StepStub
        : public services::CucumberStepProgress
    {
    public:
        explicit StepStub(infra::BoundedConstString stepName, infra::BoundedConstString stepFile)
            : services::CucumberStepProgress(stepName, stepFile)
        {}

        void Execute() override
        {}
    };
}

TEST(CucumberStepStorageTest, Instance_returns_singleton)
{
    services::CucumberStepStorage storage;

    auto& instance1 = services::CucumberStepStorage::Instance();
    auto& instance2 = services::CucumberStepStorage::Instance();

    EXPECT_EQ(&instance1, &instance2);
}

TEST(CucumberStepStorageTest, AddStep_and_MatchStep_single_step)
{
    services::CucumberStepStorage storage;
    StepStub step("Given I have a test step", "test.cpp:10");

    storage.AddStep(step);

    auto match = storage.MatchStep("Given I have a test step");

    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Success, match.result);
    EXPECT_EQ(&step, match.step);
    EXPECT_EQ(0u, match.id);
}

TEST(CucumberStepStorageTest, MatchStep_no_match_returns_fail)
{
    services::CucumberStepStorage storage;
    StepStub step("Given I have a test step", "test.cpp:10");
    storage.AddStep(step);

    auto match = storage.MatchStep("Given I have a different step");

    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Fail, match.result);
    EXPECT_EQ(nullptr, match.step);
}

TEST(CucumberStepStorageTest, MatchStep_duplicate_steps_returns_duplicate)
{
    services::CucumberStepStorage storage;
    StepStub step1("Given I have a test step", "test1.cpp:10");
    StepStub step2("Given I have a test step", "test2.cpp:20");

    storage.AddStep(step1);
    storage.AddStep(step2);

    auto match = storage.MatchStep("Given I have a test step");

    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Duplicate, match.result);
}

TEST(CucumberStepStorageTest, GetStep_returns_correct_step)
{
    services::CucumberStepStorage storage;
    StepStub step1("Given first step", "test.cpp:10");
    StepStub step2("Given second step", "test.cpp:20");
    StepStub step3("Given third step", "test.cpp:30");

    storage.AddStep(step1);
    storage.AddStep(step2);
    storage.AddStep(step3);

    EXPECT_EQ(&step1, &storage.GetStep(0));
    EXPECT_EQ(&step2, &storage.GetStep(1));
    EXPECT_EQ(&step3, &storage.GetStep(2));
}

TEST(CucumberStepStorageTest, DeleteStep_removes_step_from_storage)
{
    services::CucumberStepStorage storage;
    StepStub step1("Given first step", "test.cpp:10");
    StepStub step2("Given second step", "test.cpp:20");

    storage.AddStep(step1);
    storage.AddStep(step2);

    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Success,
        storage.MatchStep("Given first step").result);
    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Success,
        storage.MatchStep("Given second step").result);

    storage.DeleteStep(step1);

    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Fail,
        storage.MatchStep("Given first step").result);
    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Success,
        storage.MatchStep("Given second step").result);
}

TEST(CucumberStepStorageTest, ClearStorage_removes_all_steps)
{
    services::CucumberStepStorage storage;
    StepStub step1("Given first step", "test.cpp:10");
    StepStub step2("Given second step", "test.cpp:20");

    storage.AddStep(step1);
    storage.AddStep(step2);

    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Success,
        storage.MatchStep("Given first step").result);
    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Success,
        storage.MatchStep("Given second step").result);

    storage.ClearStorage();

    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Fail,
        storage.MatchStep("Given first step").result);
    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Fail,
        storage.MatchStep("Given second step").result);
}

TEST(CucumberStepStorageTest, MatchesStepName_literal_match)
{
    services::CucumberStepStorage storage;
    StepStub step("Given I have a literal step", "test.cpp:10");

    EXPECT_TRUE(storage.MatchesStepName(step, "Given I have a literal step"));
    EXPECT_FALSE(storage.MatchesStepName(step, "Given I have a different step"));
    EXPECT_FALSE(storage.MatchesStepName(step, "Given I have a literal"));
    EXPECT_FALSE(storage.MatchesStepName(step, "Given I have a literal step extra"));
}

TEST(CucumberStepStorageTest, MatchesStepName_string_parameter)
{
    services::CucumberStepStorage storage;
    StepStub step("Given I have a '%s' parameter", "test.cpp:10");

    EXPECT_TRUE(storage.MatchesStepName(step, "Given I have a 'test' parameter"));
    EXPECT_TRUE(storage.MatchesStepName(step, "Given I have a 'hello world' parameter"));
    EXPECT_TRUE(storage.MatchesStepName(step, "Given I have a 'with spaces' parameter"));
    EXPECT_FALSE(storage.MatchesStepName(step, "Given I have a test parameter"));
    EXPECT_FALSE(storage.MatchesStepName(step, "Given I have a 'test"));
    EXPECT_FALSE(storage.MatchesStepName(step, "Given I have a test' parameter"));
}

TEST(CucumberStepStorageTest, MatchesStepName_integer_parameter)
{
    services::CucumberStepStorage storage;
    StepStub step("Given I have %d items", "test.cpp:10");

    EXPECT_TRUE(storage.MatchesStepName(step, "Given I have 42 items"));
    EXPECT_TRUE(storage.MatchesStepName(step, "Given I have 0 items"));
    EXPECT_TRUE(storage.MatchesStepName(step, "Given I have 123456 items"));
    EXPECT_FALSE(storage.MatchesStepName(step, "Given I have abc items"));
    EXPECT_FALSE(storage.MatchesStepName(step, "Given I have -42 items"));
    EXPECT_FALSE(storage.MatchesStepName(step, "Given I have 42.5 items"));
}

TEST(CucumberStepStorageTest, MatchesStepName_boolean_parameter)
{
    services::CucumberStepStorage storage;
    StepStub step("Given the setting is %b", "test.cpp:10");

    EXPECT_TRUE(storage.MatchesStepName(step, "Given the setting is true"));
    EXPECT_TRUE(storage.MatchesStepName(step, "Given the setting is false"));
    EXPECT_FALSE(storage.MatchesStepName(step, "Given the setting is yes"));
    EXPECT_FALSE(storage.MatchesStepName(step, "Given the setting is no"));
    EXPECT_FALSE(storage.MatchesStepName(step, "Given the setting is 1"));
    EXPECT_FALSE(storage.MatchesStepName(step, "Given the setting is 0"));
}

TEST(CucumberStepStorageTest, MatchesStepName_multiple_parameters)
{
    services::CucumberStepStorage storage;
    StepStub step("Given I have '%s' with %d items and %b flag", "test.cpp:10");

    EXPECT_TRUE(storage.MatchesStepName(step, "Given I have 'test' with 42 items and true flag"));
    EXPECT_TRUE(storage.MatchesStepName(step, "Given I have 'hello world' with 0 items and false flag"));
    EXPECT_FALSE(storage.MatchesStepName(step, "Given I have test with 42 items and true flag"));
    EXPECT_FALSE(storage.MatchesStepName(step, "Given I have 'test' with abc items and true flag"));
    EXPECT_FALSE(storage.MatchesStepName(step, "Given I have 'test' with 42 items and maybe flag"));
}

TEST(CucumberStepStorageTest, MatchesStepName_parameter_at_beginning)
{
    services::CucumberStepStorage storage;
    StepStub stringStep("'%s' is at the beginning", "test.cpp:10");
    StepStub intStep("%d is at the beginning", "test.cpp:20");
    StepStub boolStep("%b is at the beginning", "test.cpp:30");

    EXPECT_TRUE(storage.MatchesStepName(stringStep, "'hello' is at the beginning"));
    EXPECT_TRUE(storage.MatchesStepName(intStep, "42 is at the beginning"));
    EXPECT_TRUE(storage.MatchesStepName(boolStep, "true is at the beginning"));

    EXPECT_FALSE(storage.MatchesStepName(stringStep, "hello is at the beginning"));
    EXPECT_FALSE(storage.MatchesStepName(intStep, "abc is at the beginning"));
    EXPECT_FALSE(storage.MatchesStepName(boolStep, "maybe is at the beginning"));
}

TEST(CucumberStepStorageTest, MatchesStepName_parameter_at_end)
{
    services::CucumberStepStorage storage;
    StepStub stringStep("Parameter at end '%s'", "test.cpp:10");
    StepStub intStep("Parameter at end %d", "test.cpp:20");
    StepStub boolStep("Parameter at end %b", "test.cpp:30");

    EXPECT_TRUE(storage.MatchesStepName(stringStep, "Parameter at end 'hello'"));
    EXPECT_TRUE(storage.MatchesStepName(intStep, "Parameter at end 42"));
    EXPECT_TRUE(storage.MatchesStepName(boolStep, "Parameter at end false"));

    EXPECT_FALSE(storage.MatchesStepName(stringStep, "Parameter at end hello"));
    EXPECT_FALSE(storage.MatchesStepName(intStep, "Parameter at end abc"));
    EXPECT_FALSE(storage.MatchesStepName(boolStep, "Parameter at end maybe"));
}

TEST(CucumberStepStorageTest, MatchesStepName_only_parameter)
{
    services::CucumberStepStorage storage;
    StepStub stringStep("'%s'", "test.cpp:10");
    StepStub intStep("%d", "test.cpp:20");
    StepStub boolStep("%b", "test.cpp:30");

    EXPECT_TRUE(storage.MatchesStepName(stringStep, "'hello world'"));
    EXPECT_TRUE(storage.MatchesStepName(intStep, "42"));
    EXPECT_TRUE(storage.MatchesStepName(boolStep, "true"));

    EXPECT_FALSE(storage.MatchesStepName(stringStep, "hello world"));
    EXPECT_FALSE(storage.MatchesStepName(intStep, "abc"));
    EXPECT_FALSE(storage.MatchesStepName(boolStep, "yes"));
}

TEST(CucumberStepStorageTest, MatchesStepName_empty_strings)
{
    services::CucumberStepStorage storage;
    StepStub emptyStep("", "test.cpp:10");

    EXPECT_TRUE(storage.MatchesStepName(emptyStep, ""));
    EXPECT_FALSE(storage.MatchesStepName(emptyStep, "not empty"));
}

TEST(CucumberStepStorageTest, MatchesStepName_complex_string_content)
{
    services::CucumberStepStorage storage;
    StepStub step("Given I send message '%s'", "test.cpp:10");

    EXPECT_TRUE(storage.MatchesStepName(step, "Given I send message 'hello world with spaces'"));
    EXPECT_TRUE(storage.MatchesStepName(step, "Given I send message 'with-dashes-and_underscores'"));
    EXPECT_TRUE(storage.MatchesStepName(step, "Given I send message 'with123numbers'"));
    EXPECT_TRUE(storage.MatchesStepName(step, "Given I send message 'with!@#$%special^&*()chars'"));
}

TEST(CucumberStepStorageTest, MatchStep_with_multiple_matching_and_non_matching_steps)
{
    services::CucumberStepStorage storage;
    StepStub step1("Given I have step one", "test.cpp:10");
    StepStub step2("Given I have step '%s'", "test.cpp:20");
    StepStub step3("Given I have step three", "test.cpp:30");
    StepStub step4("When I perform action", "test.cpp:40");

    storage.AddStep(step1);
    storage.AddStep(step2);
    storage.AddStep(step3);
    storage.AddStep(step4);

    auto match1 = storage.MatchStep("Given I have step one");
    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Success, match1.result);
    EXPECT_EQ(&step1, match1.step);
    EXPECT_EQ(0u, match1.id);

    auto match2 = storage.MatchStep("Given I have step 'two'");
    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Success, match2.result);
    EXPECT_EQ(&step2, match2.step);
    EXPECT_EQ(1u, match2.id);

    auto matchFail = storage.MatchStep("Given I have step that does not exist");
    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Fail, matchFail.result);
    EXPECT_EQ(nullptr, matchFail.step);

    auto match4 = storage.MatchStep("When I perform action");
    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Success, match4.result);
    EXPECT_EQ(&step4, match4.step);
    EXPECT_EQ(3u, match4.id);
}

TEST(CucumberStepStorageTest, MatchStep_duplicate_exact_matches)
{
    services::CucumberStepStorage storage;
    StepStub step1("Given duplicate step", "test1.cpp:10");
    StepStub step2("Given duplicate step", "test2.cpp:20");
    StepStub step3("Given unique step", "test3.cpp:30");

    storage.AddStep(step1);
    storage.AddStep(step2);
    storage.AddStep(step3);

    auto duplicateMatch = storage.MatchStep("Given duplicate step");
    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Duplicate, duplicateMatch.result);

    auto uniqueMatch = storage.MatchStep("Given unique step");
    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Success, uniqueMatch.result);
    EXPECT_EQ(&step3, uniqueMatch.step);
}

TEST(CucumberStepStorageTest, MatchStep_duplicate_parameterized_matches)
{
    services::CucumberStepStorage storage;
    StepStub step1("Given I have '%s' parameter", "test1.cpp:10");
    StepStub step2("Given I have '%s' parameter", "test2.cpp:20");

    storage.AddStep(step1);
    storage.AddStep(step2);

    auto duplicateMatch = storage.MatchStep("Given I have 'test' parameter");
    EXPECT_EQ(services::CucumberStepStorage::StepMatchResult::Duplicate, duplicateMatch.result);
}
