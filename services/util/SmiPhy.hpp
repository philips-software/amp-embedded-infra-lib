#ifndef SERVICES_SMI_PHY_HPP
#define SERVICES_SMI_PHY_HPP

#include "hal/interfaces/Ethernet.hpp"
#include "hal/interfaces/SmiBus.hpp"
#include <cstdint>
#include <optional>

namespace services
{
    // Wraps a single IEEE 802.3 PHY accessible over a SmiBus.
    // Owns all register address and bit-field knowledge for standard PHY
    // registers (BCR, BSR, ANLPA, ANLPAR) and tracks link state changes.
    class SmiPhy
    {
    public:
        // IEEE 802.3 Basic Control Register (register 0) bit positions.
        struct BasicControlRegister
        {
            static constexpr uint16_t Address = 0u;
            static constexpr uint16_t DuplexModeBit = 8u;
            static constexpr uint16_t RestartAutoNegBit = 9u;
            static constexpr uint16_t AutoNegEnableBit = 12u;
            static constexpr uint16_t SpeedSelectLsbBit = 13u;
            static constexpr uint16_t ResetBit = 15u;

            static bool IsResetting(uint16_t bcr);
        };

        // IEEE 802.3 Basic Status Register (register 1) bit masks.
        struct BasicStatusRegister
        {
            static constexpr uint16_t Address = 1u;
            static constexpr uint16_t LinkStatusMask = 0x0004u;      // bit 2
            static constexpr uint16_t AutoNegAbilityMask = 0x0008u;  // bit 3
            static constexpr uint16_t RemoteFaultMask = 0x0010u;     // bit 4
            static constexpr uint16_t AutoNegCompleteMask = 0x0020u; // bit 5
            static constexpr uint16_t NotPresentValue = 0xFFFFu;

            static bool IsLinked(uint16_t bsr);
            static bool IsResponding(uint16_t bsr);
            static bool IsAutoNegCapable(uint16_t bsr);
            static bool IsAutoNegComplete(uint16_t bsr);
            static bool HasRemoteFault(uint16_t bsr);
        };

        // IEEE 802.3 Auto-Negotiation Advertisement / Link Partner Ability
        // registers (4 and 5) share the same bit layout.
        struct AutoNegRegister
        {
            static constexpr uint16_t AdvertisementAddress = 4u;
            static constexpr uint16_t LinkPartnerAddress = 5u;
            static constexpr uint16_t HalfDuplex10MHzBit = 5u;
            static constexpr uint16_t FullDuplex10MHzBit = 6u;
            static constexpr uint16_t HalfDuplex100MHzBit = 7u;
            static constexpr uint16_t FullDuplex100MHzBit = 8u;
        };

        enum class LinkState
        {
            Up,
            Down,
        };

        SmiPhy(hal::SmiBus& smiBus, uint8_t phyAddress);

        uint16_t ReadBcr() const;
        void WriteBcr(uint16_t value);
        uint16_t ReadBsr() const;

        // Read BSR, check whether autoneg has completed (via BCR), and return
        // the link state transition if any. Returns nullopt when unchanged.
        std::optional<LinkState> ReadLinkState();

        // If the PHY advertises autoneg capability, enable it via BCR.
        void EnableAutoNegIfCapable();

        // Returns the current link speed, reading BCR to decide between
        // negotiated and locally-configured speed.
        hal::LinkSpeed LinkSpeed() const;

    private:
        hal::LinkSpeed NegotiatedLinkSpeed() const;
        hal::LinkSpeed LocalLinkSpeed(uint16_t bcr) const;
        std::optional<LinkState> UpdateLinkState(bool isLinked);
        uint16_t Read(uint16_t reg) const;
        void Write(uint16_t reg, uint16_t value);

        hal::SmiBus& smiBus_;
        uint8_t phyAddress_;
        bool linkUp_ = false;
    };
}

#endif
