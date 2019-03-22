#include "services/network/LlmnrResponder.hpp"

namespace services
{
    namespace
    {
        template<std::size_t Size>
        struct LlmnrString
        {
            infra::BoundedString::WithStorage<Size> value;
        };

        template<std::size_t Size>
        infra::DataInputStream& operator>>(infra::DataInputStream& stream, LlmnrString<Size>& string)
        {
            uint8_t length;
            stream >> length;
            string.value.resize(std::min<uint32_t>(length, string.value.max_size()));
            stream >> infra::StringAsByteRange(string.value);
            uint8_t null;
            stream >> null;

            stream.ErrorPolicy().ReportResult(null == 0 && length <= string.value.max_size());
            return stream;
        }

        std::size_t LlmnrStringSize(infra::BoundedConstString value)
        {
            // A string is prepended with one byte length and appended with a terminating 0
            return value.size() + 2;
        }
    }

    const services::IPv4Address LlmnrResponder::llmnpMulticastAddress{ 224, 0, 0, 252 };

    LlmnrResponder::LlmnrResponder(services::DatagramFactory& factory, services::Multicast& multicast, services::IPv4Info& ipv4Info, infra::BoundedConstString name)
        : datagramExchange(factory.Listen(*this, llmnpPort, IPVersions::ipv4))
        , multicast(multicast)
        , ipv4Info(ipv4Info)
        , name(name)
    {
        multicast.JoinMulticastGroup(llmnpMulticastAddress);
    }

    LlmnrResponder::~LlmnrResponder()
    {
        multicast.LeaveMulticastGroup(llmnpMulticastAddress);
    }

    void LlmnrResponder::DataReceived(infra::StreamReader& reader, services::UdpSocket from)
    {
        if (replying)
            return;

        infra::DataInputStream::WithErrorPolicy stream(reader, infra::softFail);
        Header header;
        stream >> header;
        LlmnrString<32> queriedName;
        stream >> queriedName;
        Footer footer;
        stream >> footer;

        if (stream.Failed())
            return;

        if (!PacketValid(header, footer))
            return;

        if (queriedName.value != name)
            return;

        replying = true;
        id = header.id;
        datagramExchange->RequestSendStream(sizeof(Header) + LlmnrStringSize(name) + sizeof(Footer) + sizeof(Answer), from);
    }

    void LlmnrResponder::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        replying = false;

        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        Header header{ id, Header::flagsResponse, 1, 1 };
        stream << header;
        stream << static_cast<uint8_t>(name.size()) << infra::StringAsByteRange(name) << '\0';
        Footer footer;
        stream << footer;
        Answer answer{ Answer::nameIsPointer | sizeof(Header), Footer::typeA, Footer::classIn, 0, Answer::defaultTtl, sizeof(IPv4Address), ipv4Info.GetIPv4Address() };
        stream << infra::MakeByteRange(answer);
    }

    bool LlmnrResponder::PacketValid(const Header& header, const Footer& footer) const
    {
        return header.flags == Header::flagsQuery && header.questionsCount == static_cast<uint16_t>(1) && footer.type == Footer::typeA && footer.class_ == Footer::classIn;
    }
}
