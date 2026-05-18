#include "services/util/SmiPhy.hpp"
#include "infra/util/BitLogic.hpp"

namespace services
{
    // BasicControlRegister

    bool SmiPhy::BasicControlRegister::IsResetting(uint16_t bcr)
    {
        return infra::IsBitSet(bcr, ResetBit);
    }

    // BasicStatusRegister

    bool SmiPhy::BasicStatusRegister::IsLinked(uint16_t bsr)
    {
        return (bsr & LinkStatusMask) != 0;
    }

    bool SmiPhy::BasicStatusRegister::IsResponding(uint16_t bsr)
    {
        return bsr != NotPresentValue;
    }

    bool SmiPhy::BasicStatusRegister::IsAutoNegCapable(uint16_t bsr)
    {
        return (bsr & AutoNegAbilityMask) != 0;
    }

    bool SmiPhy::BasicStatusRegister::IsAutoNegComplete(uint16_t bsr)
    {
        return (bsr & AutoNegCompleteMask) != 0;
    }

    bool SmiPhy::BasicStatusRegister::HasRemoteFault(uint16_t bsr)
    {
        return (bsr & RemoteFaultMask) != 0;
    }

    // SmiPhy

    SmiPhy::SmiPhy(hal::SmiBus& smiBus, uint8_t phyAddress)
        : smiBus(smiBus)
        , phyAddress(phyAddress)
    {}

    uint16_t SmiPhy::ReadBcr() const
    {
        return Read(BasicControlRegister::Address);
    }

    void SmiPhy::WriteBcr(uint16_t value)
    {
        Write(BasicControlRegister::Address, value);
    }

    uint16_t SmiPhy::ReadBsr() const
    {
        return Read(BasicStatusRegister::Address);
    }

    hal::LinkSpeed SmiPhy::NegotiatedLinkSpeed() const
    {
        const uint16_t advertisement = Read(AutoNegRegister::AdvertisementAddress);
        const uint16_t linkPartner = Read(AutoNegRegister::LinkPartnerAddress);
        const uint16_t common = advertisement & linkPartner;

        if (infra::IsBitSet(common, AutoNegRegister::FullDuplex100MHzBit))
            return hal::LinkSpeed::fullDuplex100MHz;
        else if (infra::IsBitSet(common, AutoNegRegister::HalfDuplex100MHzBit))
            return hal::LinkSpeed::halfDuplex100MHz;
        else if (infra::IsBitSet(common, AutoNegRegister::FullDuplex10MHzBit))
            return hal::LinkSpeed::fullDuplex10MHz;
        else
            return hal::LinkSpeed::halfDuplex10MHz;
    }

    hal::LinkSpeed SmiPhy::LocalLinkSpeed(uint16_t bcr) const
    {
        const bool fast = infra::IsBitSet(bcr, BasicControlRegister::SpeedSelectLsbBit);
        const bool full = infra::IsBitSet(bcr, BasicControlRegister::DuplexModeBit);

        if (fast && full)
            return hal::LinkSpeed::fullDuplex100MHz;
        else if (fast && !full)
            return hal::LinkSpeed::halfDuplex100MHz;
        else if (!fast && full)
            return hal::LinkSpeed::fullDuplex10MHz;
        else
            return hal::LinkSpeed::halfDuplex10MHz;
    }

    SmiPhy::LinkState SmiPhy::ReadLinkState()
    {
        const uint16_t bsr = ReadBsr();
        if (!BasicStatusRegister::IsResponding(bsr))
            return LinkState::Down;

        const uint16_t bcr = ReadBcr();
        const bool autoNegEnabled = infra::IsBitSet(bcr, BasicControlRegister::AutoNegEnableBit);
        const bool autoNegComplete = autoNegEnabled ? BasicStatusRegister::IsAutoNegComplete(bsr) : true;
        return BasicStatusRegister::IsLinked(bsr) && autoNegComplete ? LinkState::Up : LinkState::Down;
    }

    void SmiPhy::EnableAutoNegIfCapable()
    {
        if (BasicStatusRegister::IsAutoNegCapable(ReadBsr()))
        {
            const uint16_t bcr = ReadBcr();
            WriteBcr(bcr | infra::Bit<uint16_t>(BasicControlRegister::AutoNegEnableBit));
        }
    }

    hal::LinkSpeed SmiPhy::LinkSpeed() const
    {
        const uint16_t bcr = ReadBcr();
        if (infra::IsBitSet(bcr, BasicControlRegister::AutoNegEnableBit))
            return NegotiatedLinkSpeed();
        else
            return LocalLinkSpeed(bcr);
    }

    uint16_t SmiPhy::Read(uint16_t reg) const
    {
        return smiBus.Read(phyAddress, reg);
    }

    void SmiPhy::Write(uint16_t reg, uint16_t value)
    {
        smiBus.Write(phyAddress, reg, value);
    }
}
