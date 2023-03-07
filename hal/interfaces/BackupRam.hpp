#ifndef HAL_BACKUPRAM_HPP
#define HAL_BACKUPRAM_HPP

#include "infra/util/MemoryRange.hpp"

namespace hal
{
    template<class T>
    class BackupRam
    {
    public:
        virtual ~BackupRam() = default;

        virtual infra::MemoryRange<T> Get() const = 0;
    };
}

#endif
