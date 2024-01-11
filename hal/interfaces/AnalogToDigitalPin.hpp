#ifndef HAL_ANALOG_TO_DIGITAL_PIN_HPP
#define HAL_ANALOG_TO_DIGITAL_PIN_HPP

#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/MemoryRange.hpp"
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
        using SamplesRange = infra::MemoryRange<infra::Quantity<Unit, Storage>>;

        virtual void Measure(SamplesRange samples, const infra::Function<void()>& onDone) = 0;
    };

    class AnalogToDigitalPinImplBase
    {
    protected:
        ~AnalogToDigitalPinImplBase() = default;

    public:
        virtual void Measure(infra::MemoryRange<int32_t> samples, const infra::Function<void()>& onDone) = 0;
    };

    template<class Conversion, class Unit, class Storage, class Impl>
    class AnalogToDigitalPinConverter
        : public AnalogToDigitalPin<Unit, Storage>
        , public Impl
    {
    public:
        template<class... Args>
        explicit AnalogToDigitalPinConverter(Args&&... args);

        void Measure(typename AnalogToDigitalPin<Unit, Storage>::SamplesRange samples, const infra::Function<void()>& onDone);

    private:
        typename AnalogToDigitalPin<Unit, Storage>::SamplesRange samples;
        infra::AutoResetFunction<void()> onDone;
    };

    ////    Implementation    ////

    template<class Conversion, class Unit, class Storage, class Impl>
    template<class... Args>
    AnalogToDigitalPinConverter<Conversion, Unit, Storage, Impl>::AnalogToDigitalPinConverter(Args&&... args)
        : Impl(std::forward<Args>(args)...)
    {}

    template<class Conversion, class Unit, class Storage, class Impl>
    void AnalogToDigitalPinConverter<Conversion, Unit, Storage, Impl>::Measure(typename AnalogToDigitalPin<Unit, Storage>::SamplesRange samples, const infra::Function<void()>& onDone)
    {
        this->samples = samples;
        this->onDone = onDone;
        Impl::Measure(samples, [this]()
            {
                for (auto& sample : this->samples)
                    sample = infra::Quantity<Conversion, Storage>(sample);

                this->onDone();
            });
    }
}

#endif
