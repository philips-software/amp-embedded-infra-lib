#ifndef SERVICES_PENDING_SEND_HPP
#define SERVICES_PENDING_SEND_HPP

namespace services
{
    class PendingSendLightweightIp
    {
    public:
        virtual bool PendingSend() const = 0;
    };
}

#endif
