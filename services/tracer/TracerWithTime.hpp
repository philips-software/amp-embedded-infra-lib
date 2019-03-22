#ifndef SERVICES_TRACER_WITH_TIME_HPP
#define SERVICES_TRACER_WITH_TIME_HPP

#include "infra/timer/TimerService.hpp"
#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracerWithTime                                                                                            //TICS !OOP#013
        : public Tracer
    {
    public:
        TracerWithTime(infra::TextOutputStream& stream, const infra::TimerService& timerService);

    protected:
        virtual void InsertHeader() override;

    private:
        const infra::TimerService& timerService;
    };
}

#endif
