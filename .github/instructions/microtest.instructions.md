---
applyTo: "**/test/**, **/test_doubles/**"
---

## Google Test Suite Coding Rules

- `using namespace testing;` is not allowed; use fully qualified names instead.
- Code comments are not allowed inside tests; use descriptive test and subtest names instead.
- Always prefer `EXPECT_THAT` macros over `EXPECT_EQ` macros for better failure messages.
- When testing outputs, prefer using matchers from the Google Test Matchers library, to match the full output instead of parts of it.
- Always prefer `EXPECT_THAT` over `ASSERT_THAT`, unless the test cannot continue or for documenting pre-conditions.
- Avoid death tests unless absolutely necessary, as they can slow down the test suite significantly.
- Avoid iterating over elements in tests; use matchers that can handle collections instead.
- Whenever a parameter is needed for multiple tests, use test fixtures or parameterized functions to avoid code duplication.
- Whenever a behaviour is needed for multiple tests, use parameterized functions to avoid code duplication.
- Default parameters are allowed when writing test helper functions to increase readability.
- Prefer using test helper functions as public members of the test fixture class instead of free functions in the test file.
- Always use `MOCK_METHOD`; never use outdated `MOCK_METHOD*` macros (e.g. `MOCK_METHOD0`, `MOCK_METHOD1`) to keep tests readable and maintainable.
- Do not split a single scenario across multiple tests; if steps in a flow belong to the same scenario, combine them into one test that covers the complete sequence.
- Guard death tests with `#ifndef EMIL_MUTATION_TESTING` to avoid running them during mutation testing. They are very slow and can crash the runner.
