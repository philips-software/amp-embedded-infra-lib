#ifndef HAL_DIGITAL_TO_ANALOG_PIN_HPP
#define HAL_DIGITAL_TO_ANALOG_PIN_HPP

#include "infra/util/Unit.hpp"
#include <stdint.h>

namespace hal
{
    template<class Unit, class Storage>
    class DigitalToAnalogPin
    {
    protected:
        DigitalToAnalogPin() = default;
        DigitalToAnalogPin(const DigitalToAnalogPin&) = delete;
        DigitalToAnalogPin& operator=(const DigitalToAnalogPin&) = delete;
        ~DigitalToAnalogPin() = default;

    public:
        virtual void Set(infra::Quantity<Unit, Storage> value) = 0;
    };

    class DigitalToAnalogPinImplBase
    {
    protected:
        ~DigitalToAnalogPinImplBase() = default;

    public:
        virtual void Set(uint16_t value) = 0;
    };

    template<class Conversion, class Unit, class Storage, class Impl>
    class DigitalToAnalogPinConverter
        : public DigitalToAnalogPin<Unit, Storage>
        , public Impl
    {
    public:
        template<class... Args>
        DigitalToAnalogPinConverter(Args&&... args);

        void Set(infra::Quantity<Unit, Storage> value);
    };

    ////    Implementation    ////

    template<class Conversion, class Unit, class Storage, class Impl>
    template<class... Args>
    DigitalToAnalogPinConverter<Conversion, Unit, Storage, Impl>::DigitalToAnalogPinConverter(Args&&... args)
        : Impl(std::forward<Args>(args)...)
    {}

    template<class Conversion, class Unit, class Storage, class Impl>
    void DigitalToAnalogPinConverter<Conversion, Unit, Storage, Impl>::Set(infra::Quantity<Unit, Storage> value)
    {
        Impl::Set(infra::Quantity<Conversion, Storage>(value).Value());
    }
}

#endif
