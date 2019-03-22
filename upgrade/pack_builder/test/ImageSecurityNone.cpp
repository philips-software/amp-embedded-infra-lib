#include "upgrade/pack_builder/test/ImageSecurityNone.hpp"

namespace application
{
    uint32_t ImageSecurityNone::EncryptionAndMacMethod() const
    {
        return 0;
    }

    std::vector<uint8_t> ImageSecurityNone::Secure(const std::vector<uint8_t>& data) const
    {
        return data;
    }
}
