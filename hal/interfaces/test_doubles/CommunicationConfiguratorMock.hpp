#ifndef HAL_COMMUNICATION_CONFIGURATOR_MOCK_HPP
#define HAL_COMMUNICATION_CONFIGURATOR_MOCK_HPP

#include "hal/interfaces/CommunicationConfigurator.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class CommunicationConfiguratorMock
        : public CommunicationConfigurator
    {
    public:
        MOCK_METHOD0(ActivateConfiguration, void());
        MOCK_METHOD0(DeactivateConfiguration, void());
    };
}

#endif
