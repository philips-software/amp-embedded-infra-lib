#include "gmock/gmock.h"
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
    ~MdnsClientTest()
    {
        ExpectLeaveMulticastIpv4();
    }

    void ExpectListenIpv4()
    {
        EXPECT_CALL(factory, Listen(testing::_, mdnsPort, services::IPVersions::ipv4)).WillOnce(testing::Invoke([this](services::DatagramExchangeObserver& observer, uint16_t port, services::IPVersions versions) {
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

    void ExpectActiveQueryStarted()
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
            .Value<services::DnsRecordHeader>({ 0x0000, 0x0000, 0x0001, 0x0000, 0x0000, 0x0000 })
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

    std::vector<uint8_t> AAnswer()
    {
        return infra::ConstructBin()
            .Value<services::IPv4Address>({ 1, 2, 3, 4 })
            .Vector();
    }

    std::vector<uint8_t> AaaaAnswer()
    {
        return infra::ConstructBin()
            .Value<services::IPv6AddressNetworkOrder>({ 1, 2, 3, 4, 5, 6, 7, 8 })
            .Vector();
    }

    std::vector<uint8_t> PtrAnswer()
    {
        return infra::ConstructBin()
            (8)("instance")(7)("service")(4)("type")(5)("local")(0)
            .Vector();
    }

    std::vector<uint8_t> TxtAnswer()
    {
        return infra::ConstructBin()
            (7)("aa=text")(12)("bb=othertext")
            .Vector();
    }

    std::vector<uint8_t> SrvAnswer()
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
            (AAnswer())
            .Vector(), source);
    }

    void AaaaAnswerReceived(services::IPAddress source = mdnsMulticastAddressIpv6)
    {
        DataReceived(infra::ConstructBin()
            .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 0 })
            (9)("_instance")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeAAAA, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 16 })
            (AaaaAnswer())
            .Vector(), source);
    }

    void PtrAnswerReceived(services::IPAddress source = mdnsMulticastAddressIpv4)
    {
        DataReceived(infra::ConstructBin()
            .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 0 })
            (8)("_service")(9)("_protocol")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 0x1d })
            (PtrAnswer())
            .Vector(), source);
    }

    void TxtAnswerReceived(services::IPAddress source = mdnsMulticastAddressIpv4)
    {
        DataReceived(infra::ConstructBin()
            .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 0 })
            (9)("_instance")(8)("_service")(9)("_protocol")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeTxt, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 0x15 })
            (TxtAnswer())
            .Vector(), source);
    }

    void SrvAnswerReceived(services::IPAddress source = mdnsMulticastAddressIpv4)
    {
        DataReceived(infra::ConstructBin()
            .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 0 })
            (9)("_instance")(8)("_service")(9)("_protocol")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeSrv, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 0x16 })
            (SrvAnswer())
            .Vector(), source);
    }

    void PtrAnswerReceivedWithAdditionalRecords(services::IPAddress source = mdnsMulticastAddressIpv4)
    {
        DataReceived(infra::ConstructBin()
            .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 4 })
            (8)("_service")(9)("_protocol")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 0x1d })
            (PtrAnswer())
            (9)("_instance")(8)("_service")(9)("_protocol")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeTxt, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 0x15 })
            (TxtAnswer())
            (9)("_instance")(8)("_service")(9)("_protocol")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeSrv, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 0x16 })
            (SrvAnswer())
            (9)("_instance")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeA, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 4 })
            (AAnswer())
            (9)("_instance")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeAAAA, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 16 })
            (AaaaAnswer())
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

    void QueryPtr(infra::Function<void(infra::ConstByteRange data)>& callback, infra::Function<void(infra::BoundedString hostname, services::DnsRecordPayload payload, infra::ConstByteRange data)>& additionalRecordsCallback)
    {
        queryPtr.Emplace(client, services::DnsType::dnsTypePtr, "_service", "_protocol", callback, additionalRecordsCallback);
    }

    void QueryTxt(infra::Function<void(infra::ConstByteRange data)>& callback)
    {
        queryTxt.Emplace(client, services::DnsType::dnsTypeTxt, "_instance", "_service", "_protocol", callback);
    }

    void QuerySrv(infra::Function<void(infra::ConstByteRange data)>& callback)
    {
        querySrv.Emplace(client, services::DnsType::dnsTypeSrv, "_instance", "_service", "_protocol", callback);
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

    void ConstructPtrQueryWithExpectedCallbackAndAdditionalRecordsCallback()
    {
        QueryPtr(expectedQueryPtrCallback, expectedAdditionalRecordsCallbackPtr);
    }

    void DestructAllQueries()
    {
        queryA = infra::none;
        queryAaaa = infra::none;
        queryPtr = infra::none;
        queryTxt = infra::none;
        querySrv = infra::none;
    }

    void DestructQueryPtr()
    {
        queryPtr = infra::none;
    }

    testing::StrictMock<services::MulticastMock> multicast;
    testing::StrictMock<services::DatagramFactoryMock> factory;
    infra::SharedOptional<testing::StrictMock<services::DatagramExchangeMock>> datagramExchange;
    infra::Execute execute{ [this]
    {
        ExpectListenIpv4();
        ExpectJoinMulticastIpv4();
    } };
    services::MdnsClient client{ factory, multicast };

    infra::Function<void(infra::ConstByteRange data)> queryACallback{ [](infra::ConstByteRange data) { FAIL(); } };
    infra::Function<void(infra::ConstByteRange data)> queryAaaaCallback{ [](infra::ConstByteRange data) { FAIL(); } };
    infra::Function<void(infra::ConstByteRange data)> queryPtrCallback{ [](infra::ConstByteRange data) { FAIL(); } };
    infra::Function<void(infra::ConstByteRange data)> queryTxtCallback{ [](infra::ConstByteRange data) { FAIL(); } };
    infra::Function<void(infra::ConstByteRange data)> querySrvCallback{ [](infra::ConstByteRange data) { FAIL(); } };

    infra::MockCallback<void(services::DnsType dnsType)> callback;
    infra::Function<void(infra::ConstByteRange data)> expectedQueryACallback{ [&](infra::ConstByteRange data) { callback.callback(services::DnsType::dnsTypeA); EXPECT_EQ(data, AAnswer()); } };
    infra::Function<void(infra::ConstByteRange data)> expectedQueryAaaaCallback{ [&](infra::ConstByteRange data) { callback.callback(services::DnsType::dnsTypeAAAA); EXPECT_EQ(data, AaaaAnswer()); } };
    infra::Function<void(infra::ConstByteRange data)> expectedQueryPtrCallback{ [&](infra::ConstByteRange data) { callback.callback(services::DnsType::dnsTypePtr); EXPECT_EQ(data, PtrAnswer()); } };
    infra::Function<void(infra::ConstByteRange data)> expectedQueryTxtCallback{ [&](infra::ConstByteRange data) { callback.callback(services::DnsType::dnsTypeTxt); EXPECT_EQ(data, TxtAnswer()); } };
    infra::Function<void(infra::ConstByteRange data)> expectedQuerySrvCallback{ [&](infra::ConstByteRange data) { callback.callback(services::DnsType::dnsTypeSrv); EXPECT_EQ(data, SrvAnswer()); } };

    infra::MockCallback<void(services::DnsType dnsType)> additionalRecordsCallback; 
    infra::Function<void(infra::BoundedString hostname, services::DnsRecordPayload payload, infra::ConstByteRange data)> expectedAdditionalRecordsCallbackPtr
    { 
        [&](infra::BoundedString hostname, services::DnsRecordPayload payload, infra::ConstByteRange data)
        {
            if (payload.type == services::DnsType::dnsTypeA)
            {
                EXPECT_EQ(data, AAnswer());
                additionalRecordsCallback.callback(services::DnsType::dnsTypeA);
            }
            if (payload.type == services::DnsType::dnsTypeAAAA)
            {
                EXPECT_EQ(data, AaaaAnswer());
                additionalRecordsCallback.callback(services::DnsType::dnsTypeAAAA);
            }
            if (payload.type == services::DnsType::dnsTypeTxt)
            {
                EXPECT_EQ(data, TxtAnswer());
                additionalRecordsCallback.callback(services::DnsType::dnsTypeTxt);
            }
            if (payload.type == services::DnsType::dnsTypeSrv)
            {
                EXPECT_EQ(data, SrvAnswer());
                additionalRecordsCallback.callback(services::DnsType::dnsTypeSrv);
            }
        }
    };

    infra::Optional<services::MdnsQueryImpl> queryA;
    infra::Optional<services::MdnsQueryImpl> queryAaaa;
    infra::Optional<services::MdnsQueryImpl> queryPtr;
    infra::Optional<services::MdnsQueryImpl> queryTxt;
    infra::Optional<services::MdnsQueryImpl> querySrv;
};

TEST_F(MdnsClientTest, query_asking_starts_active_query)
{
    QueryPtr(queryPtrCallback);

    ExpectActiveQueryStarted();
    queryPtr->Ask();

    auto question = PtrQuestion();
    SendStreamAvailableAndExpectQuestion(question);
}

TEST_F(MdnsClientTest, other_query_asking_starts_active_query_after_first_query_is_done)
{
    ConstructAllQueries();

    ExpectActiveQueryStarted();
    queryPtr->Ask();
    queryA->Ask();

    ExpectActiveQueryStarted();
    auto ptrQuestion = PtrQuestion();
    SendStreamAvailableAndExpectQuestion(ptrQuestion);
    
    auto aQuestion = AQuestion();
    SendStreamAvailableAndExpectQuestion(aQuestion);
}

TEST_F(MdnsClientTest, other_query_asking_starts_active_query_after_first_query_is_canceled)
{
    ConstructAllQueries();

    ExpectActiveQueryStarted();
    queryPtr->Ask();
    queryA->Ask();

    ExpectActiveQueryStarted();
    DestructQueryPtr();

    auto aQuestion = AQuestion();
    SendStreamAvailableAndExpectQuestion(aQuestion);
}

TEST_F(MdnsClientTest, other_query_asking_after_canceling_first_starts_active_query)
{
    ConstructAllQueries();

    ExpectActiveQueryStarted();
    queryPtr->Ask();
    queryA->Ask();

    ExpectActiveQueryStarted();
    DestructQueryPtr();

    queryTxt->Ask();

    ExpectActiveQueryStarted();
    auto aQuestion = AQuestion();
    SendStreamAvailableAndExpectQuestion(aQuestion);

    auto txtQuestion = TxtQuestion();
    SendStreamAvailableAndExpectQuestion(txtQuestion);

}

TEST_F(MdnsClientTest, other_query_asking_starts_active_query_after_first_query_is_done_and_asked_again)
{
    ConstructAllQueries();

    ExpectActiveQueryStarted();
    queryPtr->Ask();
    queryA->Ask();

    ExpectActiveQueryStarted();
    auto ptrQuestion = PtrQuestion();
    SendStreamAvailableAndExpectQuestion(ptrQuestion);
    queryPtr->Ask();

    ExpectActiveQueryStarted();
    auto aQuestion = AQuestion();
    SendStreamAvailableAndExpectQuestion(aQuestion);

    SendStreamAvailableAndExpectQuestion(ptrQuestion);
}

TEST_F(MdnsClientTest, nothing_happens_when_receiving_from_incorrect_port)
{
    QueryPtr(queryPtrCallback);
    DataReceived(std::vector<uint8_t>(), services::IPv4Address{ 1, 2, 3, 4 }, 1);
}

TEST_F(MdnsClientTest, nothing_happens_when_receiving_empty_packet)
{
    QueryPtr(queryPtrCallback);
    DataReceived(std::vector<uint8_t>());
}

TEST_F(MdnsClientTest, nothing_happens_when_receiving_non_matching_answer)
{
    QueryPtr(queryPtrCallback);
    AAnswerReceived();
}

TEST_F(MdnsClientTest, destructed_passive_query_is_not_handled)
{
    ConstructAllQueries();
    DestructQueryPtr();
    PtrAnswerReceived();
}

TEST_F(MdnsClientTest, destructed_active_query_is_not_handled)
{
    ConstructAllQueries();

    ExpectActiveQueryStarted();
    queryPtr->Ask();

    DestructQueryPtr();

    PtrAnswerReceived();
}

TEST_F(MdnsClientTest, receiving_matching_answer_to_passive_query_results_in_data)
{
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
}

TEST_F(MdnsClientTest, receiving_matching_answer_to_active_query_results_in_data)
{
    ConstructAllQueriesWithExpectedCallback();

    ExpectActiveQueryStarted();
    queryA->Ask();
    queryAaaa->Ask();
    queryPtr->Ask();
    queryTxt->Ask();
    querySrv->Ask();

    ExpectActiveQueryStarted();
    auto aQuestion = AQuestion();
    SendStreamAvailableAndExpectQuestion(aQuestion);
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeA));
    AAnswerReceived();

    ExpectActiveQueryStarted();
    auto aaaaQuestion = AaaaQuestion();
    SendStreamAvailableAndExpectQuestion(aaaaQuestion);
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeAAAA));
    AaaaAnswerReceived();

    ExpectActiveQueryStarted();
    auto ptrQuestion = PtrQuestion();
    SendStreamAvailableAndExpectQuestion(ptrQuestion);
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypePtr));
    PtrAnswerReceived();

    ExpectActiveQueryStarted();
    auto txtQuestion = TxtQuestion();
    SendStreamAvailableAndExpectQuestion(txtQuestion);
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeTxt));
    TxtAnswerReceived();

    auto srvQuestion = SrvQuestion();
    SendStreamAvailableAndExpectQuestion(srvQuestion);
    EXPECT_CALL(callback, callback(services::DnsType::dnsTypeSrv));
    SrvAnswerReceived();
}

TEST_F(MdnsClientTest, receiving_additional_records_to_passive_query_results_in_data)
{
    ConstructPtrQueryWithExpectedCallbackAndAdditionalRecordsCallback();

    EXPECT_CALL(callback, callback(services::DnsType::dnsTypePtr));
    EXPECT_CALL(additionalRecordsCallback, callback(services::DnsType::dnsTypeA));
    EXPECT_CALL(additionalRecordsCallback, callback(services::DnsType::dnsTypeAAAA));
    EXPECT_CALL(additionalRecordsCallback, callback(services::DnsType::dnsTypeTxt));
    EXPECT_CALL(additionalRecordsCallback, callback(services::DnsType::dnsTypeSrv));
    PtrAnswerReceivedWithAdditionalRecords();
}

TEST_F(MdnsClientTest, receiving_additional_records_to_active_query_results_in_data)
{
    ConstructPtrQueryWithExpectedCallbackAndAdditionalRecordsCallback();

    ExpectActiveQueryStarted();
    queryPtr->Ask();

    auto ptrQuestion = PtrQuestion();
    SendStreamAvailableAndExpectQuestion(ptrQuestion);

    EXPECT_CALL(callback, callback(services::DnsType::dnsTypePtr));
    EXPECT_CALL(additionalRecordsCallback, callback(services::DnsType::dnsTypeA));
    EXPECT_CALL(additionalRecordsCallback, callback(services::DnsType::dnsTypeAAAA));
    EXPECT_CALL(additionalRecordsCallback, callback(services::DnsType::dnsTypeTxt));
    EXPECT_CALL(additionalRecordsCallback, callback(services::DnsType::dnsTypeSrv));
    PtrAnswerReceivedWithAdditionalRecords();
}
