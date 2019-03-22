#ifndef HAL_COMMUNICATION_CONFIGURATOR_HPP
#define HAL_COMMUNICATION_CONFIGURATOR_HPP

namespace hal
{
    class CommunicationConfigurator
    {
    protected:
        CommunicationConfigurator() = default;
        CommunicationConfigurator(const CommunicationConfigurator& other) = delete;
        CommunicationConfigurator& operator=(const CommunicationConfigurator& other) = delete;
        ~CommunicationConfigurator() = default;

    public:
        virtual void ActivateConfiguration() = 0;
        virtual void DeactivateConfiguration() = 0;
    };
}

#endif
