#include "upgrade/pack_builder/SupportedTargets.hpp"

namespace application
{
    SupportedTargetsBuilder& SupportedTargetsBuilder::Mandatory()
    {
        mandatory = true;
        return *this;
    }

    SupportedTargetsBuilder& SupportedTargetsBuilder::Optional()
    {
        mandatory = false;
        return *this;
    }

    SupportedTargetsBuilder& SupportedTargetsBuilder::AddHex(SupportedTargets::Target target)
    {
        AddToMandatoryWhenNecessary(target);
        targets.hex.push_back(target);
        return *this;
    }

    SupportedTargetsBuilder& SupportedTargetsBuilder::AddElf(SupportedTargets::Target target, uint32_t offset)
    {
        AddToMandatoryWhenNecessary(target);
        targets.elf.push_back(std::make_pair(target, offset));
        return *this;
    }

    SupportedTargetsBuilder& SupportedTargetsBuilder::AddBin(SupportedTargets::Target target, uint32_t offset)
    {
        AddToMandatoryWhenNecessary(target);
        targets.bin.push_back(std::make_pair(target, offset));
        return *this;
    }

    void SupportedTargetsBuilder::AddToMandatoryWhenNecessary(SupportedTargets::Target target)
    {
        if (mandatory)
            targets.mandatory.push_back(target);
    }

    SupportedTargetsBuilder SupportedTargets::Create()
    {
        return SupportedTargetsBuilder();
    }
}
