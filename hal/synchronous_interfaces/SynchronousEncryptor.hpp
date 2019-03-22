#ifndef SYNCHRONOUS_HAL_ENCRYPTOR_HPP
#define SYNCHRONOUS_HAL_ENCRYPTOR_HPP

#include "infra/util/ByteRange.hpp"

namespace hal
{
    class SynchronousEncryptor
    {
    protected:
        SynchronousEncryptor() = default;
        SynchronousEncryptor(const SynchronousEncryptor& other) = delete;
        SynchronousEncryptor& operator=(const SynchronousEncryptor& other) = delete;
        ~SynchronousEncryptor() = default;

    public:
        virtual infra::ByteRange StateBuffer() = 0;
        virtual void Reset() = 0;
        virtual void Encrypt(infra::ConstByteRange input, infra::ByteRange output) = 0;
        virtual void Decrypt(infra::ConstByteRange input, infra::ByteRange output) = 0;
    };
}

#endif
