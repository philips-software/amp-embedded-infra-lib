#ifndef SERVICES_CONNECTION_STATUS_HPP
#define SERVICES_CONNECTION_STATUS_HPP

namespace services
{
    class ConnectionStatus
    {
    public:
        virtual bool PendingSend() const = 0;
    };
}

#endif
