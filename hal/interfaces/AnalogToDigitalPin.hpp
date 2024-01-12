#ifndef HAL_ANALOG_TO_DIGITAL_PIN_HPP
#define HAL_ANALOG_TO_DIGITAL_PIN_HPP

#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/Unit.hpp"

namespace hal
{
    template<class Unit, class Storage>
    class AnalogToDigitalPin
    {
    protected:
        AnalogToDigitalPin() = default;
        AnalogToDigitalPin(const AnalogToDigitalPin&) = delete;
        AnalogToDigitalPin& operator=(const AnalogToDigitalPin&) = delete;
        ~AnalogToDigitalPin() = default;

    public:
        virtual void Measure(const infra::Function<void(infra::Quantity<Unit, Storage> value)>& onDone) = 0;
    };

    class AnalogToDigitalPinImplBase
    {
    protected:
        ~AnalogToDigitalPinImplBase() = default;

    public:
        virtual void Measure(const infra::Function<void(int32_t value)>& onDone) = 0;
    };

    template<class Storage>
    class AnalogToDigitalBulkSamples
    {
    public:
        ~AnalogToDigitalBulkSamples() = default;
        virtual void Measure(infra::MemoryRange<Storage> buffer, const infra::Function<void()>& onDone) = 0;
    };

    template<class Conversion, class Unit, class Storage, class Impl>
    class AnalogToDigitalPinConverter
        : public AnalogToDigitalPin<Unit, Storage>
        , public Impl
    {
        static_assert(std::is_base_of<AnalogToDigitalPinImplBase, Impl>::value, "Impl must be a derived class of AnalogToDigitalPinImplBase.");

    public:
        template<class... Args>
        AnalogToDigitalPinConverter(Args&&... args);

        void Measure(const infra::Function<void(infra::Quantity<Unit, Storage> value)>& onDone);

    private:
        infra::AutoResetFunction<void(infra::Quantity<Unit, Storage> value)> onDone;
    };

    ////    Implementation    ////

    template<class Conversion, class Unit, class Storage, class Impl>
    template<class... Args>
    AnalogToDigitalPinConverter<Conversion, Unit, Storage, Impl>::AnalogToDigitalPinConverter(Args&&... args)
        : Impl(std::forward<Args>(args)...)
    {}

    template<class Conversion, class Unit, class Storage, class Impl>
    void AnalogToDigitalPinConverter<Conversion, Unit, Storage, Impl>::Measure(const infra::Function<void(infra::Quantity<Unit, Storage> value)>& onDone)
    {
        this->onDone = onDone;
        Impl::Measure([this](int32_t value)
            {
                this->onDone(infra::Quantity<Conversion, Storage>(value));
            });
    }
}

#endif
