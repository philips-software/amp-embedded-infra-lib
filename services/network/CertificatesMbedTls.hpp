#ifndef SERVICES_CERTIFICATES_MBED_TLS_HPP
#define SERVICES_CERTIFICATES_MBED_TLS_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/syntax/Asn1Formatter.hpp"
#include "infra/util/BoundedString.hpp"
#include "mbedtls/ssl.h"

namespace services
{
    class CertificatesMbedTls
    {
    public:
        CertificatesMbedTls();
        CertificatesMbedTls(const CertificatesMbedTls& other) = delete;
        CertificatesMbedTls& operator=(const CertificatesMbedTls& other) = delete;
        ~CertificatesMbedTls();

        virtual void AddCertificateAuthority(infra::ConstByteRange certificate);
        virtual void AddCertificateAuthority(const infra::BoundedConstString& certificate);
        virtual void AddOwnCertificate(const infra::BoundedConstString& certificate, const infra::BoundedConstString& privateKey);

        virtual void Config(mbedtls_ssl_config& sslConfig);

        virtual void GenerateNewKey(hal::SynchronousRandomDataGenerator& randomDataGenerator);
        virtual void WritePrivateKey(infra::BoundedString& outputBuffer);
        virtual void WriteOwnCertificate(infra::BoundedString& outputBuffer, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        virtual void UpdateValidToDate();

    private:
        int32_t ExtractExponent(const mbedtls_rsa_context& rsaContext) const;

        void X509AddAlgorithm(infra::Asn1Formatter& root, const mbedtls_x509_buf& oid) const;
        void X509AddName(infra::Asn1Formatter& root, const mbedtls_x509_name& name) const;
        void X509AddTime(infra::Asn1Formatter& root, const mbedtls_x509_time& time) const;

    private:
        // As per rfc5280 the well-defined date for having no expiration is 99991231235959Z.
        const mbedtls_x509_time unlimitedExpirationDate = { 9999, 12, 31, 23, 59, 59 };

    protected:
        mbedtls_x509_crt caCertificates;
        mbedtls_x509_crt ownCertificate;
        mbedtls_pk_context privateKey;
    };
}  //! namespace services

#endif  //! SERVICES_CERTIFICATES_MBED_TLS_HPP
