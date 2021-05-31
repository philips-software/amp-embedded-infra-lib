#include "gmock/gmock.h"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/util/ConstructBin.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/network/MdnsClient.hpp"
#include "services/network/test_doubles/DatagramMock.hpp"
#include "services/network/test_doubles/MulticastMock.hpp"

namespace
{
    const uint16_t mdnsPort = 5353;
    const services::IPAddress mdnsMulticastAddressIpv4{ services::IPv4Address{ 224, 0, 0, 251 } };
    const services::IPAddress mdnsMulticastAddressIpv6{ services::IPv6Address{ 0xff02, 0, 0, 0, 0, 0, 0, 0xfb } };
}

class MdnsClientTest
    : public testing::Test
{
public:
     void ExpectListenIpv4()
     {
        EXPECT_CALL(factory, Listen(testing::_, mdnsPort, services::IPVersions::ipv4)).WillOnce(testing::Invoke([this](services::DatagramExchangeObserver& observer, uint16_t port, services::IPVersions versions)
        {
            auto ptr = datagramExchange.Emplace();
            observer.Attach(*ptr);

            return ptr;
        }));
     }

    void ExpectJoinMulticastIpv4()
    {
        EXPECT_CALL(multicast, JoinMulticastGroup(testing::_, mdnsMulticastAddressIpv4.Get<services::IPv4Address>()));
    }

    void ExpectLeaveMulticastIpv4()
    {
        EXPECT_CALL(multicast, LeaveMulticastGroup(testing::_, mdnsMulticastAddressIpv4.Get<services::IPv4Address>()));
    }

    void ExpectRequestSendStream()
    {
        EXPECT_CALL(*datagramExchange, RequestSendStream(testing::_, testing::_));
    }

    void DataReceived(const std::vector<uint8_t>& data, services::IPv4Address address = services::IPv4Address{ 1, 2, 3, 4 }, uint16_t port = mdnsPort)
    {
        datagramExchange->GetObserver().DataReceived(infra::MakeSharedOnHeap<infra::StdVectorInputStreamReader::WithStorage>(infra::inPlace, data), services::Udpv4Socket{ address, port });
    }

    void DataReceived(const std::vector<uint8_t>& data, services::IPv6Address address, uint16_t port = mdnsPort)
    {
        datagramExchange->GetObserver().DataReceived(infra::MakeSharedOnHeap<infra::StdVectorInputStreamReader::WithStorage>(infra::inPlace, data), services::Udpv6Socket{ address, port });
    }

    void DataReceived(const std::vector<uint8_t>& data, services::IPAddress address, uint16_t port = mdnsPort)
    {
        if (address.Is<services::IPv4Address>())
            DataReceived(data, address.Get<services::IPv4Address>(), port);
        else
            DataReceived(data, address.Get<services::IPv6Address>(), port);
    }

    void SendStreamAvailableAndExpectQuestion(const std::vector<uint8_t>& data)
    {
        infra::StdVectorOutputStreamWriter::WithStorage response;
        datagramExchange->GetObserver().SendStreamAvailable(infra::UnOwnedSharedPtr(response));
        EXPECT_EQ(data, response.Storage());
    }

    void SendStreamAvailableAndExpectNoQuestion()
    {
        infra::StdVectorOutputStreamWriter::WithStorage response;
        datagramExchange->GetObserver().SendStreamAvailable(infra::UnOwnedSharedPtr(response));
        EXPECT_EQ(true, response.Storage().empty());
    }

    std::vector<uint8_t> QuestionHeader()
    {
        return infra::ConstructBin()
            .Value<services::DnsRecordHeader>({ 0x0000, 0x0100, 1, 0, 0, 0 })
            .Vector();
    }

    std::vector<uint8_t> AQuestion()
    {
        return infra::ConstructBin()
            (QuestionHeader())
            (9)("_instance")(5)("local")(0)
            .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypeA, services::DnsClass::dnsClassIn})
            .Vector();
    }

    std::vector<uint8_t> AaaaQuestion()
    {
        return infra::ConstructBin()
            (QuestionHeader())
            (9)("_instance")(5)("local")(0)
            .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypeAAAA, services::DnsClass::dnsClassIn})
            .Vector();
    }

    std::vector<uint8_t> PtrQuestion()
    {
        return infra::ConstructBin()
            (QuestionHeader())
            (8)("_service")(9)("_protocol")(5)("local")(0)
            .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn})
            .Vector();
    }

    std::vector<uint8_t> TxtQuestion()
    {
        return infra::ConstructBin()
            (QuestionHeader())
            (9)("_instance")(8)("_service")(9)("_protocol")(5)("local")(0)
            .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypeTxt, services::DnsClass::dnsClassIn})
            .Vector();
    }

    std::vector<uint8_t> SrvQuestion()
    {
        return infra::ConstructBin()
            (QuestionHeader())
            (9)("_instance")(8)("_service")(9)("_protocol")(5)("local")(0)
            .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypeSrv, services::DnsClass::dnsClassIn})
            .Vector();
    }

        std::vector<uint8_t> AAnswerData()
    {
        return infra::ConstructBin()
            .Value<services::IPv4Address>({ 1, 2, 3, 4 })
            .Vector();
    }

    std::vector<uint8_t> AaaaAnswerData()
    {
        return infra::ConstructBin()
            .Value<services::IPv6AddressNetworkOrder>({ 1, 2, 3, 4, 5, 6, 7, 8 })
            .Vector();
    }

    std::vector<uint8_t> PtrAnswerData()
    {
        return infra::ConstructBin()
            (8)("instance")(7)("service")(4)("type")(5)("local")(0)
            .Vector();
    }

    std::vector<uint8_t> TxtAnswerData()
    {
        return infra::ConstructBin()
            (7)("aa=text")(12)("bb=othertext")
            .Vector();
    }

    std::vector<uint8_t> SrvAnswerData()
    {
        return infra::ConstructBin()
            .Value<infra::BigEndian<uint16_t>>(0).Value<infra::BigEndian<uint16_t>>(0).Value<infra::BigEndian<uint16_t>>(1234)
            (8)("instance")(5)("local")(0)
            .Vector();
    }

    void AAnswerReceived(services::IPAddress source = mdnsMulticastAddressIpv4)
    {
        DataReceived(infra::ConstructBin()
            .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 0 })
            (9)("_instance")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeA, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 4 })
            (AAnswerData())
            .Vector(), source);
    }

    void AaaaAnswerReceived(services::IPAddress source = mdnsMulticastAddressIpv6)
    {
        DataReceived(infra::ConstructBin()
            .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 0 })
            (9)("_instance")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeAAAA, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 16 })
            (AaaaAnswerData())
            .Vector(), source);
    }

    void PtrAnswerReceived(services::IPAddress source = mdnsMulticastAddressIpv4)
    {
        DataReceived(infra::ConstructBin()
            .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 0 })
            (8)("_service")(9)("_protocol")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 0x1d })
            (PtrAnswerData())
            .Vector(), source);
    }

    void TxtAnswerReceived(services::IPAddress source = mdnsMulticastAddressIpv4)
    {
        DataReceived(infra::ConstructBin()
            .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 0 })
            (9)("_instance")(8)("_service")(9)("_protocol")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeTxt, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 0x15 })
            (TxtAnswerData())
            .Vector(), source);
    }

    void SrvAnswerReceived(services::IPAddress source = mdnsMulticastAddressIpv4)
    {
        DataReceived(infra::ConstructBin()
            .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 0 })
            (9)("_instance")(8)("_service")(9)("_protocol")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeSrv, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 0x16 })
            (SrvAnswerData())
            .Vector(), source);
    }

    void PtrAnswerReceivedWithAdditionalRecords(services::IPAddress source = mdnsMulticastAddressIpv4)
    {
        DataReceived(infra::ConstructBin()
            .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 4 })
            (8)("_service")(9)("_protocol")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 0x1d })
            (PtrAnswerData())
            (9)("_instance")(8)("_service")(9)("_protocol")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeTxt, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 0x15 })
            (TxtAnswerData())
            (9)("_instance")(8)("_service")(9)("_protocol")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeSrv, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 0x16 })
            (SrvAnswerData())
            (9)("_instance")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeA, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 4 })
            (AAnswerData())
            (9)("_instance")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeAAAA, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 16 })
            (AaaaAnswerData())
            .Vector(), source);
    }

    void QueryA(infra::Function<void(infra::ConstByteRange data)>& callback)
    {
        queryA.Emplace(client, services::DnsType::dnsTypeA, "_instance", callback);
    }

    void QueryAaaa(infra::Function<void(infra::ConstByteRange data)>& callback)
    {
        queryAaaa.Emplace(client, services::DnsType::dnsTypeAAAA, "_instance", callback);
    }

    void QueryPtr(infra::Function<void(infra::ConstByteRange data)>& callback)
    {
        queryPtr.Emplace(client, services::DnsType::dnsTypePtr, "_service", "_protocol", callback);
    }

    void QueryTxt(infra::Function<void(infra::ConstByteRange data)>& callback)
    {
        queryTxt.Emplace(client, services::DnsType::dnsTypeTxt, "_instance", "_service", "_protocol", callback);
    }

    void QuerySrv(infra::Function<void(infra::ConstByteRange data)>& callback)
    {
        querySrv.Emplace(client, services::DnsType::dnsTypeSrv, "_instance", "_service", "_protocol", callback);
    }

    void DestructQueryPtr()
    {
        queryPtr = infra::none;
    }

    void ConstructAllQueries()
    {
        QueryA(queryACallback);
        QueryAaaa(queryAaaaCallback);
        QueryPtr(queryPtrCallback);
        QueryTxt(queryTxtCallback);
        QuerySrv(querySrvCallback);
    }

    void ConstructAllQueriesWithExpectedCallback()
    {
        QueryA(expectedQueryACallback);
        QueryAaaa(expectedQueryAaaaCallback);
        QueryPtr(expectedQueryPtrCallback);
        QueryTxt(expectedQueryTxtCallback);
        QuerySrv(expectedQuerySrvCallback);
    }

    void DestructAllQueries()
    {
        queryA = infra::none;
        queryAaaa = infra::none;
        queryPtr = infra::none;
        queryTxt = infra::none;
        querySrv = infra::none;
    }

    testing::StrictMock<services::MulticastMock> multicast;
    testing::StrictMock<services::DatagramFactoryMock> factory;
    infra::SharedOptional<testing::StrictMock<services::DatagramExchangeMock>> datagramExchange;
    services::MdnsClient client{ factory, multicast };

    infra::Function<void(infra::ConstByteRange data)> queryACallback{ [](infra::ConstByteRange data) { FAIL(); } };
    infra::Function<void(infra::ConstByteRange data)> queryAaaaCallback{ [](infra::ConstByteRange data) { FAIL(); } };
    infra::Function<void(infra::ConstByteRange data)> queryPtrCallback{ [](infra::ConstByteRange data) { FAIL(); } };
    infra::Function<void(infra::ConstByteRange data)> queryTxtCallback{ [](infra::ConstByteRange data) { FAIL(); } };
    infra::Function<void(infra::ConstByteRange data)> querySrvCallback{ [](infra::ConstByteRange data) { FAIL(); } };

    infra::MockCallback<void(services::DnsType dnsType)> callback;
    infra::Function<void(infra::ConstByteRange data)> expectedQueryACallback{ [&](infra::ConstByteRange data) { callback.callback(services::DnsType::dnsTypeA); EXPECT_EQ(data, AAnswerData()); } };
    infra::Function<void(infra::ConstByteRange data)> expectedQueryAaaaCallback{ [&](infra::ConstByteRange data) { callback.callback(services::DnsType::dnsTypeAAAA); EXPECT_EQ(data, AaaaAnswerData()); } };
    infra::Function<void(infra::ConstByteRange data)> expectedQueryPtrCallback{ [&](infra::ConstByteRange data) { callback.callback(services::DnsType::dnsTypePtr); EXPECT_EQ(data, PtrAnswerData()); } };
    infra::Function<void(infra::ConstByteRange data)> expectedQueryTxtCallback{ [&](infra::ConstByteRange data) { callback.callback(services::DnsType::dnsTypeTxt); EXPECT_EQ(data, TxtAnswerData()); } };
    infra::Function<void(infra::ConstByteRange data)> expectedQuerySrvCallback{ [&](infra::ConstByteRange data) { callback.callback(services::DnsType::dnsTypeSrv); EXPECT_EQ(data, SrvAnswerData()); } };

    infra::Optional<services::MdnsQueryImpl> queryA;
    infra::Optional<services::MdnsQueryImpl> queryAaaa;
    infra::Optional<services::MdnsQueryImpl> queryPtr;
    infra::Optional<services::MdnsQueryImpl> queryTxt;
    infra::Optional<services::MdnsQueryImpl> querySrv;
};

TEST_F(MdnsClientTest, nothing_happens_when_no_queries_registered)
{
    
}

TEST_F(MdnsClientTest, constructed_query_starts_mdns_connection)
{
    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    QueryPtr(queryPtrCallback);
    ExpectLeaveMulticastIpv4();
}

TEST_F(MdnsClientTest, multiple_constructed_queries_starts_one_mdns_connection)
{
    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    ConstructAllQueries();
    ExpectLeaveMulticastIpv4();
}

TEST_F(MdnsClientTest, destructing_all_queries_stops_mdns_connection)
{
    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    ConstructAllQueries();

    ExpectLeaveMulticastIpv4();
    DestructAllQueries();

    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    QueryPtr(queryPtrCallback);
    ExpectLeaveMulticastIpv4();
}

TEST_F(MdnsClientTest, query_asking_starts_active_query)
{
    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    QueryPtr(queryPtrCallback);

    ExpectRequestSendStream();
    queryPtr->Ask();

    auto question = PtrQuestion();
    SendStreamAvailableAndExpectQuestion(question);

    ExpectLeaveMulticastIpv4();
}

TEST_F(MdnsClientTest, other_query_asking_starts_active_query_after_first_query_is_done)
{
    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    ConstructAllQueries();

    ExpectRequestSendStream();

    queryPtr->Ask();
    queryA->Ask();

    ExpectRequestSendStream();
    auto ptrQuestion = PtrQuestion();
    SendStreamAvailableAndExpectQuestion(ptrQuestion);

    auto aQuestion = AQuestion();
    SendStreamAvailableAndExpectQuestion(aQuestion);

    ExpectLeaveMulticastIpv4();
}

TEST_F(MdnsClientTest, other_query_asking_starts_active_query_after_first_query_is_canceled)
{
    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    ConstructAllQueries();

    ExpectRequestSendStream();

    queryPtr->Ask();
    queryA->Ask();

    DestructQueryPtr();

    ExpectRequestSendStream();
    SendStreamAvailableAndExpectNoQuestion();

    auto aQuestion = AQuestion();
    SendStreamAvailableAndExpectQuestion(aQuestion);

    ExpectLeaveMulticastIpv4();
}

TEST_F(MdnsClientTest, other_query_asking_starts_active_query_after_first_query_is_done_and_asked_again)
{
    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    ConstructAllQueries();

    ExpectRequestSendStream();
    queryPtr->Ask();
    queryA->Ask();

    ExpectRequestSendStream();
    auto ptrQuestion = PtrQuestion();
    SendStreamAvailableAndExpectQuestion(ptrQuestion);
    queryPtr->Ask();

    ExpectRequestSendStream();
    auto aQuestion = AQuestion();
    SendStreamAvailableAndExpectQuestion(aQuestion);

    SendStreamAvailableAndExpectQuestion(ptrQuestion);

    ExpectLeaveMulticastIpv4();
}

TEST_F(MdnsClientTest, nothing_happens_when_receiving_from_incorrect_port)
{
    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    QueryPtr(queryPtrCallback);
    DataReceived(std::vector<uint8_t>(), services::IPv4Address{ 1, 2, 3, 4 }, 1);
    ExpectLeaveMulticastIpv4();
}

TEST_F(MdnsClientTest, nothing_happens_when_receiving_empty_packet)
{
    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    QueryPtr(queryPtrCallback);
    DataReceived(std::vector<uint8_t>());
    ExpectLeaveMulticastIpv4();
}

TEST_F(MdnsClientTest, nothing_happens_when_receiving_non_matching_answer)
{
    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    QueryPtr(queryPtrCallback);
    AAnswerReceived();
    ExpectLeaveMulticastIpv4();
}

TEST_F(MdnsClientTest, destructed_passive_query_is_not_handled)
{
    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    ConstructAllQueries();

    DestructQueryPtr();

    PtrAnswerReceived();

    ExpectLeaveMulticastIpv4();
}

TEST_F(MdnsClientTest, destructed_active_query_is_not_handled)
{
    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    ConstructAllQueries();

    ExpectRequestSendStream();
    queryPtr->Ask();

    DestructQueryPtr();

    SendStreamAvailableAndExpectNoQuestion();

    PtrAnswerReceived();

    ExpectLeaveMulticastIpv4();
}

TEST_F(MdnsClientTest, receiving_matching_answer_to_passive_query_results_in_data)
{
    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    ConstructAllQueriesWithExpectedCallback();

    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeA));
    AAnswerReceived();

    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeAAAA));
    AaaaAnswerReceived();

    EXPECT_CALL(callback, callback(services::DnsType::dnsTypePtr));
    PtrAnswerReceived();

    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeTxt));
    TxtAnswerReceived();

    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeSrv));
    SrvAnswerReceived();

    ExpectLeaveMulticastIpv4();
}

TEST_F(MdnsClientTest, receiving_matching_answer_to_active_query_results_in_data)
{
    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    ConstructAllQueriesWithExpectedCallback();

    ExpectRequestSendStream();
    queryA->Ask();
    queryAaaa->Ask();
    queryPtr->Ask();
    queryTxt->Ask();
    querySrv->Ask();

    ExpectRequestSendStream();
    auto aQuestion = AQuestion();
    SendStreamAvailableAndExpectQuestion(aQuestion);
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeA));
    AAnswerReceived();

    ExpectRequestSendStream();
    auto aaaaQuestion = AaaaQuestion();
    SendStreamAvailableAndExpectQuestion(aaaaQuestion);
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeAAAA));
    AaaaAnswerReceived();

    ExpectRequestSendStream();
    auto ptrQuestion = PtrQuestion();
    SendStreamAvailableAndExpectQuestion(ptrQuestion);
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypePtr));
    PtrAnswerReceived();

    ExpectRequestSendStream();
    auto txtQuestion = TxtQuestion();
    SendStreamAvailableAndExpectQuestion(txtQuestion);
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeTxt));
    TxtAnswerReceived();

    auto srvQuestion = SrvQuestion();
    SendStreamAvailableAndExpectQuestion(srvQuestion);
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeSrv));
    SrvAnswerReceived();

    ExpectLeaveMulticastIpv4();
}

TEST_F(MdnsClientTest, receiving_matching_additional_answer_to_passive_query_results_in_data)
{
    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    ConstructAllQueriesWithExpectedCallback();

    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeA));
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeAAAA));
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypePtr));
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeTxt));
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeSrv));
    PtrAnswerReceivedWithAdditionalRecords();

    ExpectLeaveMulticastIpv4();
}

TEST_F(MdnsClientTest, receiving_matching_additional_answer_to_active_query_results_in_data)
{
    ExpectListenIpv4();
    ExpectJoinMulticastIpv4();
    ConstructAllQueriesWithExpectedCallback();

    ExpectRequestSendStream();
    queryPtr->Ask();

    auto ptrQuestion = PtrQuestion();
    SendStreamAvailableAndExpectQuestion(ptrQuestion);

    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeA));
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeAAAA));
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypePtr));
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeTxt));
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeSrv));
    PtrAnswerReceivedWithAdditionalRecords();

    ExpectLeaveMulticastIpv4();
}
