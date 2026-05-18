#include "services/util/SmiPhyLinkMonitor.hpp"
#include "infra/util/ReallyAssert.hpp"

namespace services
{
    SmiPhyLinkMonitor::SmiPhyLinkMonitor(hal::SmiBus& smi, uint8_t phyAddress, infra::Duration pollInterval)
        : phyAddress(phyAddress)
        , phy(smi, phyAddress)
    {
        really_assert(pollInterval > infra::Duration{});
        pollingTimer.Start(pollInterval, [this]
            {
                PollPort();
            });
    }

    uint16_t SmiPhyLinkMonitor::PhyAddress() const
    {
        return phyAddress;
    }

    void SmiPhyLinkMonitor::PollPort()
    {
        const auto linkState = phy.ReadLinkState();
        if (linkState == lastReportedState)
            return;

        lastReportedState = linkState;
        if (!HasObserver())
            return;

        if (linkState == SmiPhy::LinkState::Up)
            GetObserver().LinkUp(phy.LinkSpeed());
        else
            GetObserver().LinkDown();
    }
}
