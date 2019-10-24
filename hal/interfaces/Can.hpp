#ifndef HAL_CAN_HPP
#define HAL_CAN_HPP

#include "hal/interfaces/Gpio.hpp"
#include "infra/util/BoundedVector.hpp"
#include <cstdint>

namespace hal
{
    class Can
    {
    public:
        class Id
        {
        public:
            static Id Create11BitId(uint32_t id);
            static Id Create29BitId(uint32_t id);

            bool Is11BitId() const;
            bool Is29BitId() const;

            uint32_t Get11BitId() const;
            uint32_t Get29BitId() const;

        private:
            explicit Id(uint32_t id);

        private:
            static const uint32_t indicator29Bit = static_cast<uint32_t>(1) << 31;

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
