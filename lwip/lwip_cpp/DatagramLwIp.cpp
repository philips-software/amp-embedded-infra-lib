#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "lwip/lwip_cpp/DatagramLwIp.hpp"

namespace services
{
    namespace
    {
        ip_addr_t Convert(IPv4Address address)
        {
            ip_addr_t result IPADDR4_INIT(0);
            IP4_ADDR(&result.u_addr.ip4, address[0], address[1], address[2], address[3]);
            return result;
        }

        ip_addr_t Convert(IPv6Address address)
        {
            ip_addr_t result IPADDR6_INIT(0, 0, 0, 0);
            IP6_ADDR(&result.u_addr.ip6, PP_HTONL(address[1] + (static_cast<uint32_t>(address[0]) << 16)), PP_HTONL(address[3] + (static_cast<uint32_t>(address[2]) << 16)), PP_HTONL(address[5] + (static_cast<uint32_t>(address[4]) << 16)), PP_HTONL(address[7] + (static_cast<uint32_t>(address[6]) << 16)));
            return result;
        }

        std::pair<ip_addr_t, uint16_t> Convert(UdpSocket socket)
        {
            if (socket.Is<Udpv4Socket>())
                return std::make_pair(Convert(socket.Get<Udpv4Socket>().first), socket.Get<Udpv4Socket>().second);
            else
                return std::make_pair(Convert(socket.Get<Udpv6Socket>().first), socket.Get<Udpv6Socket>().second);
        }
    }

    DatagramExchangeLwIP::DatagramExchangeLwIP(DatagramExchangeObserver& observer)
        : state(infra::InPlaceType<StateIdle>(), *this)
    {
        observer.Attach(*this);
    }

    DatagramExchangeLwIP::~DatagramExchangeLwIP()
    {
        GetObserver().Detach();
        udp_remove(control);
    }

    void DatagramExchangeLwIP::Listen(uint16_t port, IPVersions versions)
    {
        control = CreateUdpPcb(versions);
        assert(control != nullptr);
        ip_set_option(control, SOF_BROADCAST);
        err_t result = udp_bind(control, IpAddrAny(versions), port);
        assert(result == ERR_OK);

        udp_recv(control, &DatagramExchangeLwIP::StaticRecv, this);
    }

    void DatagramExchangeLwIP::Listen(IPVersions versions)
    {
        control = CreateUdpPcb(versions);
        assert(control != nullptr);
        ip_set_option(control, SOF_BROADCAST);
        err_t result = udp_bind(control, IpAddrAny(versions), 0);
        assert(result == ERR_OK);

        udp_recv(control, &DatagramExchangeLwIP::StaticRecv, this);
    }

    void DatagramExchangeLwIP::Connect(UdpSocket remote)
    {
        if (remote.Is<Udpv4Socket>())
        {
            control = CreateUdpPcb(IPVersions::ipv4);
            assert(control != nullptr);
            IPv4Address ipv4Address = remote.Get<Udpv4Socket>().first;
            ip_addr_t ipAddress IPADDR4_INIT(0);
            IP4_ADDR(&ipAddress.u_addr.ip4, ipv4Address[0], ipv4Address[1], ipv4Address[2], ipv4Address[3]);
            err_t result = udp_connect(control, &ipAddress, remote.Get<Udpv4Socket>().second);
            assert(result == ERR_OK);
        }
        else
        {
            control = CreateUdpPcb(IPVersions::ipv6);
            assert(control != nullptr);
            IPv6Address ipv6Address = remote.Get<Udpv6Socket>().first;
            ip_addr_t ipAddress IPADDR6_INIT(0, 0, 0, 0);
            IP6_ADDR(&ipAddress.u_addr.ip6, PP_HTONL(ipv6Address[1] + (static_cast<uint32_t>(ipv6Address[0]) << 16)), PP_HTONL(ipv6Address[3] + (static_cast<uint32_t>(ipv6Address[2]) << 16)), PP_HTONL(ipv6Address[5] + (static_cast<uint32_t>(ipv6Address[4]) << 16)), PP_HTONL(ipv6Address[7] + (static_cast<uint32_t>(ipv6Address[6]) << 16)));
            err_t result = udp_connect(control, &ipAddress, remote.Get<Udpv6Socket>().second);
            assert(result == ERR_OK);
        }

        udp_recv(control, &DatagramExchangeLwIP::StaticRecv, this);
    }

    void DatagramExchangeLwIP::Connect(uint16_t localPort, UdpSocket remote)
    {
        if (remote.Is<Udpv4Socket>())
        {
            control = CreateUdpPcb(IPVersions::ipv4);
            assert(control != nullptr);
            ip_set_option(control, SOF_BROADCAST);
            IPv4Address ipv4Address = remote.Get<Udpv4Socket>().first;
            ip_addr_t ipAddress IPADDR4_INIT(0);
            IP4_ADDR(&ipAddress.u_addr.ip4, ipv4Address[0], ipv4Address[1], ipv4Address[2], ipv4Address[3]);
            err_t result = udp_connect(control, &ipAddress, remote.Get<Udpv4Socket>().second);
            assert(result == ERR_OK);
#ifdef ESP_PLATFORM
            result = udp_bind(control, IP_ADDR_ANY, localPort);
#else
            result = udp_bind(control, IP4_ADDR_ANY, localPort);
#endif
            assert(result == ERR_OK);
        }
        else
        {
            control = CreateUdpPcb(IPVersions::ipv6);
            assert(control != nullptr);
            IPv6Address ipv6Address = remote.Get<Udpv6Socket>().first;
            ip_addr_t ipAddress IPADDR6_INIT(0, 0, 0, 0);
            IP6_ADDR(&ipAddress.u_addr.ip6, PP_HTONL(ipv6Address[1] + (static_cast<uint32_t>(ipv6Address[0]) << 16)), PP_HTONL(ipv6Address[3] + (static_cast<uint32_t>(ipv6Address[2]) << 16)), PP_HTONL(ipv6Address[5] + (static_cast<uint32_t>(ipv6Address[4]) << 16)), PP_HTONL(ipv6Address[7] + (static_cast<uint32_t>(ipv6Address[6]) << 16)));
            err_t result = udp_connect(control, &ipAddress, remote.Get<Udpv6Socket>().second);
            assert(result == ERR_OK);
            result = udp_bind(control, IP6_ADDR_ANY, localPort);
            assert(result == ERR_OK);
        }

        udp_recv(control, &DatagramExchangeLwIP::StaticRecv, this);
    }

    void DatagramExchangeLwIP::RequestSendStream(std::size_t sendSize)
    {
        state->RequestSendStream(sendSize);
    }

    void DatagramExchangeLwIP::RequestSendStream(std::size_t sendSize, UdpSocket remote)
    {
        state->RequestSendStream(sendSize, remote);
    }

    udp_pcb* DatagramExchangeLwIP::CreateUdpPcb(IPVersions versions) const
    {
        switch (versions)
        {
            case IPVersions::ipv4:
                return udp_new_ip_type(IPADDR_TYPE_V4);
            case IPVersions::ipv6:
                return udp_new_ip_type(IPADDR_TYPE_V6);
            case IPVersions::both:
                return udp_new_ip_type(IPADDR_TYPE_ANY);
            default:
                std::abort();
        }
    }

    const ip_addr_t* DatagramExchangeLwIP::IpAddrAny(IPVersions versions) const
    {
        switch (versions)
        {
            case IPVersions::ipv4:
#ifdef ESP_PLATFORM
				return IP_ADDR_ANY;
#else
                return IP4_ADDR_ANY;
#endif
            case IPVersions::ipv6:
                return IP6_ADDR_ANY;
            case IPVersions::both:
                return IP_ADDR_ANY;
            default:
                std::abort();
        }
    }

    void DatagramExchangeLwIP::StaticRecv(void* arg, udp_pcb* pcb, pbuf* buffer, const ip_addr_t* address, u16_t port)
    {
        reinterpret_cast<DatagramExchangeLwIP*>(arg)->Recv(buffer, address, port);
    }

    void DatagramExchangeLwIP::Recv(pbuf* buffer, const ip_addr_t* address, u16_t port)
    {
        UdpReader reader(buffer);
        if (IP_GET_TYPE(address) == IPADDR_TYPE_V4)
            GetObserver().DataReceived(reader, Udpv4Socket{ IPv4Address{ ip4_addr1(ip_2_ip4(address)), ip4_addr2(ip_2_ip4(address)), ip4_addr3(ip_2_ip4(address)), ip4_addr4(ip_2_ip4(address)) }, port });
        else
            GetObserver().DataReceived(reader, Udpv6Socket{ IPv6Address{ IP6_ADDR_BLOCK1(ip_2_ip6(address)), IP6_ADDR_BLOCK2(ip_2_ip6(address)), IP6_ADDR_BLOCK3(ip_2_ip6(address)), IP6_ADDR_BLOCK4(ip_2_ip6(address)),
                IP6_ADDR_BLOCK5(ip_2_ip6(address)), IP6_ADDR_BLOCK6(ip_2_ip6(address)), IP6_ADDR_BLOCK7(ip_2_ip6(address)), IP6_ADDR_BLOCK8(ip_2_ip6(address)) }, port });
    }

    DatagramExchangeLwIP::UdpReader::UdpReader(pbuf* buffer)
        : buffer(buffer)
    {}

    DatagramExchangeLwIP::UdpReader::~UdpReader()
    {
        pbuf_free(buffer);
    }

    void DatagramExchangeLwIP::UdpReader::Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(range.size() <= Available());
        range.shrink_from_back_to(Available());

        u16_t numCopied = pbuf_copy_partial(buffer, range.begin(), static_cast<uint16_t>(range.size()), bufferOffset);
        assert(numCopied == range.size());
        bufferOffset += static_cast<uint16_t>(range.size());
    }

    uint8_t DatagramExchangeLwIP::UdpReader::Peek(infra::StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(!Empty());

        uint8_t result;
        pbuf_copy_partial(buffer, &result, 1, bufferOffset);
        return result;
    }

    infra::ConstByteRange DatagramExchangeLwIP::UdpReader::ExtractContiguousRange(std::size_t max)
    {
        uint16_t offset = bufferOffset;
        pbuf* currentBuffer = buffer;
        while (offset >= currentBuffer->len)
        {
            offset -= currentBuffer->len;
            currentBuffer = currentBuffer->next;
        }

        infra::ConstByteRange result = infra::Head(infra::ConstByteRange(reinterpret_cast<const uint8_t*>(currentBuffer->payload) + offset,
            reinterpret_cast<const uint8_t*>(currentBuffer->payload) + currentBuffer->len), max);
        bufferOffset += static_cast<uint16_t>(result.size());
        return result;
    }

    infra::ConstByteRange DatagramExchangeLwIP::UdpReader::PeekContiguousRange(std::size_t start)
    {
        infra::ConstByteRange result(reinterpret_cast<const uint8_t*>(buffer->payload) + bufferOffset + start, 
            reinterpret_cast<const uint8_t*>(buffer->payload) + buffer->len);
        return result;
    }

    bool DatagramExchangeLwIP::UdpReader::Empty() const
    {
        return Available() == 0;
    }

    std::size_t DatagramExchangeLwIP::UdpReader::Available() const
    {
        return buffer->tot_len - bufferOffset;
    }

    std::size_t DatagramExchangeLwIP::UdpReader::ConstructSaveMarker() const
    {
        return bufferOffset;
    }

    void DatagramExchangeLwIP::UdpReader::Rewind(std::size_t marker)
    {
        bufferOffset = static_cast<uint16_t>(marker);
    }

    DatagramExchangeLwIP::UdpWriter::UdpWriter(udp_pcb* control, pbuf* buffer, infra::Optional<UdpSocket> remote)
        : control(control)
        , buffer(buffer)
        , remote(remote)
    {}

    DatagramExchangeLwIP::UdpWriter::~UdpWriter()
    {
        pbuf_realloc(buffer, bufferOffset);
        if (!remote)
        {
            err_t result = udp_send(control, buffer);
            assert(result == ERR_OK);
        }
        else
        {
            auto address = Convert(*remote).first;
            err_t result = udp_sendto(control, buffer, &address, Convert(*remote).second);
            assert(result == ERR_OK);
        }
        pbuf_free(buffer);
    }

    void DatagramExchangeLwIP::UdpWriter::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(range.size() <= Available());
        range.shrink_from_back_to(Available());
        err_t result = pbuf_take_at(buffer, range.begin(), static_cast<uint16_t>(range.size()), bufferOffset);
        assert(result == ERR_OK);
        bufferOffset += static_cast<uint16_t>(range.size());
    }

    std::size_t DatagramExchangeLwIP::UdpWriter::Available() const
    {
        return buffer->tot_len - bufferOffset;
    }

    void DatagramExchangeLwIP::StateBase::RequestSendStream(std::size_t sendSize)
    {
        std::abort();
    }

    void DatagramExchangeLwIP::StateBase::RequestSendStream(std::size_t sendSize, UdpSocket remote)
    {
        std::abort();
    }

    DatagramExchangeLwIP::StateIdle::StateIdle(DatagramExchangeLwIP& datagramExchange)
        : datagramExchange(datagramExchange)
    {}

    void DatagramExchangeLwIP::StateIdle::RequestSendStream(std::size_t sendSize)
    {
        StateWaitingForBuffer& state = datagramExchange.state.Emplace<StateWaitingForBuffer>(datagramExchange, sendSize, infra::none);
        state.TryAllocateBuffer();
    }

    void DatagramExchangeLwIP::StateIdle::RequestSendStream(std::size_t sendSize, UdpSocket remote)
    {
        StateWaitingForBuffer& state = datagramExchange.state.Emplace<StateWaitingForBuffer>(datagramExchange, sendSize, infra::MakeOptional(remote));
        state.TryAllocateBuffer();
    }

    DatagramExchangeLwIP::StateWaitingForBuffer::StateWaitingForBuffer(DatagramExchangeLwIP& datagramExchange, std::size_t sendSize, infra::Optional<UdpSocket> remote)
        : datagramExchange(datagramExchange)
        , sendSize(sendSize)
        , remote(remote)
        , allocateTimer(std::chrono::milliseconds(50), [this]() { TryAllocateBuffer(); })
    {}

    void DatagramExchangeLwIP::StateWaitingForBuffer::TryAllocateBuffer()
    {
        pbuf* buffer = pbuf_alloc(PBUF_TRANSPORT, static_cast<uint16_t>(sendSize), PBUF_POOL);
        if (buffer != nullptr)
            datagramExchange.state.Emplace<StateBufferAllocated>(datagramExchange, buffer, infra::Optional<UdpSocket>(remote));
    }

    DatagramExchangeLwIP::StateBufferAllocated::StateBufferAllocated(DatagramExchangeLwIP& datagramExchange, pbuf* buffer, infra::Optional<UdpSocket> remote)
        : datagramExchange(datagramExchange)
        , stream([this]() { this->datagramExchange.state.Emplace<StateIdle>(this->datagramExchange); })
        , streamPtr(stream.Emplace(datagramExchange.control, buffer, remote))
    {
        infra::EventDispatcherWithWeakPtr::Instance().Schedule([this](const infra::SharedPtr<DatagramExchange>& datagramExchange)
        {
            datagramExchange->GetObserver().SendStreamAvailable(infra::SharedPtr<UdpWriter>(std::move(streamPtr)));
        }, datagramExchange.SharedFromThis());
    }

    infra::SharedPtr<DatagramExchange> DatagramFactoryLwIp::Listen(DatagramExchangeObserver& observer, uint16_t port, IPVersions versions)
    {
        infra::SharedPtr<DatagramExchangeLwIP> datagramExchange = allocatorDatagramExchanges.Allocate(observer);
        if (datagramExchange)
            datagramExchange->Listen(port, versions);
        return datagramExchange;
    }

    infra::SharedPtr<DatagramExchange> DatagramFactoryLwIp::Listen(DatagramExchangeObserver& observer, IPVersions versions)
    {
        infra::SharedPtr<DatagramExchangeLwIP> datagramExchange = allocatorDatagramExchanges.Allocate(observer);
        if (datagramExchange)
            datagramExchange->Listen(versions);
        return datagramExchange;
    }

    infra::SharedPtr<DatagramExchange> DatagramFactoryLwIp::Connect(DatagramExchangeObserver& observer, UdpSocket remote)
    {
        infra::SharedPtr<DatagramExchangeLwIP> datagramExchange = allocatorDatagramExchanges.Allocate(observer);
        if (datagramExchange)
            datagramExchange->Connect(remote);
        return datagramExchange;
    }

    infra::SharedPtr<DatagramExchange> DatagramFactoryLwIp::Connect(DatagramExchangeObserver& observer, uint16_t localPort, UdpSocket remote)
    {
        infra::SharedPtr<DatagramExchangeLwIP> datagramExchange = allocatorDatagramExchanges.Allocate(observer);
        if (datagramExchange)
            datagramExchange->Connect(localPort, remote);
        return datagramExchange;
    }
}
