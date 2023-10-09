#ifndef UPGRADE_MOCK_DECRYPTOR_HPP
#define UPGRADE_MOCK_DECRYPTOR_HPP

#include "upgrade/boot_loader/Decryptor.hpp"
#include "gmock/gmock.h"

namespace application
{
    class MockDecryptor
        : public Decryptor
    {
    public:
        infra::ByteRange StateBuffer() override;
        void Reset() override;
        void DecryptPart(infra::ByteRange data) override;
        bool DecryptAndAuthenticate(infra::ByteRange data) override;

        MOCK_METHOD0(StateBufferMock, uint8_t());
        MOCK_METHOD0(ResetMock, void());
        MOCK_METHOD1(DecryptPartMock, std::vector<uint8_t>(std::vector<uint8_t>));
        MOCK_METHOD1(DecryptAndAuthenticateMock, std::vector<uint8_t>(std::vector<uint8_t>));

        std::vector<uint8_t> stateBuffer;
    };
}

#endif
