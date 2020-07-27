#ifndef SERVICES_DATAGRAM_HPP
#define SERVICES_DATAGRAM_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/stream/OutputStream.hpp"
#include "infra/util/Observer.hpp"
#include "infra/util/SharedPtr.hpp"
#include "services/network/Address.hpp"

namespace services
{
    class DatagramExchange;

    class DatagramExchangeObserver
        : public infra::SingleObserver<DatagramExchangeObserver, DatagramExchange>
    {
    public:
        virtual void DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, UdpSocket from) = 0;

        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) = 0;
    };

    class DatagramExchange
        : public infra::Subject<DatagramExchangeObserver>
    {
    public:
        // RequestStream without address is only possible on DatagramExchange objects obtained via a Connect call
        virtual void RequestSendStream(std::size_t sendSize) = 0;

        // RequestStream with address is possible on all DatagramExchange objects
        virtual void RequestSendStream(std::size_t sendSize, UdpSocket to) = 0;
    };

    class DatagramFactory
    {
    protected:
        DatagramFactory() = default;
        DatagramFactory(const DatagramFactory& other) = delete;
        DatagramFactory& operator=(const DatagramFactory& other) = delete;
        virtual ~DatagramFactory() = default;

    public:
        virtual infra::SharedPtr<DatagramExchange> Listen(DatagramExchangeObserver& observer, uint16_t port, IPVersions versions = IPVersions::both) = 0;
        // Listen without a port picks a random local port
        virtual infra::SharedPtr<DatagramExchange> Listen(DatagramExchangeObserver& observer, IPVersions versions = IPVersions::both) = 0;
        // DatagramExchange objects obtained with a Connect call have a random local port if not explicitly specified
        virtual infra::SharedPtr<DatagramExchange> Connect(DatagramExchangeObserver& observer, UdpSocket remote) = 0;
        virtual infra::SharedPtr<DatagramExchange> Connect(DatagramExchangeObserver& observer, uint16_t localPort, UdpSocket remote) = 0;
    };
}

#endif
