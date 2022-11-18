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
    if (ContainsArgument(arguments, 0) && ContainsArgument(arguments, 1))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("the WiFi network '%s' is seen within %d minutes and %d seconds")
{
    if (ContainsArgument(arguments, 0) && ContainsArgument(arguments, 1) && ContainsArgument(arguments, 2))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("the WiFi network '%s' is seen within %d minutes '%s' is seen within %d seconds")
{
    if (ContainsArgument(arguments, 0) && ContainsArgument(arguments, 1) && ContainsArgument(arguments, 2) && ContainsArgument(arguments, 3))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("the Node connects to that network")
{
    if (ContainsTableArgument(arguments, "ssid") && ContainsTableArgument(arguments, "foobar") && ContainsTableArgument(arguments, "WLAN"))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("a network is available")
{
    if (ContainsTableArgument(arguments, "field") && ContainsTableArgument(arguments, "ssid") && ContainsTableArgument(arguments, "key"))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("sentence with '%s' and %d digit")
{
    if (ContainsArgument(arguments, 0) && ContainsArgument(arguments, 1))
        if (ContainsTableArgument(arguments, "field") && ContainsTableArgument(arguments, "ssid") && ContainsTableArgument(arguments, "key"))
            Success();
        else
            Error("Incorrect Arguments");
    else
        Error("Incorrect Arguments");
}

GIVEN("nothing happens for %d seconds")
{
    auto timeout = GetUIntegerArgument(arguments, 0);

    if (timeout)
        Context().TimeoutTimer().Start(std::chrono::seconds(*timeout), [this]()
            { Success(); });
}
