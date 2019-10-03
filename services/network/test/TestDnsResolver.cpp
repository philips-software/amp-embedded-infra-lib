#include "gmock/gmock.h"
#include "hal/synchronous_interfaces/test_doubles/SynchronousRandomDataGeneratorMock.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "infra/util/test_helper/VariantPrintTo.hpp"
#include "infra/util/Tokenizer.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/test/StreamMock.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/network/DnsResolver.hpp"
#include "services/network/test_doubles/DatagramMock.hpp"
#include "services/network/test_doubles/NameResolverMock.hpp"

class DnsResolverTest
    : public testing::TestWithParam<int>
    , public infra::ClockFixture
{
public:
    DnsResolverTest()
    {
        EXPECT_CALL(randomDataGenerator, GenerateRandomData(testing::_)).WillRepeatedly(testing::Invoke([](infra::ByteRange result) { std::fill(result.begin(), result.end(), 9); }));
    }

    auto&& ExpectRequestSendStream(services::NameResolverResultMock& result, infra::BoundedConstString hostname, services::IPAddress dnsServer)
    {
        EXPECT_CALL(result, Hostname()).Times(testing::AnyNumber()).WillRepeatedly(testing::Return(hostname));
        return EXPECT_CALL(datagram, RequestSendStream(18 + hostname.size(), services::MakeUdpSocket(dnsServer, 53)));
    }

    void ExpectAndRespondToRequestSendStream(services::NameResolverResultMock& result, infra::BoundedConstString hostname, services::IPAddress dnsServer)
    {
        ExpectRequestSendStream(result, hostname, dnsServer).WillOnce(testing::Invoke([this, &result, hostname](std::size_t sendSize, services::UdpSocket remote) {
            EXPECT_CALL(writer, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 9, 9, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0 } }), testing::_));

            infra::Tokenizer tokenizer(hostname, '.');
            for (uint32_t i = 0; i != tokenizer.Size(); ++i)
            {
                EXPECT_CALL(writer, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { static_cast<uint8_t>(tokenizer.Token(i).size()) } }), testing::_));
                EXPECT_CALL(writer, Insert(infra::CheckByteRangeContents(tokenizer.Token(i)), testing::_));
            }

            EXPECT_CALL(writer, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 0 } }), testing::_));
            EXPECT_CALL(writer, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 0, 1, 0, 1 } }), testing::_));
            datagramExchangeObserver->SendStreamAvailable(infra::UnOwnedSharedPtr(writer));
        }));
    }

    void Lookup(services::NameResolverResultMock& result)
    {
        resolver.Lookup(result);
    }

    void GiveSendStream(services::NameResolverResultMock& result, infra::BoundedConstString hostname, services::IPAddress dnsServer)
    {
        currentHostname = hostname;
        EXPECT_CALL(datagramFactory, Listen(testing::_, services::IPVersions::both)).WillOnce(testing::Invoke([this](services::DatagramExchangeObserver& observer, services::IPVersions versions)
        {
            datagramExchangeObserver = &observer;
            return infra::UnOwnedSharedPtr(datagram);
        }));
        ExpectAndRespondToRequestSendStream(result, hostname, dnsServer);
    }

    void LookupAndGiveSendStream(services::NameResolverResultMock& result, infra::BoundedConstString hostname, services::IPAddress dnsServer)
    {
        GiveSendStream(result, hostname, dnsServer);
        Lookup(result);
    }

    void LookupIsRetried(services::NameResolverResultMock& result, infra::BoundedConstString hostname, services::IPAddress dnsServer)
    {
        ExpectAndRespondToRequestSendStream(result, currentHostname, dnsServer);
        ForwardTime(std::chrono::seconds(5));
    }

    std::vector<uint8_t> Concatenate(std::initializer_list<const std::vector<uint8_t>> vectors)
    {
        std::vector<uint8_t> result;

        for (auto& vector : vectors)
            result.insert(result.end(), vector.begin(), vector.end());

        return result;
    }

    std::vector<uint8_t> ConvertDns(infra::BoundedConstString hostname)
    {
        std::vector<uint8_t> result;

        infra::Tokenizer tokenizer(hostname, '.');
        for (uint32_t i = 0; i != tokenizer.Size(); ++i)
            result = Concatenate({ result, std::vector<uint8_t>{ { static_cast<uint8_t>(tokenizer.Token(i).size()) } }, std::vector<uint8_t>(tokenizer.Token(i).begin(), tokenizer.Token(i).end()) });
        result.push_back(0);

        return result;
    }

    std::vector<uint8_t> ConvertDns(services::IPv4Address address)
    {
        return std::vector<uint8_t>(address.begin(), address.end());
    }

    std::vector<uint8_t> MakeHeader(uint8_t answers, uint8_t authorativeNameServers = 0, uint8_t additionRecords = 0)
    {
        return std::vector<uint8_t>{ { 9, 9, 0x80, 0, 0, 1, 0, answers, 0, authorativeNameServers, 0, additionRecords } };
    }

    std::vector<uint8_t> MakeQuestion(infra::BoundedConstString hostname)
    {
        std::vector<uint8_t> footer{ { 0, 1, 0, 1 } };
        return Concatenate({ ConvertDns(hostname), footer });
    }

    std::vector<uint8_t> MakeAnswerA(const std::vector<uint8_t>& name, services::IPv4Address answer)
    {
        std::vector<uint8_t> resourceInner{ { 0, 1, 0, 1, 0, 1, 0, 30, 0, 4 } };
        return Concatenate({ name, resourceInner, ConvertDns(answer) });
    }

    std::vector<uint8_t> MakeAnswerCName(const std::vector<uint8_t>& name, infra::BoundedConstString answer)
    {
        std::vector<uint8_t> resourceInner{ { 0, 5, 0, 1, 0, 1, 0, 30, 0, static_cast<uint8_t>(answer.size() + 2) } };
        return Concatenate({ name, resourceInner, ConvertDns(answer) });
    }

    std::vector<uint8_t> MakeAnswerNS(const std::vector<uint8_t>& name, infra::BoundedConstString answer)
    {
        std::vector<uint8_t> resourceInner{ { 0, 2, 0, 1, 0, 1, 0, 30, 0, static_cast<uint8_t>(answer.size() + 2) } };
        return Concatenate({ name, resourceInner, ConvertDns(answer) });
    }

    std::vector<uint8_t> MakeReferenceAnswerA(services::IPv4Address address, uint8_t start = 0x0c)
    {
        std::vector<uint8_t> nameReference{ { 0xc0, start } };
        return MakeAnswerA(nameReference, address);
    }

    std::vector<uint8_t> MakeReferenceAnswerCName(infra::BoundedConstString answer, uint8_t start = 0x0c)
    {
        std::vector<uint8_t> nameReference{ { 0xc0, start } };
        return MakeAnswerCName(nameReference, answer);
    }

    std::vector<uint8_t> MakeReferenceAnswerNS(infra::BoundedConstString answer, uint8_t start = 0x0c)
    {
        std::vector<uint8_t> nameReference{ { 0xc0, start } };
        return MakeAnswerNS(nameReference, answer);
    }

    std::vector<uint8_t> MakeDnsResponse(infra::BoundedConstString hostname, services::IPv4Address address)
    {
        return Concatenate({ MakeHeader(1), MakeQuestion(hostname), MakeReferenceAnswerA(address) });
    }

    std::vector<uint8_t> MakeDnsResponseWithUncompressedHost(infra::BoundedConstString hostname, services::IPv4Address address)
    {
        return Concatenate({ MakeHeader(1), MakeQuestion(hostname), MakeAnswerA(ConvertDns(hostname), address) });
    }

    std::vector<uint8_t> MakeDnsResponseWithCName(infra::BoundedConstString hostname, infra::BoundedConstString alias)
    {
        return Concatenate({ MakeHeader(1), MakeQuestion(hostname), MakeReferenceAnswerCName(alias) });
    }

    std::vector<uint8_t> MakeDnsResponseWithCNameAndAnswer(infra::BoundedConstString hostname, infra::BoundedConstString alias, services::IPv4Address address)
    {
        return Concatenate({ MakeHeader(2), MakeQuestion(hostname), MakeReferenceAnswerCName(alias), MakeReferenceAnswerA(address, 42) });
    }

    std::vector<uint8_t> MakeDnsResponseWithCNameAndAnswerForDifferentHost(infra::BoundedConstString hostname, infra::BoundedConstString alias, services::IPv4Address address)
    {
        auto result = MakeDnsResponseWithCNameAndAnswer(hostname, alias, address);

        result[54] = 0x0c;

        return result;
    }

    std::vector<uint8_t> MakeDnsResponseWithCNameWithInnerReference(infra::BoundedConstString hostname, infra::BoundedConstString alias)
    {
        std::vector<uint8_t>  nameStart(ConvertDns(hostname.substr(0, hostname.find('.'))));
        nameStart.pop_back();
        std::vector<uint8_t> nameWithInnerReference(Concatenate({ nameStart, { 0xc0, 21 } }));
        return Concatenate({ MakeHeader(1), MakeQuestion(hostname), MakeAnswerCName(nameWithInnerReference, alias) });
    }

    std::vector<uint8_t> MakeDnsResponseWithSecondCNameWithInnerReference(infra::BoundedConstString hostname, infra::BoundedConstString alias)
    {
        std::vector<uint8_t>  nameStart(ConvertDns("other"));
        nameStart.pop_back();
        std::vector<uint8_t> nameWithInnerReference(Concatenate({ nameStart, { 0xc0, 21 } }));
        return Concatenate({ MakeHeader(2), MakeQuestion(hostname), MakeAnswerCName(nameWithInnerReference, alias), MakeReferenceAnswerCName(alias) });
    }

    std::vector<uint8_t> MakeDnsResponseWithCNameWithReferenceInAnswer(infra::BoundedConstString hostname, infra::BoundedConstString alias)
    {
        std::vector<uint8_t> header(MakeHeader(1));
        std::vector<uint8_t> question(MakeQuestion(hostname));
        std::vector<uint8_t> nameReference{ { 0xc0, 0x0c } };
        std::vector<uint8_t> resourceInner{ { 0, 5, 0, 1, 0, 1, 0, 30, 0, static_cast<uint8_t>(alias.size() + 2) } };

        resourceInner.back() -= static_cast<uint8_t>(alias.size() - alias.rfind('.') - 1);
        std::vector<uint8_t> aliasData(ConvertDns(alias));
        aliasData.erase(aliasData.begin() + alias.rfind('.') + 1, aliasData.end());
        aliasData.push_back(0xc0);
        aliasData.push_back(static_cast<uint8_t>(13 + hostname.rfind('.')));

        return Concatenate({ header, question, nameReference, resourceInner, aliasData });
    }

    std::vector<uint8_t> MakeDnsResponseWithNS(infra::BoundedConstString hostname, infra::BoundedConstString ns, services::IPv4Address address)
    {
        return Concatenate({ MakeHeader(0, 1, 1), MakeQuestion(hostname), MakeReferenceAnswerNS(ns), MakeReferenceAnswerA(address, 42) });
    }

    std::vector<uint8_t> MakeDnsResponseWithNSTooShort(infra::BoundedConstString hostname, infra::BoundedConstString ns, services::IPv4Address address)
    {
        auto result = MakeDnsResponseWithNS(hostname, ns, address);
        result.pop_back();
        return result;
    }

    std::vector<uint8_t> MakeDnsResponseWithAdditional(infra::BoundedConstString hostname, infra::BoundedConstString ns, services::IPv4Address address)
    {
        return Concatenate({ MakeHeader(0, 0, 1), MakeQuestion(hostname), MakeAnswerA(ConvertDns(ns), address) });
    }

    std::vector<uint8_t> MakeShortenedDnsResponse(infra::BoundedConstString hostname, services::IPv4Address address, uint32_t amountToShort)
    {
        auto result = MakeDnsResponse(hostname, address);
        result.erase(result.end() - amountToShort, result.end());
        return result;
    }

    std::vector<uint8_t> MakeDnsQuestion(infra::BoundedConstString hostname, services::IPv4Address address)
    {
        auto result = MakeDnsResponse(hostname, address);
        result[2] = 0;
        return result;
    }

    std::vector<uint8_t> MakeDnsResponseWithSecondQuestion(infra::BoundedConstString hostname, services::IPv4Address address)
    {
        auto result = MakeDnsResponse(hostname, address);
        result[5] = 2;
        return result;
    }

    std::vector<uint8_t> MakeDnsResponseWithDifferentId(infra::BoundedConstString hostname, services::IPv4Address address)
    {
        auto result = MakeDnsResponse(hostname, address);
        result[0] = 5;
        return result;
    }

    std::vector<uint8_t> MakeDnsResponseWithDifferentType(infra::BoundedConstString hostname, services::IPv4Address address)
    {
        auto result = MakeDnsResponse(hostname, address);
        result[27] = 5;
        return result;
    }

    std::vector<uint8_t> MakeDnsResponseWithDifferentClass(infra::BoundedConstString hostname, services::IPv4Address address)
    {
        auto result = MakeDnsResponse(hostname, address);
        result[29] = 5;
        return result;
    }

    std::vector<uint8_t> MakeDnsResponseWithError(infra::BoundedConstString hostname, services::IPv4Address address)
    {
        auto result = MakeDnsResponse(hostname, address);
        result[3] = 1;
        return result;
    }

    std::vector<uint8_t> MakeDnsResponseWithType2(infra::BoundedConstString hostname, services::IPv4Address address)
    {
        auto result = MakeDnsResponse(hostname, address);
        result[33] = 2;
        return result;
    }

    std::vector<uint8_t> MakeDnsResponseWithClass2(infra::BoundedConstString hostname, services::IPv4Address address)
    {
        auto result = MakeDnsResponse(hostname, address);
        result[35] = 2;
        return result;
    }

    std::vector<uint8_t> MakeDnsResponseWithIncorrectReference(infra::BoundedConstString hostname, services::IPv4Address address)
    {
        auto result = MakeDnsResponse(hostname, address);
        result[17] = 2;
        return result;
    }

    std::vector<uint8_t> MakeDnsResponseWithSecondAnswer(infra::BoundedConstString hostname, services::IPv4Address address)
    {
        auto result = MakeDnsResponseWithClass2(hostname, address);

        result[7] = 2;  // 2 answers

        std::vector<uint8_t> nameReference{ { 0xc0, 0x0c } };
        std::vector<uint8_t> resourceInner{ { 0, 1, 0, 1, 0, 1, 0, 30, 0, 4 } };

        return Concatenate({ result, nameReference, resourceInner, ConvertDns(address) });
    }

    void DataReceived(const std::vector<uint8_t>& response, services::UdpSocket from)
    {
        infra::StdVectorInputStream::WithStorage stream(infra::inPlace, response);
        datagramExchangeObserver->DataReceived(stream.Reader(), from);
    }

    const services::IPv4Address dnsServer1{ { 1, 2, 3, 4 } };
    const services::IPv4Address dnsServer2{ { 2, 3, 4, 5 } };
    const services::IPv4Address hostAddress1{ { 3, 4, 5, 6 } };
    const services::IPv4Address nsServer1{ { 4, 5, 6, 7 } };

    testing::StrictMock<services::DatagramFactoryMock> datagramFactory;
    const std::array<services::IPAddress, 2> dnsServers{ { dnsServer1, dnsServer2 } };
    testing::StrictMock<hal::SynchronousRandomDataGeneratorMock> randomDataGenerator;
    services::DnsResolver resolver{ datagramFactory, { dnsServers }, randomDataGenerator };
    testing::StrictMock<services::NameResolverResultMock> result1;
    testing::StrictMock<services::NameResolverResultMock> result2;
    testing::StrictMock<services::DatagramExchangeMock> datagram;
    services::DatagramExchangeObserver* datagramExchangeObserver = nullptr;
    testing::StrictMock<infra::StreamWriterMock> writer;
    infra::BoundedConstString currentHostname;
    infra::TimePoint expiration = infra::Now() + std::chrono::seconds(0x10000) + std::chrono::seconds(30);
};

TEST_F(DnsResolverTest, Lookup_starts_dns_lookup)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);
}

TEST_F(DnsResolverTest, other_Lookup_starts_dns_lookup)
{
    LookupAndGiveSendStream(result1, "some.other.com", dnsServer2);
}

TEST_F(DnsResolverTest, after_timeout_Lookup_is_retried)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);
    LookupIsRetried(result1, "hostname.com", dnsServer1);
}

TEST_F(DnsResolverTest, CancelLookup_stops_retrying)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);
    resolver.CancelLookup(result1);
    ForwardTime(std::chrono::seconds(5));
}

TEST_F(DnsResolverTest, after_3_attempts_Lookup_fails)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);
    LookupIsRetried(result1, "hostname.com", dnsServer1);
    LookupIsRetried(result1, "hostname.com", dnsServer2);

    EXPECT_CALL(result1, NameLookupFailed());
    ForwardTime(std::chrono::seconds(5));
}

TEST_F(DnsResolverTest, new_lookup_is_started_after_previous_lookup_fails)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);
    Lookup(result2);
    LookupIsRetried(result1, "hostname.com", dnsServer1);
    LookupIsRetried(result1, "hostname.com", dnsServer2);

    EXPECT_CALL(result1, NameLookupFailed());
    GiveSendStream(result2, "second.com", dnsServer1);
    ForwardTime(std::chrono::seconds(5));
}

TEST_F(DnsResolverTest, new_lookup_is_started_after_previous_lookup_is_cancelled)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);
    Lookup(result2);
    LookupIsRetried(result1, "hostname.com", dnsServer1);
    LookupIsRetried(result1, "hostname.com", dnsServer2);

    GiveSendStream(result2, "second.com", dnsServer1);
    resolver.CancelLookup(result1);
}

TEST_F(DnsResolverTest, new_lookup_is_not_started_if_it_is_cancelled)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);
    Lookup(result2);
    resolver.CancelLookup(result2);
    LookupIsRetried(result1, "hostname.com", dnsServer1);
    LookupIsRetried(result1, "hostname.com", dnsServer2);

    EXPECT_CALL(result1, NameLookupFailed());
    ForwardTime(std::chrono::seconds(5));
}

TEST_F(DnsResolverTest, response_results_in_Successful)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    EXPECT_CALL(result1, NameLookupDone(services::IPAddress(hostAddress1), expiration));
    DataReceived(MakeDnsResponse("hostname.com", hostAddress1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, response_with_uncompressed_host_in_answer_results_in_Successful)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    EXPECT_CALL(result1, NameLookupDone(services::IPAddress(hostAddress1), expiration));
    DataReceived(MakeDnsResponseWithUncompressedHost("hostname.com", hostAddress1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, response_for_other_host_is_ignored)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    DataReceived(MakeDnsResponse("other.com", hostAddress1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, response_from_different_server_is_ignored)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    DataReceived(MakeDnsResponse("hostname.com", hostAddress1), services::Udpv4Socket{ dnsServer1, 53 });
}

TEST_F(DnsResolverTest, error_in_response_results_in_retry)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    ExpectRequestSendStream(result1, "hostname.com", dnsServer1);
    DataReceived(MakeDnsResponseWithError("hostname.com", hostAddress1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, wrong_type_in_response_results_in_retry)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    ExpectRequestSendStream(result1, "hostname.com", dnsServer1);
    DataReceived(MakeDnsResponseWithType2("hostname.com", hostAddress1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, wrong_class_in_response_results_in_retry)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    ExpectRequestSendStream(result1, "hostname.com", dnsServer1);
    DataReceived(MakeDnsResponseWithClass2("hostname.com", hostAddress1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, response_in_second_answer_results_in_Successful)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    EXPECT_CALL(result1, NameLookupDone(services::IPAddress(hostAddress1), expiration));
    DataReceived(MakeDnsResponseWithSecondAnswer("hostname.com", hostAddress1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, response_as_question_is_ignored)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    DataReceived(MakeDnsQuestion("hostname.com", hostAddress1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, response_with_2_questions_is_ignored)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    DataReceived(MakeDnsResponseWithSecondQuestion("hostname.com", hostAddress1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, response_with_different_id_is_ignored)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    DataReceived(MakeDnsResponseWithDifferentId("hostname.com", hostAddress1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, response_with_different_type_is_ignored)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    DataReceived(MakeDnsResponseWithDifferentType("hostname.com", hostAddress1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, response_with_different_class_is_ignored)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    DataReceived(MakeDnsResponseWithDifferentClass("hostname.com", hostAddress1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, response_with_incorrect_reference_is_ignored)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    DataReceived(MakeDnsResponseWithIncorrectReference("hostname.com", hostAddress1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, cname_results_in_retry)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    ExpectAndRespondToRequestSendStream(result1, "cname.com", dnsServer2);
    DataReceived(MakeDnsResponseWithCName("hostname.com", "cname.com"), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, cname_with_answer_results_in_success)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    EXPECT_CALL(result1, NameLookupDone(services::IPAddress(hostAddress1), expiration));
    DataReceived(MakeDnsResponseWithCNameAndAnswer("hostname.com", "cname.com", hostAddress1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, cname_with_inner_reference_results_in_retry)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    ExpectAndRespondToRequestSendStream(result1, "cname.com", dnsServer2);
    DataReceived(MakeDnsResponseWithCNameWithInnerReference("hostname.com", "cname.com"), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, second_cname_with_inner_reference_results_in_retry)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    ExpectAndRespondToRequestSendStream(result1, "cname.com", dnsServer2);
    DataReceived(MakeDnsResponseWithSecondCNameWithInnerReference("hostname.com", "cname.com"), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, cname_with_answer_for_different_host_results_in_retry)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    ExpectAndRespondToRequestSendStream(result1, "cname.com", dnsServer2);
    DataReceived(MakeDnsResponseWithCNameAndAnswerForDifferentHost("hostname.com", "cname.com", hostAddress1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, cname_with_reference_in_answer_results_in_retry)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    ExpectAndRespondToRequestSendStream(result1, "cname.com", dnsServer2);
    DataReceived(MakeDnsResponseWithCNameWithReferenceInAnswer("hostname.com", "cname.com"), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, cname_with_reference_loop_stops_after_5_attempts)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    for (int i = 0; i != 4; ++i)
    {
        ExpectAndRespondToRequestSendStream(result1, "hostname.com", dnsServer2);
        DataReceived(MakeDnsResponseWithCNameWithReferenceInAnswer("hostname.com", "hostname.com"), services::Udpv4Socket{ dnsServer2, 53 });
    }

    EXPECT_CALL(result1, NameLookupFailed());
    DataReceived(MakeDnsResponseWithCNameWithReferenceInAnswer("hostname.com", "hostname.com"), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, ns_results_in_recursion)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    ExpectAndRespondToRequestSendStream(result1, "hostname.com", nsServer1);
    DataReceived(MakeDnsResponseWithNS("hostname.com", "ns.com", nsServer1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, additional_record_which_is_not_ns_results_in_retry)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    ExpectAndRespondToRequestSendStream(result1, "hostname.com", dnsServer1);
    DataReceived(MakeDnsResponseWithAdditional("hostname.com", "ns.com", nsServer1), services::Udpv4Socket{ dnsServer2, 53 });
}

TEST_F(DnsResolverTest, too_short_ns_results_in_retry)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    ExpectAndRespondToRequestSendStream(result1, "hostname.com", dnsServer1);
    DataReceived(MakeDnsResponseWithNSTooShort("hostname.com", "ns.com", nsServer1), services::Udpv4Socket{ dnsServer2, 53 });
}

class DnsResolverTestTooShort
    : public DnsResolverTest
{};

TEST_P(DnsResolverTestTooShort, too_short_response_results_in_retry)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    ExpectRequestSendStream(result1, "hostname.com", dnsServer1);
    DataReceived(MakeShortenedDnsResponse("hostname.com", hostAddress1, GetParam()), services::Udpv4Socket{ dnsServer2, 53 });
}

INSTANTIATE_TEST_CASE_P(too_short_response_results_in_retry, DnsResolverTestTooShort, testing::Range(1, 17));

class DnsResolverTestEvenShorter
    : public DnsResolverTest
{};

TEST_P(DnsResolverTestEvenShorter, too_short_response_is_ignored)
{
    LookupAndGiveSendStream(result1, "hostname.com", dnsServer2);

    DataReceived(MakeShortenedDnsResponse("hostname.com", hostAddress1, GetParam()), services::Udpv4Socket{ dnsServer2, 53 });
}

INSTANTIATE_TEST_CASE_P(too_short_response_is_ignored, DnsResolverTestEvenShorter, testing::Range(17, 47));
