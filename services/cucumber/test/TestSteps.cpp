#include "services/cucumber/CucumberStepMacro.hpp"

GIVEN("a duplicate step")
{
    Success();
}

GIVEN("a duplicate step")
{
    Success();
}

GIVEN("a step")
{
    Success();
}

GIVEN("the WiFi network '%s' is seen within %d minutes")
{
    if (ContainsStringArgument(0) && ContainsStringArgument(1))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("the WiFi network '%s' is seen within %d minutes and %d seconds")
{
    if (ContainsStringArgument(0) && ContainsStringArgument(1) && ContainsStringArgument(2))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("the WiFi network '%s' is seen within %d minutes '%s' is seen within %d seconds")
{
    if (ContainsStringArgument(0) && ContainsStringArgument(1) && ContainsStringArgument(2) && ContainsStringArgument(3))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("the Node connects to that network")
{
    if (ContainsTableArgument("ssid") && ContainsTableArgument("foobar") && ContainsTableArgument("WLAN"))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("a network is available")
{
    if (ContainsTableArgument("field") && ContainsTableArgument("ssid") && ContainsTableArgument("key"))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("sentence with '%s' and %d digit")
{
    if (ContainsStringArgument(0) && ContainsStringArgument(1))
        if (ContainsTableArgument("field") && ContainsTableArgument("ssid") && ContainsTableArgument("key"))
            Success();
        else
            Error("Incorrect Arguments");
    else
        Error("Incorrect Arguments");
}

GIVEN("nothing happens for %d seconds")
{
    auto timeout = GetUIntegerArgument(0);

    if (timeout)
        Context().TimeoutTimer().Start(std::chrono::seconds(*timeout), [this]()
            { Success(); });
}
