#include "infra/stream/StringInputStream.hpp"
#include "services/network/CertificateConvertor.hpp"

namespace
{
    uint8_t DecodeByte(char base64)
    {
        static const char* encodeTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        return static_cast<uint8_t>(std::find(&encodeTable[0], &encodeTable[64], base64) - &encodeTable[0]);
    }

    bool ConvertBase64(const std::string& fromString, std::vector<uint8_t>& to)
    {
        uint8_t bitIndex = 0;
        uint16_t decodedWord = 0;
        auto currentChar = fromString.begin();

        while (currentChar != fromString.end())
        {
            uint8_t decodedValue = DecodeByte(*currentChar);

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
