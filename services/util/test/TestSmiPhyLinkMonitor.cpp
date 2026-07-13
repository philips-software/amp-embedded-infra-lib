#include "hal/interfaces/test_doubles/EthernetSmiObserverMock.hpp"
#include "hal/interfaces/test_doubles/SmiBusMock.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/util/SmiPhyLinkMonitor.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;
using namespace services;
using namespace hal;

class SmiPhyLinkMonitorTest
    : public ::testing::Test
    , public infra::ClockFixture
{
protected:
    static constexpr uint8_t portPhyAddress = 9;
    static constexpr uint16_t bsrLinkUp = 0x0004u;          // bit 2: link status
    static constexpr uint16_t bsrAutoNegComplete = 0x0020u; // bit 5: autoneg complete
    static constexpr uint16_t bsrNotPresent = 0xFFFFu;
    static constexpr uint16_t bcrFullDuplex100MHz = 0x2100u; // bit 8 (duplex) + bit 13 (speed)

    StrictMock<SmiBusMock> smi;
    SmiPhyLinkMonitor monitor{ smi, portPhyAddress };
};

// ---- PhyAddress ------------------------------------------------------------

TEST_F(SmiPhyLinkMonitorTest, PhyAddressReturnsPortPhyAddress)
{
    ASSERT_THAT(monitor.PhyAddress(), Eq(portPhyAddress));
}

// ---- Polling timing --------------------------------------------------------

TEST_F(SmiPhyLinkMonitorTest, NoPollBeforeFirstInterval)
{
    StrictMock<EthernetSmiObserverMock> observer{ monitor };

    ForwardTime(std::chrono::milliseconds{ 199 });
}

TEST_F(SmiPhyLinkMonitorTest, PollOccursAtFirstInterval)
{
    StrictMock<EthernetSmiObserverMock> observer{ monitor };

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(0));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(0));

    ForwardTime(std::chrono::milliseconds{ 200 });
}

TEST_F(SmiPhyLinkMonitorTest, PollsRepeatinglyAtInterval)
{
    StrictMock<EthernetSmiObserverMock> observer{ monitor };

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).Times(3).WillRepeatedly(Return(0));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).Times(3).WillRepeatedly(Return(0));

    ForwardTime(std::chrono::milliseconds{ 600 });
}

// ---- Custom poll interval --------------------------------------------------

TEST_F(SmiPhyLinkMonitorTest, CustomPollIntervalIsRespected)
{
    SmiPhyLinkMonitor fastMonitor{ smi, portPhyAddress, std::chrono::milliseconds{ 50 } };
    StrictMock<EthernetSmiObserverMock> observer{ fastMonitor };

    // fastMonitor polls 4 times (50, 100, 150, 200ms) and the fixture monitor polls once (200ms)
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).Times(5).WillRepeatedly(Return(0));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).Times(5).WillRepeatedly(Return(0));

    ForwardTime(std::chrono::milliseconds{ 200 });
}

// ---- Link down (initial state) ---------------------------------------------

TEST_F(SmiPhyLinkMonitorTest, PhyDownInitialStateNoCallback)
{
    StrictMock<EthernetSmiObserverMock> observer{ monitor };

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(0));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(0));

    ForwardTime(std::chrono::milliseconds{ 200 });
}

// ---- Link up ---------------------------------------------------------------

TEST_F(SmiPhyLinkMonitorTest, PhyUpCallsLinkUp)
{
    StrictMock<EthernetSmiObserverMock> observer{ monitor };

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrLinkUp));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).Times(2).WillRepeatedly(Return(bcrFullDuplex100MHz));
    EXPECT_CALL(observer, LinkUp(LinkSpeed::fullDuplex100MHz));

    ForwardTime(std::chrono::milliseconds{ 200 });
}

TEST_F(SmiPhyLinkMonitorTest, NoDuplicateLinkUpCallback)
{
    StrictMock<EthernetSmiObserverMock> observer{ monitor };

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrLinkUp));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).Times(2).WillRepeatedly(Return(bcrFullDuplex100MHz));
    EXPECT_CALL(observer, LinkUp(LinkSpeed::fullDuplex100MHz));
    ForwardTime(std::chrono::milliseconds{ 200 });

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrLinkUp));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(bcrFullDuplex100MHz));
    ForwardTime(std::chrono::milliseconds{ 200 });
}

// ---- Link down after up ----------------------------------------------------

TEST_F(SmiPhyLinkMonitorTest, LinkUpThenDownCallsLinkDown)
{
    StrictMock<EthernetSmiObserverMock> observer{ monitor };

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrLinkUp));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).Times(2).WillRepeatedly(Return(bcrFullDuplex100MHz));
    EXPECT_CALL(observer, LinkUp(LinkSpeed::fullDuplex100MHz));
    ForwardTime(std::chrono::milliseconds{ 200 });

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(0));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(0));
    EXPECT_CALL(observer, LinkDown());
    ForwardTime(std::chrono::milliseconds{ 200 });
}

TEST_F(SmiPhyLinkMonitorTest, NoDuplicateLinkDownCallback)
{
    StrictMock<EthernetSmiObserverMock> observer{ monitor };

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrLinkUp));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).Times(2).WillRepeatedly(Return(bcrFullDuplex100MHz));
    EXPECT_CALL(observer, LinkUp(LinkSpeed::fullDuplex100MHz));
    ForwardTime(std::chrono::milliseconds{ 200 });

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(0));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(0));
    EXPECT_CALL(observer, LinkDown());
    ForwardTime(std::chrono::milliseconds{ 200 });

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(0));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(0));
    ForwardTime(std::chrono::milliseconds{ 200 });
}

TEST_F(SmiPhyLinkMonitorTest, LinkUpDownUpReportsAllTransitions)
{
    StrictMock<EthernetSmiObserverMock> observer{ monitor };

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrLinkUp));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).Times(2).WillRepeatedly(Return(bcrFullDuplex100MHz));
    EXPECT_CALL(observer, LinkUp(LinkSpeed::fullDuplex100MHz));
    ForwardTime(std::chrono::milliseconds{ 200 });

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(0));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(0));
    EXPECT_CALL(observer, LinkDown());
    ForwardTime(std::chrono::milliseconds{ 200 });

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrLinkUp));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).Times(2).WillRepeatedly(Return(bcrFullDuplex100MHz));
    EXPECT_CALL(observer, LinkUp(LinkSpeed::fullDuplex100MHz));
    ForwardTime(std::chrono::milliseconds{ 200 });
}

// ---- PHY not present -------------------------------------------------------

TEST_F(SmiPhyLinkMonitorTest, PhyNotRespondingNoCallback)
{
    StrictMock<EthernetSmiObserverMock> observer{ monitor };

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrNotPresent));

    ForwardTime(std::chrono::milliseconds{ 200 });
}

TEST_F(SmiPhyLinkMonitorTest, PhyDisappearsAfterLinkUpCallsLinkDown)
{
    StrictMock<EthernetSmiObserverMock> observer{ monitor };

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrLinkUp));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).Times(2).WillRepeatedly(Return(bcrFullDuplex100MHz));
    EXPECT_CALL(observer, LinkUp(LinkSpeed::fullDuplex100MHz));
    ForwardTime(std::chrono::milliseconds{ 200 });

    // PHY not responding: 0xFFFF → early return, no BCR read
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrNotPresent));
    EXPECT_CALL(observer, LinkDown());
    ForwardTime(std::chrono::milliseconds{ 200 });
}

// ---- No observer -----------------------------------------------------------

TEST_F(SmiPhyLinkMonitorTest, LinkUpWithNoObserverDoesNotCrash)
{
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrLinkUp));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(0));

    ForwardTime(std::chrono::milliseconds{ 200 });
}

TEST_F(SmiPhyLinkMonitorTest, LinkDownWithNoObserverDoesNotCrash)
{
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(bsrLinkUp));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(0));
    ForwardTime(std::chrono::milliseconds{ 200 });

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(0));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(0));
    ForwardTime(std::chrono::milliseconds{ 200 });
}

// ---- Port PHY address is used for polling ----------------------------------

TEST_F(SmiPhyLinkMonitorTest, PortPhyAddressIsUsedForPolling)
{
    static constexpr uint8_t otherAddress = 3;
    StrictMock<EthernetSmiObserverMock> observer{ monitor };
    SmiPhyLinkMonitor otherMonitor{ smi, otherAddress };

    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(0));
    EXPECT_CALL(smi, Read(portPhyAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(0));
    EXPECT_CALL(smi, Read(otherAddress, SmiPhy::BasicStatusRegister::Address)).WillOnce(Return(0));
    EXPECT_CALL(smi, Read(otherAddress, SmiPhy::BasicControlRegister::Address)).WillOnce(Return(0));

    ForwardTime(std::chrono::milliseconds{ 200 });
}
