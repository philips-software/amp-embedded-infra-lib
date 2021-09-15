#ifndef NETWORK_DATAGRAM_MOCK_HPP
#define NETWORK_DATAGRAM_MOCK_HPP

#include "services/network/Datagram.hpp"
#include "gmock/gmock.h"

namespace services
{
    class DatagramExchangeMock
        : public DatagramExchange
    {
    public:
        ~DatagramExchangeMock()
        {
            if (HasObserver())
                GetObserver().Detach();
        }

        MOCK_METHOD1(RequestSendStream, void(std::size_t sendSize));
        MOCK_METHOD2(RequestSendStream, void(std::size_t sendSize, UdpSocket to));
    };

    class DatagramFactoryMock
        : public DatagramFactory
    {
    public:
        MOCK_METHOD3(Listen, infra::SharedPtr<DatagramExchange>(DatagramExchangeObserver& observer, uint16_t port, IPVersions versions));
        MOCK_METHOD2(Listen, infra::SharedPtr<DatagramExchange>(DatagramExchangeObserver& observer, IPVersions versions));
        MOCK_METHOD2(Connect, infra::SharedPtr<DatagramExchange>(DatagramExchangeObserver& observer, UdpSocket remote));
        MOCK_METHOD3(Connect, infra::SharedPtr<DatagramExchange>(DatagramExchangeObserver& observer, uint16_t localPort, UdpSocket remote));
    };
}

#endif
