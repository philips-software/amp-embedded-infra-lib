#ifndef LWIP_DATAGRAM_LW_IP_HPP
#define LWIP_DATAGRAM_LW_IP_HPP

#include "infra/timer/Timer.hpp"
#include "infra/util/SharedObjectAllocatorFixedSize.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/PolymorphicVariant.hpp"
#include "lwip/udp.h"
#include "lwip/lwip_cpp/ConnectionLwIp.hpp"
#include "services/network/Datagram.hpp"

namespace services
{
    class DatagramExchangeLwIP
        : public DatagramExchange
        , public infra::EnableSharedFromThis<DatagramExchangeLwIP>
    {
    public:
        DatagramExchangeLwIP(DatagramExchangeObserver& observer);
        ~DatagramExchangeLwIP();

        void Listen(uint16_t port, IPVersions versions);
        void Listen(IPVersions versions);
        void Connect(UdpSocket remote);
        void Connect(uint16_t localPort, UdpSocket remote);

        // Implementation of DatagramExchange
        virtual void RequestSendStream(std::size_t sendSize) override;
        virtual void RequestSendStream(std::size_t sendSize, UdpSocket to) override;

    private:
        udp_pcb* CreateUdpPcb(IPVersions versions) const;
        const ip_addr_t* IpAddrAny(IPVersions versions) const;
        static void StaticRecv(void* arg, udp_pcb* pcb, pbuf* buffer, const ip_addr_t* address, u16_t port);
        void Recv(pbuf* buffer, const ip_addr_t* address, u16_t port);

    private:
        class UdpReader
            : public infra::StreamReaderWithRewinding
        {
        public:
            UdpReader(pbuf* buffer);
            ~UdpReader();

            virtual void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
            virtual uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
            virtual infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
            virtual infra::ConstByteRange PeekContiguousRange(std::size_t start) override;
            virtual bool Empty() const override;
            virtual std::size_t Available() const override;
            virtual std::size_t ConstructSaveMarker() const override;
            virtual void Rewind(std::size_t marker) override;

        private:
            pbuf* buffer;
            uint16_t bufferOffset = 0;
        };

        class UdpWriter
            : public infra::StreamWriter
        {
        public:
            UdpWriter(udp_pcb* control, pbuf* buffer, infra::Optional<UdpSocket> remote);
            ~UdpWriter();

            virtual void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
            virtual std::size_t Available() const override;

        private:
            udp_pcb* control;
            pbuf* buffer;
            infra::Optional<UdpSocket> remote;
            uint16_t bufferOffset = 0;
        };

        class StateBase
        {
        public:
            virtual ~StateBase() = default;

            virtual void RequestSendStream(std::size_t sendSize);
            virtual void RequestSendStream(std::size_t sendSize, UdpSocket remote);
        };

        class StateIdle
            : public StateBase
        {
        public:
            StateIdle(DatagramExchangeLwIP& datagramExchange);

            virtual void RequestSendStream(std::size_t sendSize) override;
            virtual void RequestSendStream(std::size_t sendSize, UdpSocket remote) override;

        private:
            DatagramExchangeLwIP& datagramExchange;
        };

        class StateWaitingForBuffer
            : public StateBase
        {
        public:
            StateWaitingForBuffer(DatagramExchangeLwIP& datagramExchange, std::size_t sendSize, infra::Optional<UdpSocket> remote);

            void TryAllocateBuffer();

        private:
            DatagramExchangeLwIP& datagramExchange;
            std::size_t sendSize;
            infra::Optional<UdpSocket> remote;
            infra::TimerRepeating allocateTimer;
        };

        class StateBufferAllocated
            : public StateBase
        {
        public:
            StateBufferAllocated(DatagramExchangeLwIP& datagramExchange, pbuf* buffer, infra::Optional<UdpSocket> remote);

        private:
            DatagramExchangeLwIP& datagramExchange;
            infra::NotifyingSharedOptional<UdpWriter> stream;
            infra::SharedPtr<UdpWriter> streamPtr;
        };

    private:
        udp_pcb* control = nullptr;
        infra::PolymorphicVariant<StateBase, StateIdle, StateWaitingForBuffer, StateBufferAllocated> state;
    };

    using AllocatorDatagramExchangeLwIp = infra::SharedObjectAllocator<DatagramExchangeLwIP, void(DatagramExchangeObserver& observer)>;

    class DatagramFactoryLwIp
        : public DatagramFactory
    {
    public:
        virtual infra::SharedPtr<DatagramExchange> Listen(DatagramExchangeObserver& observer, uint16_t port, IPVersions versions = IPVersions::both) override;
        virtual infra::SharedPtr<DatagramExchange> Listen(DatagramExchangeObserver& observer, IPVersions versions = IPVersions::both) override;
        virtual infra::SharedPtr<DatagramExchange> Connect(DatagramExchangeObserver& observer, UdpSocket remote) override;
        virtual infra::SharedPtr<DatagramExchange> Connect(DatagramExchangeObserver& observer, uint16_t localPort, UdpSocket remote) override;

    private:
        AllocatorDatagramExchangeLwIp::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MEMP_NUM_UDP_PCB> allocatorDatagramExchanges;
    };
}

#endif
