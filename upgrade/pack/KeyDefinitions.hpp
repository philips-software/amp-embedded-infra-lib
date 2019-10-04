#ifndef UPGRADE_PACK_KEY_DEFINITIONS_HPP
#define UPGRADE_PACK_KEY_DEFINITIONS_HPP

#include "infra/util/ByteRange.hpp"

struct RsaPublicKey
{
    RsaPublicKey(infra::ConstByteRange N, infra::ConstByteRange E)
        : N(N)
        , E(E)
    {}

    infra::ConstByteRange N;
    infra::ConstByteRange E;
};

struct RsaPrivateKey
{
    RsaPrivateKey(infra::ConstByteRange D, infra::ConstByteRange P, infra::ConstByteRange Q)
        : D(D)
        , P(P)
        , Q(Q)
    {}

    infra::ConstByteRange D;
    infra::ConstByteRange P;
    infra::ConstByteRange Q;
};

#endif
