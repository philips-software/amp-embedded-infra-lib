#include "services/network/CertificateConvertor.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/util/Base64.hpp"

namespace
{
    bool ConvertBase64(const std::string& fromString, std::vector<uint8_t>& to)
    {
        uint8_t bitIndex = 0;
        uint16_t decodedWord = 0;
        auto currentChar = fromString.begin();

        while (currentChar != fromString.end())
        {
            uint8_t decodedValue = infra::DecodeBase64Byte(*currentChar);

            if (decodedValue < 64)
            {
                decodedWord = (decodedWord << 6) | decodedValue;
                bitIndex += 6;

                if (bitIndex >= 8)
                {
                    to.push_back(static_cast<uint8_t>(decodedWord >> static_cast<uint8_t>(bitIndex - 8)));
                    bitIndex -= 8;
                }
            }
            else if (*currentChar != '=')
                return false;

            ++currentChar;
        }

        return currentChar == fromString.end() || *currentChar == '=';
    }
}

namespace services
{
    CertificateConvertor::DerCertificateList CertificateConvertor::Convert(const std::vector<std::string>& pemCertificates)
    {
        std::vector<std::vector<uint8_t>> derCertificates;
        std::string pemContents;

        for (auto& line : pemCertificates)
        {
            if (line.find("-----BEGIN CERTIFICATE-----") != std::string::npos)
                continue;

            if (line.find("-----END CERTIFICATE-----") != std::string::npos)
            {
                derCertificates.push_back(std::vector<uint8_t>{});
                if (!ConvertBase64(pemContents, derCertificates.back()))
                    throw std::runtime_error("CertificateConvertor::Convert Base64 decoding failed");

                pemContents.clear();
            }
            else
                pemContents += line;
        }

        return derCertificates;
    }
}
