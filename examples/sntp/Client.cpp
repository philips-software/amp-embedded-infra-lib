#include "hal/generic/TimerServiceGeneric.hpp"
#include "services/network/SntpClient.hpp"
#include "services/network_instantiations/NetworkAdapter.hpp"
#include "services/tracer/TracerOnIoOutputInfrastructure.hpp"
#include "services/util/TimeWithLocalization.hpp"

struct TimeWithSynchronization
    : public services::SntpResultObserver
    , public services::NameResolverResult
{
    TimeWithSynchronization(services::DatagramFactory& datagramFactory, services::NameResolver& nameResolver, services::Tracer& tracer)
        : nameResolver(nameResolver)
        , sntpClient(datagramFactory, timeWithLocalization)
        , tracer(tracer)
    {
        services::SntpResultObserver::Attach(sntpClient);
        nameResolver.Lookup(*this);
    }

    // From services::SntpResultObserver
    virtual void TimeAvailable(infra::Duration roundTripDelay, infra::Duration localClockOffset) override
    {
        timeWithLocalization.Utc().Shift(localClockOffset - timeWithLocalization.Utc().GetCurrentShift());

        tracer.Trace() << "Time synchronized; round trip delay was: " << roundTripDelay
                       << " local offset was: " << localClockOffset
                       << " new time set to: " << timeWithLocalization.Utc().Now();
    }

    virtual void TimeUnavailable() override
    {
        tracer.Trace() << "Time failed to synchronize";
    }

    virtual void KissOfDeath(services::SntpResultObserver::KissCode reason) override
    {}

    // From services::NameResolverResult
    virtual infra::BoundedConstString Hostname() const override
    {
        return "pool.ntp.org";
    }

    virtual void NameLookupDone(services::IPAddress address, infra::TimePoint validUntil) override
    {
        tracer.Trace() << "Name lookup done; found address " << address;
        sntpClient.RequestTime(address.Get<services::IPv4Address>());
    }

    virtual void NameLookupFailed() override
    {
        tracer.Trace() << "Name lookup failed";
    }

    services::NameResolver& nameResolver;
    services::TimeWithLocalization timeWithLocalization;
    services::SntpClient sntpClient;
    services::Tracer& tracer;
};

int main(int argc, const char* argv[], const char* env[])
{
    static hal::TimerServiceGeneric timerService;
    static main_::TracerOnIoOutputInfrastructure tracer;
    static main_::NetworkAdapter network;
    static TimeWithSynchronization time(network.DatagramFactory(), network.NameResolver(), tracer.tracer);

    network.Run();

    return 0;
}
