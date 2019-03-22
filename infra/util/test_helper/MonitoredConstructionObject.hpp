#ifndef INFRA_MONITORED_CONSTRUCTION_OBJECT_HPP
#define INFRA_MONITORED_CONSTRUCTION_OBJECT_HPP

#include "gmock/gmock.h"

namespace infra
{
    class ConstructionMonitorMock
    {
    public:
        MOCK_METHOD1(Construct, void(void* object));
        MOCK_METHOD1(Destruct, void(void* object));
    };

    class MonitoredConstructionObject
    {
    public:
        MonitoredConstructionObject(ConstructionMonitorMock& mock)
            : mock(mock)
        {
            mock.Construct(this);
        }

        ~MonitoredConstructionObject()
        {
            mock.Destruct(this);
        }

    private:
        ConstructionMonitorMock& mock;
    };
}

#endif
