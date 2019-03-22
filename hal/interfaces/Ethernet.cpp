#include "hal/interfaces/Ethernet.hpp"

namespace hal
{
    EthernetSmiObserver::EthernetSmiObserver(EthernetSmi& subject)
        : infra::SingleObserver<EthernetSmiObserver, EthernetSmi>(subject)
    {}

    EthernetMacObserver::EthernetMacObserver(EthernetMac& subject)
        : infra::SingleObserver<EthernetMacObserver, EthernetMac>(subject)
    {}
}
