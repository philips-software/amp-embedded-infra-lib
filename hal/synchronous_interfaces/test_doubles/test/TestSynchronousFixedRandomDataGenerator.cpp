#include "gtest/gtest.h"
#include "hal/synchronous_interfaces/test_doubles/SynchronousFixedRandomDataGenerator.hpp"

class SynchronousFixedRandomDataGeneratorTest
    : public testing::Test
{
public:
    std::vector<uint8_t> randomData{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
};

TEST_F(SynchronousFixedRandomDataGeneratorTest, generates_fixed_data)
{
    hal::SynchronousFixedRandomDataGenerator rng{ randomData };
    std::vector<uint8_t> output(20, 255);

    rng.GenerateRandomData(infra::MakeRange(output));

    EXPECT_EQ((std::vector<uint8_t>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }), output);
}

TEST_F(SynchronousFixedRandomDataGeneratorTest, consumes_initial_data_when_generating)
{
    hal::SynchronousFixedRandomDataGenerator rng{ randomData };
    std::vector<uint8_t> output(10, 255);

    rng.GenerateRandomData(infra::MakeRange(output));
    rng.GenerateRandomData(infra::MakeRange(output));

    EXPECT_EQ((std::vector<uint8_t>{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }), output);
}

TEST_F(SynchronousFixedRandomDataGeneratorTest, output_smaller_than_initial_data)
{
    hal::SynchronousFixedRandomDataGenerator rng{ randomData };
    std::vector<uint8_t> output(5, 255);

    rng.GenerateRandomData(infra::MakeRange(output));

    EXPECT_EQ((std::vector<uint8_t>{ 0, 1, 2, 3, 4 }), output);
}

TEST_F(SynchronousFixedRandomDataGeneratorTest, deplete_buffer_in_chunks)
{
    hal::SynchronousFixedRandomDataGenerator rng{ std::vector<uint8_t>(12, 128) };
    std::vector<uint8_t> output(5, 255);

    rng.GenerateRandomData(output);
    EXPECT_EQ((std::vector<uint8_t>{ 128, 128, 128, 128, 128 }), output);

    rng.GenerateRandomData(output);
    EXPECT_EQ((std::vector<uint8_t>{ 128, 128, 128, 128, 128 }), output);

    rng.GenerateRandomData(output);
    EXPECT_EQ((std::vector<uint8_t>{ 128, 128, 0, 0, 0 }), output);
}
