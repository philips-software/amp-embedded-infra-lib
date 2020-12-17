#ifndef SERVICES_NETWORK_CERTIFICATE_CONVERTOR_HPP
#define SERVICES_NETWORK_CERTIFICATE_CONVERTOR_HPP

#include <string>
#include <vector>

namespace services
{
    class CertificateConvertor
    {
    public:
        using DerCertificate = std::vector<uint8_t>;
        using DerCertificateList = std::vector<DerCertificate>;

        static DerCertificateList Convert(const std::vector<std::string>& pemCertificates);
    };
}

#endif
