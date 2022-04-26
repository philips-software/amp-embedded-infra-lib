#include "gmock/gmock.h"
#include "services/network/CertificateConvertor.hpp"

TEST(CertificateConvertorTest, should_convert_certificate_from_pem_to_der)
{
    std::vector<std::string> pemCertificates = {
        "-----BEGIN CERTIFICATE-----",
        "YWFiYg==",
        "-----END CERTIFICATE-----"
    };

    EXPECT_EQ((std::vector<uint8_t>{ 'a', 'a', 'b', 'b' }), services::CertificateConvertor::Convert(pemCertificates).back());
}

TEST(CertificateConvertorTest, should_convert_multi_line_certificate_from_pem_to_der)
{
    std::vector<std::string> pemCertificates = {
        "-----BEGIN CERTIFICATE-----",
        "YWF",
        "iYg==",
        "-----END CERTIFICATE-----"
    };

    auto derCertificates = services::CertificateConvertor::Convert(pemCertificates);

    EXPECT_EQ(1, derCertificates.size());
    EXPECT_EQ((std::vector<uint8_t>{ 'a', 'a', 'b', 'b' }), derCertificates.back());
}

TEST(CertificateConvertorTest, should_convert_multiple_certificates)
{
    std::vector<std::string> pemCertificates = {
        "-----BEGIN CERTIFICATE-----",
        "YWFiYg==",
        "-----END CERTIFICATE-----",
        "-----BEGIN CERTIFICATE-----",
        "Y2NkZA==",
        "-----END CERTIFICATE-----"
    };

    auto derCertificates = services::CertificateConvertor::Convert(pemCertificates);

    EXPECT_EQ(2, derCertificates.size());
    EXPECT_EQ((std::vector<uint8_t>{ 'a', 'a', 'b', 'b' }), derCertificates[0]);
    EXPECT_EQ((std::vector<uint8_t>{ 'c', 'c', 'd', 'd' }), derCertificates[1]);
}

TEST(CertificateConvertorTest, should_throw_on_invalid_certificate)
{
    std::vector<std::string> pemCertificates = {
        "-----BEGIN CERTIFICATE-----",
        "YWFiY@",
        "-----END CERTIFICATE-----"
    };

    EXPECT_THROW({
        try
        {
            services::CertificateConvertor::Convert(pemCertificates);
        }
        catch (const std::exception& e)
        {
            EXPECT_STREQ("CertificateConvertor::Convert Base64 decoding failed", e.what());
            throw;
        }
    },
        std::exception);
}
