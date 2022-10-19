#ifndef SYNCHRONOUS_HAL_ENCRYPTOR_MOCK_HPP
#define SYNCHRONOUS_HAL_ENCRYPTOR_MOCK_HPP

#include "hal/synchronous_interfaces/SynchronousEncryptor.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class SynchronousEncryptorMock
        : public SynchronousEncryptor
    {
    public:
        MOCK_METHOD0(StateBuffer, infra::ByteRange());
        MOCK_METHOD0(Reset, void());
        MOCK_METHOD2(Encrypt, void(infra::ConstByteRange input, infra::ByteRange output));
        MOCK_METHOD2(Decrypt, void(infra::ConstByteRange input, infra::ByteRange output));
    };
}

#endif
