#include "gtest/gtest.h"
#include "hal/interfaces/QuadSpi.hpp"

TEST(QuadSpiTest, AddressToVectorToAddress)
{
    EXPECT_EQ(1, hal::QuadSpi::VectorToAddress(hal::QuadSpi::AddressToVector(1, 1)));
    EXPECT_EQ(1, hal::QuadSpi::VectorToAddress(hal::QuadSpi::AddressToVector(1, 2)));
    EXPECT_EQ(1, hal::QuadSpi::VectorToAddress(hal::QuadSpi::AddressToVector(1, 3)));
    EXPECT_EQ(1, hal::QuadSpi::VectorToAddress(hal::QuadSpi::AddressToVector(1, 4)));

    EXPECT_EQ(0, hal::QuadSpi::VectorToAddress(hal::QuadSpi::AddressToVector(0x12345678, 0)));
    EXPECT_EQ(0x78, hal::QuadSpi::VectorToAddress(hal::QuadSpi::AddressToVector(0x12345678, 1)));
    EXPECT_EQ(0x5678, hal::QuadSpi::VectorToAddress(hal::QuadSpi::AddressToVector(0x12345678, 2)));
    EXPECT_EQ(0x345678, hal::QuadSpi::VectorToAddress(hal::QuadSpi::AddressToVector(0x12345678, 3)));
    EXPECT_EQ(0x12345678, hal::QuadSpi::VectorToAddress(hal::QuadSpi::AddressToVector(0x12345678, 4)));
}
