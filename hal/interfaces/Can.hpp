#ifndef HAL_CAN_HPP
#define HAL_CAN_HPP

#include "infra/util/BoundedVector.hpp"
#include "infra/util/Function.hpp"
#include <cstdint>

namespace hal
{
    class Can
    {
    public:
        class [[nodiscard]] Id
        {
        public:
            [[nodiscard]] static constexpr Id Create11BitId(uint32_t id)
            {
                return Can::Id(id);
            }

            [[nodiscard]] static constexpr Id Create29BitId(uint32_t id)
            {
                return Can::Id(id | indicator29Bit);
            }

            [[nodiscard]] constexpr bool Is11BitId() const
            {
                return (id & indicator29Bit) == 0;
            }

            [[nodiscard]] constexpr bool Is29BitId() const
            {
                return !Is11BitId();
            }

            [[nodiscard]] constexpr uint32_t Get11BitId() const
            {
                return id;
            }

            [[nodiscard]] constexpr uint32_t Get29BitId() const
            {
                return id ^ indicator29Bit;
            }

            [[nodiscard]] constexpr bool operator==(const Id& other) const
            {
                return id == other.id;
            }

            [[nodiscard]] constexpr bool operator!=(const Id& other) const
            {
                return !(*this == other);
            }

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
}

#endif
