#include "services/util/SmiPhyLinkMonitor.hpp"

namespace services
{
    SmiPhyLinkMonitor::SmiPhyLinkMonitor(hal::SmiBus& smi, uint8_t phyAddress, infra::Duration pollInterval)
        : phyAddress_(phyAddress)
        , phy_(smi, phyAddress_)
    {
        pollingTimer_.Start(pollInterval, [this]
            {
                PollPort();
            });
    }

    uint16_t SmiPhyLinkMonitor::PhyAddress() const
    {
        return phyAddress_;
    }

    void SmiPhyLinkMonitor::PollPort()
    {
        const auto linkState = phy_.ReadLinkState();
        if (linkState == SmiPhy::LinkState::Up && HasObserver())
            GetObserver().LinkUp(phy_.LinkSpeed());
        else if (linkState == SmiPhy::LinkState::Down && HasObserver())
            GetObserver().LinkDown();
    }
}
