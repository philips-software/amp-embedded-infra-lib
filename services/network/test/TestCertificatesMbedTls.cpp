#include "gmock/gmock.h"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "services/network/CertificatesMbedTls.hpp"
#include "services/network/test_doubles/Certificates.hpp"

bool operator==(const mbedtls_x509_time& lhs, const mbedtls_x509_time& rhs)
{
    return lhs.year == rhs.year &&
           lhs.mon == rhs.mon &&
           lhs.day == rhs.day &&
           lhs.hour == rhs.hour &&
           lhs.min == rhs.min &&
           lhs.sec == rhs.sec;
}

class CertificatesMbedTlsWithVerify
    : public services::CertificatesMbedTls
{
public:
    void VerifyCertificate()
    {
        uint32_t flags = 0;

        EXPECT_EQ(0, mbedtls_x509_crt_verify(&ownCertificate, &caCertificates, nullptr, "es.philips.local", &flags, nullptr, nullptr));

        if (flags != 0)
        {
            const size_t size = 2048;
            char info[size] = {};

            mbedtls_x509_crt_verify_info(info, size, "", flags);
            std::cerr << info;
        }

        EXPECT_EQ(0, flags);
    }

    void CompareDate(const mbedtls_x509_time& date)
    {
        EXPECT_EQ(date, ownCertificate.valid_to);
    }
};

bool CompareStringIgnoreNewline(const std::string& expected, const std::string& actual, std::string ignore = "\r\n")
{
    size_t expectedIndex = 0;
    size_t actualIndex = 0;

    while (true)
    {
        while (expectedIndex < expected.size() && ignore.find_first_of(expected[expectedIndex]) != std::string::npos)
            expectedIndex++;
        while (actualIndex < actual.size() && ignore.find_first_of(actual[actualIndex]) != std::string::npos)
            actualIndex++;

        if (expectedIndex >= expected.size())
            return actualIndex >= actual.size();
        if (actualIndex >= actual.size())
            return false;

        if (expected[expectedIndex++] != actual[actualIndex++])
            return false;
    }

    return true;
}

class CertificatesMbedTlsTest
    : public testing::Test
{
public:
    CertificatesMbedTlsTest()
    {
        certificates.AddCertificateAuthority(services::testCaCertificate);
        certificates.AddOwnCertificate(services::testServerCertificate, services::testServerKey, randomDataGenerator);
    }

    CertificatesMbedTlsWithVerify certificates;
    hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
};

TEST_F(CertificatesMbedTlsTest, write_private_key)
{
    infra::BoundedString::WithStorage<2048> privateKey;
    certificates.WritePrivateKey(privateKey);

    EXPECT_TRUE(CompareStringIgnoreNewline(services::testServerKey.data(), privateKey.data()));
}

TEST_F(CertificatesMbedTlsTest, generate_new_key)
{
    certificates.GenerateNewKey(randomDataGenerator);

    infra::BoundedString::WithStorage<2048> privateKey;
    certificates.WritePrivateKey(privateKey);

    EXPECT_FALSE(CompareStringIgnoreNewline(services::testServerKey.data(), privateKey.data()));
}

TEST_F(CertificatesMbedTlsTest, write_certificate)
{
    infra::BoundedString::WithStorage<2048> ownCertificate;
    certificates.WriteOwnCertificate(ownCertificate, randomDataGenerator);

    certificates.VerifyCertificate();
}

TEST_F(CertificatesMbedTlsTest, update_valid_to_date)
{
    certificates.UpdateValidToDate();

    infra::BoundedString::WithStorage<2048> ownCertificate;
    certificates.WriteOwnCertificate(ownCertificate, randomDataGenerator);

    certificates.CompareDate({ 9999, 12, 31, 23, 59, 59 });
}

TEST_F(CertificatesMbedTlsTest, sign_certificate_with_new_key)
{
    infra::BoundedString::WithStorage<2048> privateKey;
    certificates.GenerateNewKey(randomDataGenerator);
    certificates.WritePrivateKey(privateKey);

    infra::BoundedString::WithStorage<2048> ownCertificate;
    certificates.WriteOwnCertificate(ownCertificate, randomDataGenerator);

    certificates.VerifyCertificate();
}
