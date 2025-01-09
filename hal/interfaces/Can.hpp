#ifndef HAL_CAN_HPP
#define HAL_CAN_HPP

#include "infra/util/BoundedVector.hpp"
#include "infra/util/Function.hpp"
#include <cassert>
#include <cstdint>

namespace hal
{
    class Can
    {
    public:
        class [[nodiscard]] Id
        {
        public:
            static constexpr Id Create11BitId(uint32_t id);
            static constexpr Id Create29BitId(uint32_t id);

            [[nodiscard]] constexpr bool Is11BitId() const;
            [[nodiscard]] constexpr bool Is29BitId() const;

            [[nodiscard]] constexpr uint32_t Get11BitId() const;
            [[nodiscard]] constexpr uint32_t Get29BitId() const;

            [[nodiscard]] constexpr bool operator==(const Id& other) const;
            [[nodiscard]] constexpr bool operator!=(const Id& other) const;

        private:
            constexpr explicit Id(uint32_t id)
                : id(id)
            {}

        private:
            static constexpr uint32_t indicator29Bit{ 1u << 31 };

            uint32_t id;
        };

    protected:
        Can() = default;
        Can(const Can& other) = delete;
        Can& operator=(const Can& other) = delete;
        ~Can() = default;

    public:
        using Message = infra::BoundedVector<uint8_t>::WithMaxSize<8>;

    public:
        virtual void SendData(Id id, const Message& data, const infra::Function<void(bool success)>& actionOnCompletion) = 0;
        virtual void ReceiveData(const infra::Function<void(Id id, const Message& data)>& receivedAction) = 0;
    };

    //// Implementation ////

    constexpr Can::Id Can::Id::Create11BitId(uint32_t id)
    {
        return Can::Id(id);
    }

    constexpr Can::Id Can::Id::Create29BitId(uint32_t id)
    {
        return Can::Id(id | indicator29Bit);
    }

    constexpr bool Can::Id::Is11BitId() const
    {
        return (id & indicator29Bit) == 0;
    }

    constexpr bool Can::Id::Is29BitId() const
    {
        return !Is11BitId();
    }

    constexpr uint32_t Can::Id::Get11BitId() const
    {
        assert(Is11BitId());
        return id;
    }

    constexpr uint32_t Can::Id::Get29BitId() const
    {
        assert(Is29BitId());
        return id ^ indicator29Bit;
    }

    constexpr bool Can::Id::operator==(const Id& other) const
    {
        return id == other.id;
    }

    constexpr bool Can::Id::operator!=(const Id& other) const
    {
        return !(*this == other);
    }
}

#endif
