#ifndef UPGRADE_PACK_BUILD_LIBRARY_UPGRADE_PACK_BUILDER_HPP
#define UPGRADE_PACK_BUILD_LIBRARY_UPGRADE_PACK_BUILDER_HPP

#include "hal/interfaces/FileSystem.hpp"
#include "infra/util/MemoryRange.hpp"
#include "upgrade/pack_builder/Input.hpp"
#include "upgrade/pack_builder/ImageSigner.hpp"
#include <memory>
#include <vector>

namespace application
{
    class SignatureDoesNotVerifyException
        : public std::runtime_error
    {
    public:
        SignatureDoesNotVerifyException();
    };

    class UpgradePackBuilder
    {
    public:
        struct HeaderInfo
        {
            std::string productName;
            std::string productVersion;
            std::string componentName;
            uint32_t componentVersion;
        };

    public:
        UpgradePackBuilder(const HeaderInfo& headerInfo, std::vector<std::unique_ptr<Input>>&& inputs, ImageSigner& signer, UpgradePackStatus initialStatus = UpgradePackStatus::readyToDeploy);

        std::vector<uint8_t>& UpgradePack();
        void WriteUpgradePack(const std::string& fileName, hal::FileSystem& fileSystem);

    private:
        void CreateUpgradePack();
        void AddPrologueAndSignature();
        void AddEpilogue();
        void AddImages();
        void AssignZeroFilled(const std::string& data, infra::MemoryRange<char> destination) const;
        void CheckSignature();

    private:
        HeaderInfo headerInfo;
        UpgradePackStatus initialStatus;
        std::vector<std::unique_ptr<Input>> inputs;
        ImageSigner& signer;
        std::vector<uint8_t> upgradePack;
    };
}

#endif
