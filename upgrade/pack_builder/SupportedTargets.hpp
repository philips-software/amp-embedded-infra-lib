#ifndef UPGRADE_SUPPORTED_TARGETS_HPP
#define UPGRADE_SUPPORTED_TARGETS_HPP

#include <string>
#include <utility>
#include <vector>

namespace application
{
    class SupportedTargetsBuilder;

    class SupportedTargets
    {
    public:
        using Target = std::string;
        using TargetWithOffset = std::pair<Target, uint32_t>;

        friend class SupportedTargetsBuilder;
        static SupportedTargetsBuilder Create();

        const auto& CmdTargets() const { return cmd; }
        const auto& HexTargets() const { return hex; }
        const auto& ElfTargets() const { return elf; }
        const auto& BinTargets() const { return bin; }
        const auto& MandatoryTargets() const { return mandatory; }

    private:
        std::vector<Target> cmd;
        std::vector<Target> hex;
        std::vector<TargetWithOffset> elf;
        std::vector<TargetWithOffset> bin;

        std::vector<Target> mandatory;
    };

    class SupportedTargetsBuilder
    {
    public:
        operator SupportedTargets() const { return std::move(targets); }

        SupportedTargetsBuilder& Mandatory();
        SupportedTargetsBuilder& Optional();

        SupportedTargetsBuilder& AddCmd(const SupportedTargets::Target& target);
        SupportedTargetsBuilder& AddHex(const SupportedTargets::Target& target);
        SupportedTargetsBuilder& AddElf(const SupportedTargets::Target& target, uint32_t offset);
        SupportedTargetsBuilder& AddBin(const SupportedTargets::Target& target, uint32_t offset);

    private:
        void AddToMandatoryWhenNecessary(const SupportedTargets::Target& target);

    private:
        SupportedTargets targets;
        bool mandatory{ false };
    };
}

#endif
