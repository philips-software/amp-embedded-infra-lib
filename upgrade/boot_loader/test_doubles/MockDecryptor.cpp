#include "upgrade/boot_loader/test_doubles/MockDecryptor.hpp"

namespace application
{
    infra::ByteRange MockDecryptor::StateBuffer()
    {
        stateBuffer.resize(StateBufferMock());
        return stateBuffer;
    }

    void MockDecryptor::Reset()
    {
        ResetMock();
    }

    void MockDecryptor::DecryptPart(infra::ByteRange data)
    {
        std::vector<uint8_t> result = DecryptPartMock(std::vector<uint8_t>(data.begin(), data.end()));
        assert(result.size() == data.size());
        std::copy(result.begin(), result.end(), data.begin());
    }

    bool MockDecryptor::DecryptAndAuthenticate(infra::ByteRange data)
    {
        std::vector<uint8_t> result = DecryptAndAuthenticateMock(std::vector<uint8_t>(data.begin(), data.end()));
        assert(result.size() == data.size());
        std::copy(result.begin(), result.end(), data.begin());
        return true;
    }
}
