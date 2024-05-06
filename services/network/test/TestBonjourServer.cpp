#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/ConstructBin.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/BonjourServer.hpp"
#include "services/network/test_doubles/DatagramMock.hpp"
#include "services/network/test_doubles/MulticastMock.hpp"
#include "gmock/gmock.h"

namespace
{
    const services::IPAddress ipv4Source{ services::IPv4Address{ 224, 0, 0, 251 } };
    const services::IPAddress ipv6Source{ services::IPv6Address{ 0xff02, 0, 0, 0, 0, 0, 0, 0xfb } };
}

class BonjourServerTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    ~BonjourServerTest() override
    {
        expectLeave();
    }

    void DataReceived(const std::vector<uint8_t>& data, services::IPv4Address address = services::IPv4Address{ 1, 2, 3, 4 }, uint16_t port = 5353)
    {
        datagramExchange->GetObserver().DataReceived(infra::MakeSharedOnHeap<infra::StdVectorInputStreamReader::WithStorage>(std::in_place, data), services::Udpv4Socket{ address, port });
    }

    void DataReceived(const std::vector<uint8_t>& data, services::IPv6Address address, uint16_t port = 5353)
    {
        datagramExchange->GetObserver().DataReceived(infra::MakeSharedOnHeap<infra::StdVectorInputStreamReader::WithStorage>(std::in_place, data), services::Udpv6Socket{ address, port });
    }

    void DataReceived(const std::vector<uint8_t>& data, services::IPAddress address, uint16_t port = 5353)
    {
        if (address.Is<services::IPv4Address>())
            DataReceived(data, address.Get<services::IPv4Address>(), port);
        else
            DataReceived(data, address.Get<services::IPv6Address>(), port);
    }

    void PacketReceived(services::IPv4Address address = services::IPv4Address{ 1, 2, 3, 4 }, uint16_t port = 5353)
    {
        DataReceived(std::vector<uint8_t>(), address, port);
    }

    void EmptyPacketReceived()
    {
        DataReceived(std::vector<uint8_t>());
    }

    void PtrQueryReceived(services::IPAddress source = ipv4Source, uint16_t answers = 0, uint16_t nameServers = 0, uint16_t additional = 0)
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 1, answers, nameServers, additional })(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn })
                         .Vector(),
            source);
    }

    void PtrQueryWithTwoQuestionsReceived()
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 2, 0, 0, 0 })(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn })(5)("other")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn })
                         .Vector(),
            ipv4Source);
    }

    void PtrQueryWithTwoEqualQuestionsReceived()
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 2, 0, 0, 0 })(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn })(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn })
                         .Vector(),
            ipv4Source);
    }

    void TooShortQueryReceived()
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 1, 0, 0, 0 })(7)("service")(4)("type")(5)("local")(0)
                         .Vector());
    }

    void QueryWithAnswerReceived()
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 1, 1, 0, 0 })(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn })(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn })
                         .Vector(),
            ipv4Source);
    }

    void QueryWithNameServerReceived()
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 1, 0, 1, 0 })(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn })(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn })
                         .Vector(),
            ipv4Source);
    }

    void QueryWithAdditionalReceived()
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 1, 0, 0, 1 })(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn })(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn })
                         .Vector(),
            ipv4Source);
    }

    void AnswerReceived()
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, services::DnsRecordHeader::flagsResponse, 1, 0, 0, 0 })(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn })
                         .Vector());
    }

    void DifferentOpcodeReceived()
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, services::DnsRecordHeader::flagsOpcodeMask, 1, 0, 0, 0 })(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn })
                         .Vector());
    }

    void NonInQueryReceived()
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 1, 0, 0, 0 })(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassCh })
                         .Vector());
    }

    void CNameQueryReceived()
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 1, 0, 0, 0 })(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypeCName, services::DnsClass::dnsClassIn })
                         .Vector());
    }

    void QueryForDifferentServiceReceived()
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 1, 0, 0, 0 })(7)("service")(4)("aaaa")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn })
                         .Vector());
    }

    void QueryForDifferentInstanceReceived()
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 1, 0, 0, 0 })(8)("aaaaaaaa")(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypeSrv, services::DnsClass::dnsClassIn })
                         .Vector());
    }

    void QueryForDifferentShortInstanceReceived()
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 1, 0, 0, 0 })(8)("aaaaaaaa")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypeA, services::DnsClass::dnsClassIn })
                         .Vector());
    }

    void QueryForEmptyNameReceived()
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 1, 0, 0, 0 })(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypeSrv, services::DnsClass::dnsClassIn })
                         .Vector());
    }

    void QueryForLongNameReceived()
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 1, 0, 0, 0 })(3)("bla")(8)("instance")(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypeSrv, services::DnsClass::dnsClassIn })
                         .Vector());
    }

    void SrvQueryReceived(services::IPAddress source = ipv4Source)
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 1, 0, 0, 0 })(8)("instance")(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypeSrv, services::DnsClass::dnsClassIn })
                         .Vector(),
            source);
    }

    void TxtQueryReceived(services::IPAddress source = ipv4Source)
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 1, 0, 0, 0 })(8)("instance")(7)("service")(4)("type")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypeTxt, services::DnsClass::dnsClassIn })
                         .Vector(),
            source);
    }

    void AQueryReceived(services::IPAddress source = ipv4Source)
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 1, 0, 0, 0 })(8)("instance")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypeA, services::DnsClass::dnsClassIn })
                         .Vector(),
            source);
    }

    void AaaaQueryReceived(services::IPAddress source = ipv4Source)
    {
        DataReceived(infra::ConstructBin()
                         .Value<services::DnsRecordHeader>({ 0x0200, 0, 1, 0, 0, 0 })(8)("instance")(5)("local")(0)
                         .Value<services::DnsQuestionFooter>({ services::DnsType::dnsTypeAAAA, services::DnsClass::dnsClassIn })
                         .Vector(),
            source);
    }

    void ExpectResponse(const std::vector<uint8_t>& data)
    {
        struct Ptr
        {
            BonjourServerTest& self;
            std::vector<uint8_t> data;
        };

        auto ptr = std::make_shared<Ptr>(Ptr{ *this, data });

        EXPECT_CALL(*datagramExchange, RequestSendStream(data.size(), testing::_)).WillOnce(testing::Invoke([ptr](std::size_t sendSize, services::UdpSocket to)
            {
                infra::EventDispatcher::Instance().Schedule([ptr]()
                    {
                        infra::StdVectorOutputStreamWriter::WithStorage response;
                        ptr->self.datagramExchange->GetObserver().SendStreamAvailable(infra::UnOwnedSharedPtr(response));
                        EXPECT_EQ(ptr->data, response.Storage());
                    });
            }));
    }

    std::vector<uint8_t> PtrAnswer()
    {
        return infra::ConstructBin()(7)("service")(4)("type")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypePtr, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 0x1d })(8)("instance")(7)("service")(4)("type")(5)("local")(0)
            .Vector();
    }

    std::vector<uint8_t> TxtAnswer()
    {
        return infra::ConstructBin()(8)("instance")(7)("service")(4)("type")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeTxt, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 0x15 })(7)("aa=text")(12)("bb=othertext")
            .Vector();
    }

    std::vector<uint8_t> SrvAnswer()
    {
        return infra::ConstructBin()(8)("instance")(7)("service")(4)("type")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeSrv, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 0x16 })
            .Value<infra::BigEndian<uint16_t>>(0)
            .Value<infra::BigEndian<uint16_t>>(0)
            .Value<infra::BigEndian<uint16_t>>(1234)(8)("instance")(5)("local")(0)
            .Vector();
    }

    std::vector<uint8_t> AAnswer()
    {
        return infra::ConstructBin()(8)("instance")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeA, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 4 })
            .Value<services::IPv4Address>({ 1, 2, 3, 4 })
            .Vector();
    }

    std::vector<uint8_t> NoAAnswer()
    {
        return infra::ConstructBin()(8)("instance")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeNsec, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 22 })(8)("instance")(5)("local")(0)(0)(4)(0)(0)(0)(8)
            .Vector();
    }

    std::vector<uint8_t> AaaaAnswer()
    {
        return infra::ConstructBin()(8)("instance")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeAAAA, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 16 })
            .Value<services::IPv6AddressNetworkOrder>({ 1, 2, 3, 4, 5, 6, 7, 8 })
            .Vector();
    }

    std::vector<uint8_t> NoAaaaAnswer()
    {
        return infra::ConstructBin()(8)("instance")(5)("local")(0)
            .Value<services::DnsRecordPayload>({ services::DnsType::dnsTypeNsec, services::DnsClass::dnsClassIn, std::chrono::seconds(5), 22 })(8)("instance")(5)("local")(0)(0)(4)(0x40)(0)(0)(0)
            .Vector();
    }

    void ExpectListenIpv4()
    {
        EXPECT_CALL(factory, Listen(testing::Ref(server), 5353, services::IPVersions::ipv4)).WillOnce(testing::Invoke([this](services::DatagramExchangeObserver& observer, uint16_t port, services::IPVersions versions)
            {
                auto ptr = datagramExchange.emplace();
                observer.Attach(*ptr);

                ExpectResponse(infra::ConstructBin()
                                   .Value<services::DnsRecordHeader>({ 0x0000, 0x8000, 0, 5, 0, 0 })(PtrAnswer())(SrvAnswer())(TxtAnswer())(AAnswer())(NoAaaaAnswer())
                                   .Vector());

                return ptr;
            }));
    }

    void ExpectListenIpv6()
    {
        EXPECT_CALL(factory, Listen(testing::Ref(server), 5353, services::IPVersions::ipv6)).WillOnce(testing::Invoke([this](services::DatagramExchangeObserver& observer, uint16_t port, services::IPVersions versions)
            {
                auto ptr = datagramExchange.emplace();
                observer.Attach(*ptr);

                ExpectResponse(infra::ConstructBin()
                                   .Value<services::DnsRecordHeader>({ 0x0000, 0x8000, 0, 5, 0, 0 })(PtrAnswer())(SrvAnswer())(TxtAnswer())(NoAAnswer())(AaaaAnswer())
                                   .Vector());

                return ptr;
            }));
    }

    void ExpectJoinMulticastIpv4()
    {
        EXPECT_CALL(multicast, JoinMulticastGroup(testing::_, services::IPv4Address{ 224, 0, 0, 251 }));
    }

    void ExpectJoinMulticastIpv6()
    {
        EXPECT_CALL(multicast, JoinMulticastGroup(testing::_, services::IPv6Address{ 0xff02, 0, 0, 0, 0, 0, 0, 0xfb }));
    }

    void ExpectLeaveMulticastIpv4()
    {
        EXPECT_CALL(multicast, LeaveMulticastGroup(testing::_, services::IPv4Address{ 224, 0, 0, 251 }));
    }

    void ExpectLeaveMulticastIpv6()
    {
        EXPECT_CALL(multicast, LeaveMulticastGroup(testing::_, services::IPv6Address{ 0xff02, 0, 0, 0, 0, 0, 0, 0xfb }));
    }

    void ReConstructIPv6()
    {
        ExpectLeaveMulticastIpv4();
        ExpectListenIpv6();
        ExpectJoinMulticastIpv6();
        infra::ReConstruct(server, factory, multicast, "instance", "service", "type", std::nullopt, std::make_optional(services::IPv6Address{ 1, 2, 3, 4, 5, 6, 7, 8 }), 1234, text);
        ExecuteAllActions();
        expectLeave = [this]()
        {
            ExpectLeaveMulticastIpv6();
        };
    }

    testing::StrictMock<services::MulticastMock> multicast;
    testing::StrictMock<services::DatagramFactoryMock> factory;
    infra::SharedOptional<testing::StrictMock<services::DatagramExchangeMock>> datagramExchange;
    infra::Execute execute{ [this]
        {
            ExpectListenIpv4();
            ExpectJoinMulticastIpv4();
        } };
    services::DnsHostnameInPartsHelper<2> text{ services::DnsHostnameInParts("aa=text")("bb=othertext") };
    services::BonjourServer server{ factory, multicast, "instance", "service", "type", std::make_optional(services::IPv4Address{ 1, 2, 3, 4 }), std::nullopt, 1234, text };

    infra::Execute execute2{ [this]
        {
            ExecuteAllActions();
        } };

    infra::Function<void()> expectLeave{ [this]()
        {
            ExpectLeaveMulticastIpv4();
        } };
};

TEST_F(BonjourServerTest, nothing_happens_when_receiving_from_incorrect_port)
{
    PacketReceived(services::IPv4Address{ 1, 2, 3, 4 }, 1);
}

TEST_F(BonjourServerTest, nothing_happens_when_receiving_empty_packet)
{
    EmptyPacketReceived();
}

TEST_F(BonjourServerTest, ptr_question_has_answers_and_additional_data)
{
    ExpectResponse(infra::ConstructBin()
                       .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 4 })(PtrAnswer())(TxtAnswer())(SrvAnswer())(AAnswer())(NoAaaaAnswer())
                       .Vector());

    PtrQueryReceived();
    ExecuteAllActions();
}

TEST_F(BonjourServerTest, ptr_question_with_two_questions_has_answers_and_additional_data)
{
    ExpectResponse(infra::ConstructBin()
                       .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 4 })(PtrAnswer())(TxtAnswer())(SrvAnswer())(AAnswer())(NoAaaaAnswer())
                       .Vector());

    PtrQueryWithTwoQuestionsReceived();
    ExecuteAllActions();
}

TEST_F(BonjourServerTest, ptr_question_with_two_equal_questions_has_answers_and_additional_data)
{
    ExpectResponse(infra::ConstructBin()
                       .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 2, 0, 8 })(PtrAnswer())(PtrAnswer())(TxtAnswer())(SrvAnswer())(AAnswer())(NoAaaaAnswer())(TxtAnswer())(SrvAnswer())(AAnswer())(NoAaaaAnswer())
                       .Vector());

    PtrQueryWithTwoEqualQuestionsReceived();
    ExecuteAllActions();
}

TEST_F(BonjourServerTest, srv_question_has_answers_and_additional_data)
{
    ExpectResponse(infra::ConstructBin()
                       .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 2 })(SrvAnswer())(AAnswer())(NoAaaaAnswer())
                       .Vector());

    SrvQueryReceived();
    ExecuteAllActions();
}

TEST_F(BonjourServerTest, txt_question_has_answers)
{
    ExpectResponse(infra::ConstructBin()
                       .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 0 })(TxtAnswer())
                       .Vector());

    TxtQueryReceived();
    ExecuteAllActions();
}

TEST_F(BonjourServerTest, a_question_has_answers)
{
    ExpectResponse(infra::ConstructBin()
                       .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 0 })(AAnswer())
                       .Vector());

    AQueryReceived();
    ExecuteAllActions();
}

TEST_F(BonjourServerTest, second_question_while_first_is_busy_is_ignored)
{
    ExpectResponse(infra::ConstructBin()
                       .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 4 })(PtrAnswer())(TxtAnswer())(SrvAnswer())(AAnswer())(NoAaaaAnswer())
                       .Vector());

    PtrQueryReceived();
    PtrQueryReceived();
    ExecuteAllActions();
}

TEST_F(BonjourServerTest, aaaa_query_is_declined_when_no_ipv6_address_is_available)
{
    ExpectResponse(infra::ConstructBin()
                       .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 0 })(NoAaaaAnswer())
                       .Vector());

    AaaaQueryReceived();
}

TEST_F(BonjourServerTest, invalid_questions_are_ignored)
{
    TooShortQueryReceived();
    AnswerReceived();
    DifferentOpcodeReceived();
    NonInQueryReceived();
    CNameQueryReceived();
    QueryForDifferentServiceReceived();
    QueryForDifferentInstanceReceived();
    QueryForDifferentShortInstanceReceived();
    QueryForEmptyNameReceived();
    QueryForLongNameReceived();
}

TEST_F(BonjourServerTest, answer_in_query_is_ignored)
{
    ExpectResponse(infra::ConstructBin()
                       .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 4 })(PtrAnswer())(TxtAnswer())(SrvAnswer())(AAnswer())(NoAaaaAnswer())
                       .Vector());

    QueryWithAnswerReceived();
    ExecuteAllActions();
}

TEST_F(BonjourServerTest, name_server_in_query_is_ignored)
{
    ExpectResponse(infra::ConstructBin()
                       .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 4 })(PtrAnswer())(TxtAnswer())(SrvAnswer())(AAnswer())(NoAaaaAnswer())
                       .Vector());

    QueryWithNameServerReceived();
    ExecuteAllActions();
}

TEST_F(BonjourServerTest, additional_record_in_query_is_ignored)
{
    ExpectResponse(infra::ConstructBin()
                       .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 4 })(PtrAnswer())(TxtAnswer())(SrvAnswer())(AAnswer())(NoAaaaAnswer())
                       .Vector());

    QueryWithAdditionalReceived();
    ExecuteAllActions();
}

TEST_F(BonjourServerTest, aaaa_query_is_answered_when_ipv6_address_is_available)
{
    ReConstructIPv6();

    ExpectResponse(infra::ConstructBin()
                       .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 0 })(AaaaAnswer())
                       .Vector());

    AaaaQueryReceived(ipv6Source);
    ExecuteAllActions();
}

TEST_F(BonjourServerTest, a_query_is_declined_when_no_ipv4_address_is_available)
{
    ReConstructIPv6();

    ExpectResponse(infra::ConstructBin()
                       .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 0 })(NoAAnswer())
                       .Vector());

    AQueryReceived(ipv6Source);
}

TEST_F(BonjourServerTest, ptr_query_is_answered_with_ipv6_address_when_ipv6_address_is_available)
{
    ReConstructIPv6();

    ExpectResponse(infra::ConstructBin()
                       .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 4 })(PtrAnswer())(TxtAnswer())(SrvAnswer())(NoAAnswer())(AaaaAnswer())
                       .Vector());

    PtrQueryReceived(ipv6Source);
}

TEST_F(BonjourServerTest, srv_query_is_answered_with_ipv6_address_when_ipv6_address_is_available)
{
    ReConstructIPv6();

    ExpectResponse(infra::ConstructBin()
                       .Value<services::DnsRecordHeader>({ 0x0200, 0x8000, 0, 1, 0, 2 })(SrvAnswer())(NoAAnswer())(AaaaAnswer())
                       .Vector());

    SrvQueryReceived(ipv6Source);
}
