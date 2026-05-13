#ifndef SERVICES_SMI_PHY_LINK_MONITOR_HPP
#define SERVICES_SMI_PHY_LINK_MONITOR_HPP

#include "hal/interfaces/Ethernet.hpp"
#include "hal/interfaces/SmiBus.hpp"
#include "infra/timer/Timer.hpp"
#include "services/util/SmiPhy.hpp"
#include <chrono>
#include <cstdint>

namespace services
{
    // Polls a single SMI-accessible port PHY at a fixed interval and exposes the
    // link state via the hal::EthernetSmi observer interface.
    // PhyAddress() returns the virtual PHY address (VPHY) used by the MAC-side
    // RMII connection, not the polled port PHY address.
    class SmiPhyLinkMonitor
        : public hal::EthernetSmi
    {
    public:
        SmiPhyLinkMonitor(hal::SmiBus& smi, uint8_t vphyAddress, uint8_t portPhyAddress, infra::Duration pollInterval = std::chrono::milliseconds{ 200 });

        uint16_t PhyAddress() const override;

    private:
        void PollPort();

        uint8_t vphyAddress_;
        SmiPhy phy_;
        infra::TimerRepeating pollingTimer_;
    };
}

#endif
