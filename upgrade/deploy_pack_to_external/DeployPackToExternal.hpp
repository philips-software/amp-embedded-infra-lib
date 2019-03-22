#ifndef UPGRADE_DEPLOY_PACK_TO_EXTERNAL_HPP
#define UPGRADE_DEPLOY_PACK_TO_EXTERNAL_HPP

#include "hal/interfaces/Flash.hpp"
#include "hal/interfaces/Gpio.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/Observer.hpp"
#include "infra/util/Sequencer.hpp"
#include "upgrade/pack/UpgradePackHeader.hpp"

namespace application
{
    class DeployPackToExternal;

    class DeployPackToExternalObserver
        : public infra::SingleObserver<DeployPackToExternalObserver, DeployPackToExternal>
    {
    public:
        using infra::SingleObserver<DeployPackToExternalObserver, DeployPackToExternal>::SingleObserver;

        virtual void NotDeployable() = 0;
        virtual void DoesntFit() = 0;
        virtual void Done() = 0;
    };

    class DeployPackToExternal
        : public infra::Subject<DeployPackToExternalObserver>
    {
    public:
        DeployPackToExternal(hal::Flash& from, hal::Flash& to);

    private:
        hal::Flash& from;
        hal::Flash& to;

        infra::Sequencer sequencer;

        UpgradePackHeaderPrologue header;
        std::array<uint8_t, 256> buffer;
        std::size_t sizeToDo = 0;
        std::size_t currentSize = 0;
        uint32_t currentReadAddress = 0;
        uint32_t currentWriteAddress = 0;
    };
}

#endif
