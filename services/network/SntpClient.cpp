#include "services/network/SntpClient.hpp"
#include "infra/util/EnumCast.hpp"

namespace
{
    const uint16_t ntpPort = 123;
    const uint8_t ntpRequestVersion = 4;
    const infra::Duration ntpEpochOffset{ std::chrono::seconds((70 * 365 + 17) * 86400u) };
    const uint32_t ntpFractionFactor = 4295; // 2^32 / 10^6
}

namespace services
{
    infra::Duration SntpClient::NtpTimestamp::Convert()
    {
        return std::chrono::seconds(seconds) + std::chrono::microseconds(fraction / ntpFractionFactor) - ntpEpochOffset;
    }

    bool SntpClient::NtpMessage::Valid(infra::Duration& requestTime)
    {
        if (LeapIndicator() == NtpLeapIndicator::alarmConditionClockNotSynchronized)
            return false;

        if (Version() != ntpRequestVersion)
            return false;

        if (Mode() != NtpMode::server)
            return false;

        if (stratum > 15)
            return false;

        if (timestamps.transmit.seconds == uint32_t{0})
            return false;

        if (std::chrono::duration_cast<std::chrono::seconds>(timestamps.originate.Convert()) != std::chrono::duration_cast<std::chrono::seconds>(requestTime))
            return false;

        return true;
    }

    SntpClient::NtpLeapIndicator SntpClient::NtpMessage::LeapIndicator()
    {
        return static_cast<NtpLeapIndicator>((header >> 6) & 0b11);
    }

    uint8_t SntpClient::NtpMessage::Version()
    {
        return (header >> 3) & 0b111;
    }

    SntpClient::NtpMode SntpClient::NtpMessage::Mode()
    {
        return static_cast<NtpMode>(header & 0b111);
    }

    SntpClient::SntpClient(services::DatagramFactory& factory, services::TimeWithLocalization& timeWithLocalization)
        : factory(factory)
        , timeWithLocalization(timeWithLocalization)
    {}

    void SntpClient::RequestTime(const services::IPv4Address& address)
    {
        datagramExchange = factory.Connect(*this, Udpv4Socket{ address, ntpPort });
        datagramExchange->RequestSendStream(sizeof(NtpMessage));
    }

    void SntpClient::DataReceived(infra::StreamReaderWithRewinding& reader, services::UdpSocket from)
    {
        if (reader.Empty())
        {
            NotifyTimeUnavailable();
            return;
        }

        infra::DataInputStream::WithErrorPolicy stream(reader, infra::softFail);
        NtpMessage message{};
        stream >> message;

        if (!stream.Failed() && message.Valid(originRequestTime))
        {
            if (message.stratum == 0)
                NotifyKissOfDeath(message);
            else
                NotifyOffsetAndDelay(message);
        }
        else
            NotifyTimeUnavailable();
    }

    void SntpClient::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        NtpMessage message{};

        originRequestTime = timeWithLocalization.Utc().Now().time_since_epoch();
        message.header = CreateNtpHeader(NtpLeapIndicator::noWarning, ntpRequestVersion, NtpMode::client);
        message.timestamps.transmit = Convert(originRequestTime);

        stream << message;
    }
    
    void SntpClient::NotifyTimeUnavailable()
    {
        NotifyObservers([](SntpResultObserver& observer) { observer.TimeUnavailable(); });
    }

    void SntpClient::NotifyKissOfDeath(NtpMessage& message)
    {
        auto kissCode = infra::ByteRangeAsString(infra::MakeByteRange(message.referenceIdentifier));

        if (kissCode == "DENY")
            NotifyObservers([](SntpResultObserver& observer) { observer.KissOfDeath(SntpResultObserver::KissCode::deny); });
        else if (kissCode == "RSTR")
            NotifyObservers([](SntpResultObserver& observer) { observer.KissOfDeath(SntpResultObserver::KissCode::restrict); });
        else if (kissCode == "RATE")
            NotifyObservers([](SntpResultObserver& observer) { observer.KissOfDeath(SntpResultObserver::KissCode::rateExceeded); });
        else
            NotifyTimeUnavailable();
    }

    void SntpClient::NotifyOffsetAndDelay(NtpMessage& message)
    {
        // 'Algorithm' according to section 5 of RFC5905
        auto t1 = message.timestamps.originate.Convert();
        auto t2 = message.timestamps.receive.Convert();
        auto t3 = message.timestamps.transmit.Convert();
        auto t4 = timeWithLocalization.Utc().Now().time_since_epoch();

        auto roundTripDelay = (t4 - t1) - (t3 - t2);
        auto localClockOffset = ((t2 - t1) + (t3 - t4)) / 2;

        NotifyObservers([roundTripDelay, localClockOffset](SntpResultObserver& observer) { observer.TimeAvailable(roundTripDelay, localClockOffset); });
    }

    SntpClient::NtpHeader SntpClient::CreateNtpHeader(NtpLeapIndicator leapIndicator, uint8_t versionNumber, NtpMode mode)
    {
        uint8_t result = 0;

        really_assert(versionNumber == 3 || versionNumber == 4);

        result |= infra::enum_cast(leapIndicator) << 6;
        result |= versionNumber << 3;
        result |= infra::enum_cast(mode);

        return result;
    }

    SntpClient::NtpTimestamp SntpClient::Convert(infra::Duration time)
    {
        NtpTimestamp ntpTime = { 0, 0 };
        ntpTime.seconds = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(time + ntpEpochOffset).count());
        ntpTime.fraction = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::microseconds>(time).count() * ntpFractionFactor);

        return ntpTime;
    }
}
