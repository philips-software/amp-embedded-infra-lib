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
    class SmiPhyLinkMonitor
        : public hal::EthernetSmi
    {
    public:
        SmiPhyLinkMonitor(hal::SmiBus& smi, uint8_t phyAddress, infra::Duration pollInterval = std::chrono::milliseconds{ 200 });

        uint16_t PhyAddress() const override;

    private:
        void PollPort();

        uint8_t phyAddress_;
        SmiPhy phy_;
        infra::TimerRepeating pollingTimer_;
    };
}

#endif
