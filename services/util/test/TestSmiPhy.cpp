#include "hal/interfaces/test_doubles/SmiBusMock.hpp"
#include "services/util/SmiPhy.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;
using namespace services;
using namespace hal;

namespace
{
    static constexpr uint8_t phyAddress = 5;

    // Bit helpers (matches register layout in SmiPhy.hpp)
    static constexpr uint16_t bcrAutoNegEnable = static_cast<uint16_t>(1u << SmiPhy::BasicControlRegister::AutoNegEnableBit);
    static constexpr uint16_t bcrSpeedSelectLsb = static_cast<uint16_t>(1u << SmiPhy::BasicControlRegister::SpeedSelectLsbBit);
    static constexpr uint16_t bcrDuplexMode = static_cast<uint16_t>(1u << SmiPhy::BasicControlRegister::DuplexModeBit);
    static constexpr uint16_t bcrReset = static_cast<uint16_t>(1u << SmiPhy::BasicControlRegister::ResetBit);

    static constexpr uint16_t bsrLinked = SmiPhy::BasicStatusRegister::LinkStatusMask;
    static constexpr uint16_t bsrAutoNegCapable = SmiPhy::BasicStatusRegister::AutoNegAbilityMask;
    static constexpr uint16_t bsrAutoNegComplete = SmiPhy::BasicStatusRegister::AutoNegCompleteMask;
    static constexpr uint16_t bsrNotPresent = SmiPhy::BasicStatusRegister::NotPresentValue;

    static constexpr uint16_t anegFullDuplex100 = static_cast<uint16_t>(1u << SmiPhy::AutoNegRegister::FullDuplex100MHzBit);
    static constexpr uint16_t anegHalfDuplex100 = static_cast<uint16_t>(1u << SmiPhy::AutoNegRegister::HalfDuplex100MHzBit);
    static constexpr uint16_t anegFullDuplex10 = static_cast<uint16_t>(1u << SmiPhy::AutoNegRegister::FullDuplex10MHzBit);
    static constexpr uint16_t anegHalfDuplex10 = static_cast<uint16_t>(1u << SmiPhy::AutoNegRegister::HalfDuplex10MHzBit);
}

class SmiPhyTest
    : public ::testing::Test
{
protected:
    StrictMock<SmiBusMock> smi;
    SmiPhy phy{ smi, phyAddress };
};

// ---- BasicControlRegister static helpers -----------------------------------

TEST(SmiPhyBasicControlRegisterTest, IsResettingReturnsTrueWhenResetBitSet)
{
    EXPECT_TRUE(SmiPhy::BasicControlRegister::IsResetting(bcrReset));
}

TEST(SmiPhyBasicControlRegisterTest, IsResettingReturnsFalseWhenResetBitClear)
{
    EXPECT_FALSE(SmiPhy::BasicControlRegister::IsResetting(0));
}

// ---- BasicStatusRegister static helpers ------------------------------------

TEST(SmiPhyBasicStatusRegisterTest, IsLinkedReturnsTrueWhenLinkBitSet)
{
    EXPECT_TRUE(SmiPhy::BasicStatusRegister::IsLinked(bsrLinked));
}

TEST(SmiPhyBasicStatusRegisterTest, IsLinkedReturnsFalseWhenLinkBitClear)
{
    EXPECT_FALSE(SmiPhy::BasicStatusRegister::IsLinked(0));
}

TEST(SmiPhyBasicStatusRegisterTest, IsRespondingReturnsFalseFor0xFFFF)
{
    EXPECT_FALSE(SmiPhy::BasicStatusRegister::IsResponding(bsrNotPresent));
}

TEST(SmiPhyBasicStatusRegisterTest, IsRespondingReturnsTrueForOtherValues)
{
    EXPECT_TRUE(SmiPhy::BasicStatusRegister::IsResponding(0));
    EXPECT_TRUE(SmiPhy::BasicStatusRegister::IsResponding(bsrLinked));
    EXPECT_TRUE(SmiPhy::BasicStatusRegister::IsResponding(0xFFFEu));
}

TEST(SmiPhyBasicStatusRegisterTest, IsAutoNegCapableReturnsTrueWhenBitSet)
{
    EXPECT_TRUE(SmiPhy::BasicStatusRegister::IsAutoNegCapable(bsrAutoNegCapable));
}

TEST(SmiPhyBasicStatusRegisterTest, IsAutoNegCapableReturnsFalseWhenBitClear)
{
    EXPECT_FALSE(SmiPhy::BasicStatusRegister::IsAutoNegCapable(0));
}

TEST(SmiPhyBasicStatusRegisterTest, IsAutoNegCompleteReturnsTrueWhenBitSet)
{
    EXPECT_TRUE(SmiPhy::BasicStatusRegister::IsAutoNegComplete(bsrAutoNegComplete));
}

TEST(SmiPhyBasicStatusRegisterTest, IsAutoNegCompleteReturnsFalseWhenBitClear)
{
    EXPECT_FALSE(SmiPhy::BasicStatusRegister::IsAutoNegComplete(0));
}

TEST(SmiPhyBasicStatusRegisterTest, HasRemoteFaultReturnsTrueWhenBitSet)
{
    EXPECT_TRUE(SmiPhy::BasicStatusRegister::HasRemoteFault(SmiPhy::BasicStatusRegister::RemoteFaultMask));
}

TEST(SmiPhyBasicStatusRegisterTest, HasRemoteFaultReturnsFalseWhenBitClear)
{
    EXPECT_FALSE(SmiPhy::BasicStatusRegister::HasRemoteFault(0));
}

// ---- ReadBcr / WriteBcr / ReadBsr ------------------------------------------

TEST_F(SmiPhyTest, ReadBcrReadsRegister0)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(0x1234u));
    EXPECT_THAT(phy.ReadBcr(), Eq(0x1234u));
}

TEST_F(SmiPhyTest, WriteBcrWritesRegister0)
{
    EXPECT_CALL(smi, Write(phyAddress, SmiPhy::BasicControlRegister::Address, 0xABCDu));
    phy.WriteBcr(0xABCDu);
}

TEST_F(SmiPhyTest, ReadBsrReadsRegister1)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(0x5678u));
    EXPECT_THAT(phy.ReadBsr(), Eq(0x5678u));
}

// ---- ReadLinkState ---------------------------------------------------------

TEST_F(SmiPhyTest, ReadLinkStatePhyNotRespondingReturnsDown)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrNotPresent));
    EXPECT_THAT(phy.ReadLinkState(), Eq(SmiPhy::LinkState::Down));
}

TEST_F(SmiPhyTest, ReadLinkStateNoLinkNoAutoNegReturnsDown)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(0));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(0));
    EXPECT_THAT(phy.ReadLinkState(), Eq(SmiPhy::LinkState::Down));
}

TEST_F(SmiPhyTest, ReadLinkStateLinkUpNoAutoNegReturnsUp)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrLinked));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(0));
    EXPECT_THAT(phy.ReadLinkState(), Eq(SmiPhy::LinkState::Up));
}

TEST_F(SmiPhyTest, ReadLinkStateLinkUpAutoNegEnabledButNotCompleteReturnsDown)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrLinked));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(bcrAutoNegEnable));
    EXPECT_THAT(phy.ReadLinkState(), Eq(SmiPhy::LinkState::Down));
}

TEST_F(SmiPhyTest, ReadLinkStateLinkUpAutoNegEnabledAndCompleteReturnsUp)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrLinked | bsrAutoNegComplete));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(bcrAutoNegEnable));
    EXPECT_THAT(phy.ReadLinkState(), Eq(SmiPhy::LinkState::Up));
}

// ---- EnableAutoNegIfCapable ------------------------------------------------

TEST_F(SmiPhyTest, EnableAutoNegIfCapableWritesBcrWhenCapable)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrAutoNegCapable));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(0));
    EXPECT_CALL(smi, Write(phyAddress, SmiPhy::BasicControlRegister::Address, bcrAutoNegEnable));
    phy.EnableAutoNegIfCapable();
}

TEST_F(SmiPhyTest, EnableAutoNegIfCapableDoesNotWriteWhenNotCapable)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(0));
    phy.EnableAutoNegIfCapable();
}

// ---- LinkSpeed (manual, autoneg disabled) ----------------------------------

TEST_F(SmiPhyTest, LinkSpeedManualFullDuplex100MHz)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicControlRegister::Address))
        .WillOnce(Return(bcrSpeedSelectLsb | bcrDuplexMode));
    EXPECT_THAT(phy.LinkSpeed(), Eq(LinkSpeed::fullDuplex100MHz));
}

TEST_F(SmiPhyTest, LinkSpeedManualHalfDuplex100MHz)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicControlRegister::Address))
        .WillOnce(Return(bcrSpeedSelectLsb));
    EXPECT_THAT(phy.LinkSpeed(), Eq(LinkSpeed::halfDuplex100MHz));
}

TEST_F(SmiPhyTest, LinkSpeedManualFullDuplex10MHz)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicControlRegister::Address))
        .WillOnce(Return(bcrDuplexMode));
    EXPECT_THAT(phy.LinkSpeed(), Eq(LinkSpeed::fullDuplex10MHz));
}

TEST_F(SmiPhyTest, LinkSpeedManualHalfDuplex10MHz)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicControlRegister::Address))
        .WillOnce(Return(0));
    EXPECT_THAT(phy.LinkSpeed(), Eq(LinkSpeed::halfDuplex10MHz));
}

// ---- LinkSpeed (autoneg enabled — negotiated from ANLPA/ANLPAR) ------------

TEST_F(SmiPhyTest, LinkSpeedNegotiatedFullDuplex100MHz)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(bcrAutoNegEnable));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::AutoNegRegister::AdvertisementAddress)).WillOnce(Return(anegFullDuplex100));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::AutoNegRegister::LinkPartnerAddress)).WillOnce(Return(anegFullDuplex100));
    EXPECT_THAT(phy.LinkSpeed(), Eq(LinkSpeed::fullDuplex100MHz));
}

TEST_F(SmiPhyTest, LinkSpeedNegotiatedHalfDuplex100MHz)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(bcrAutoNegEnable));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::AutoNegRegister::AdvertisementAddress)).WillOnce(Return(anegHalfDuplex100));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::AutoNegRegister::LinkPartnerAddress)).WillOnce(Return(anegHalfDuplex100));
    EXPECT_THAT(phy.LinkSpeed(), Eq(LinkSpeed::halfDuplex100MHz));
}

TEST_F(SmiPhyTest, LinkSpeedNegotiatedFullDuplex10MHz)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(bcrAutoNegEnable));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::AutoNegRegister::AdvertisementAddress)).WillOnce(Return(anegFullDuplex10));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::AutoNegRegister::LinkPartnerAddress)).WillOnce(Return(anegFullDuplex10));
    EXPECT_THAT(phy.LinkSpeed(), Eq(LinkSpeed::fullDuplex10MHz));
}

TEST_F(SmiPhyTest, LinkSpeedNegotiatedHalfDuplex10MHz)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(bcrAutoNegEnable));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::AutoNegRegister::AdvertisementAddress)).WillOnce(Return(anegHalfDuplex10));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::AutoNegRegister::LinkPartnerAddress)).WillOnce(Return(anegHalfDuplex10));
    EXPECT_THAT(phy.LinkSpeed(), Eq(LinkSpeed::halfDuplex10MHz));
}

TEST_F(SmiPhyTest, LinkSpeedNegotiatedPrefersHigherSpeed)
{
    // Both sides advertise all speeds — highest common wins (full 100)
    const uint16_t allSpeeds = anegFullDuplex100 | anegHalfDuplex100 | anegFullDuplex10 | anegHalfDuplex10;
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(bcrAutoNegEnable));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::AutoNegRegister::AdvertisementAddress)).WillOnce(Return(allSpeeds));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::AutoNegRegister::LinkPartnerAddress)).WillOnce(Return(allSpeeds));
    EXPECT_THAT(phy.LinkSpeed(), Eq(LinkSpeed::fullDuplex100MHz));
}

TEST_F(SmiPhyTest, LinkSpeedNegotiatedNoCommonCapabilityFallsBackToHalfDuplex10MHz)
{
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(bcrAutoNegEnable));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::AutoNegRegister::AdvertisementAddress)).WillOnce(Return(anegFullDuplex100));
    EXPECT_CALL(smi, Read(phyAddress, SmiPhy::AutoNegRegister::LinkPartnerAddress)).WillOnce(Return(anegHalfDuplex10));
    EXPECT_THAT(phy.LinkSpeed(), Eq(LinkSpeed::halfDuplex10MHz));
}
