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

    SupportedTargetsBuilder& SupportedTargetsBuilder::Order(uint8_t order)
    {
        this->order.emplace(order);
        return *this;
    }

    SupportedTargetsBuilder& SupportedTargetsBuilder::AddCmd(const SupportedTargets::Target& target)
    {
        AddToMandatoryWhenNecessary(target);
        AddInOrder(target);
        targets.cmd.emplace_back(target);
        return *this;
    }

    SupportedTargetsBuilder& SupportedTargetsBuilder::AddHex(const SupportedTargets::Target& target)
    {
        AddToMandatoryWhenNecessary(target);
        AddInOrder(target);
        targets.hex.emplace_back(target);
        return *this;
    }

    SupportedTargetsBuilder& SupportedTargetsBuilder::AddElf(const SupportedTargets::Target& target, uint32_t offset)
    {
        AddToMandatoryWhenNecessary(target);
        AddInOrder(target);
        targets.elf.emplace_back(target, offset);
        return *this;
    }

    SupportedTargetsBuilder& SupportedTargetsBuilder::AddBin(const SupportedTargets::Target& target, uint32_t offset)
    {
        AddToMandatoryWhenNecessary(target);
        AddInOrder(target);
        targets.bin.emplace_back(target, offset);
        return *this;
    }

    void SupportedTargetsBuilder::AddToMandatoryWhenNecessary(const SupportedTargets::Target& target)
    {
        if (mandatory)
        {
            targets.mandatory.emplace_back(target);
            mandatory = false;
        }
    }

    void SupportedTargetsBuilder::AddInOrder(const SupportedTargets::Target& target)
    {
        if (order)
        {
            targets.order[*order].emplace_back(target);
            order = infra::none;
        }
    }

    SupportedTargetsBuilder SupportedTargets::Create()
    {
        return SupportedTargetsBuilder();
    }
}
