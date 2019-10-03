#ifndef SERVICES_SNTP_CLIENT_HPP
#define SERVICES_SNTP_CLIENT_HPP

#include "infra/util/Endian.hpp"
#include "infra/util/Observer.hpp"
#include "services/network/Datagram.hpp"
#include "services/network/Multicast.hpp"
#include "services/util/TimeWithLocalization.hpp"

namespace services
{
    class SntpClient;

    class SntpResultObserver
        : public infra::SingleObserver<SntpResultObserver, SntpClient>
    {
    protected:
        using infra::SingleObserver<SntpResultObserver, SntpClient>::SingleObserver;

    public:
        enum class KissCode : uint8_t
        {
            deny,
            restrict,
            rateExceeded
        };

        virtual void TimeAvailable(infra::Duration roundTripDelay, infra::Duration localClockOffset) = 0;
        virtual void TimeUnavailable() = 0;
        virtual void KissOfDeath(KissCode reason) = 0;
    };

    class SntpClient
        : public DatagramExchangeObserver
        , public infra::Subject<SntpResultObserver>
    {
    public:
        SntpClient(services::DatagramFactory& factory, services::TimeWithLocalization& timeWithLocalization);
        ~SntpClient() = default;

        void RequestTime(const services::IPv4Address& address);

        // From DatagramExchangeObserver
        virtual void DataReceived(infra::StreamReaderWithRewinding& reader, services::UdpSocket from) override;
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

    private:
        enum class NtpLeapIndicator : uint8_t
        {
            noWarning,
            lastMinuteHas61Seconds,
            lastMinuteHas59Seconds,
            alarmConditionClockNotSynchronized
        };

        enum class NtpMode : uint8_t
        {
            reserved,
            symmetricActive,
            symmetricPassive,
            client,
            server,
            broadcast
        };

        using NtpHeader = uint8_t;

        struct NtpTimestamp
        {
            infra::BigEndian<uint32_t> seconds;
            infra::BigEndian<uint32_t> fraction;

            infra::Duration Convert();
        };

        struct NtpTimestamps
        {
            NtpTimestamp reference;
            NtpTimestamp originate;
            NtpTimestamp receive;
            NtpTimestamp transmit;
        };

        struct NtpMessage
        {
            NtpHeader header;
            uint8_t stratum;
            int8_t poll;
            int8_t precision;
            infra::BigEndian<uint32_t> rootDelay;
            infra::BigEndian<uint32_t> rootDispersion;
            infra::BigEndian<uint32_t> referenceIdentifier;
            NtpTimestamps timestamps;

            bool Valid(infra::Duration& requestTime);
            NtpLeapIndicator LeapIndicator();
            uint8_t Version();
            NtpMode Mode();
        };

    private:
        void NotifyTimeUnavailable();
        void NotifyKissOfDeath(NtpMessage& message);
        void NotifyOffsetAndDelay(NtpMessage& message);

        static NtpHeader CreateNtpHeader(NtpLeapIndicator leapIndicator, uint8_t versionNumber, NtpMode mode);
        static NtpTimestamp Convert(infra::Duration time);

    private:
        services::DatagramFactory& factory;
        services::TimeWithLocalization& timeWithLocalization;
        infra::SharedPtr<services::DatagramExchange> datagramExchange;
        infra::Duration originRequestTime{};
    };
}

#endif
