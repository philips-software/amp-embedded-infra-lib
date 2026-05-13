#include "services/util/SmiPhyLinkMonitor.hpp"

namespace services
{
    SmiPhyLinkMonitor::SmiPhyLinkMonitor(hal::SmiBus& smi, uint8_t vphyAddress, uint8_t portPhyAddress, infra::Duration pollInterval)
        : vphyAddress_(vphyAddress)
        , phy_(smi, portPhyAddress)
    {
        pollingTimer_.Start(pollInterval, [this]
            {
                PollPort();
            });
    }

    uint16_t SmiPhyLinkMonitor::PhyAddress() const
    {
        return vphyAddress_;
    }

    void SmiPhyLinkMonitor::PollPort()
    {
        const auto linkState = phy_.ReadLinkState();
        if (linkState == SmiPhy::LinkState::Up && HasObserver())
            GetObserver().LinkUp(hal::LinkSpeed::fullDuplex100MHz);
        else if (linkState == SmiPhy::LinkState::Down && HasObserver())
            GetObserver().LinkDown();
    }
}
