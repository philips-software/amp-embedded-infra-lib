#ifndef CUCUMBER_WIRE_PROTOCOL_SERVER_MOCK_HPP
#define CUCUMBER_WIRE_PROTOCOL_SERVER_MOCK_HPP

#include "gmock/gmock.h"
#include "services\cucumber\CucumberWireProtocolServer.hpp"

namespace services
{
    class CucumberWireProtocolServerMock
        : public services::CucumberWireProtocolServer
    {

    };

    class CucumberStepMock
        : public services::CucumberStep
    {
    public:
        CucumberStepMock()
            : CucumberStep("Mock Step")
        {}

        MOCK_METHOD1(Invoke, bool(infra::JsonArray& arguments));
    };

}

#endif