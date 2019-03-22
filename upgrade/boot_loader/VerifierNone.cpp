#include "upgrade/boot_loader/VerifierNone.hpp"

namespace application
{
    bool VerifierNone::IsValid(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature, const hal::SynchronousFlash::Range& data) const
    {
        return true;
    }
}
