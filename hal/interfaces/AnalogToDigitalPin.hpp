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

    template<class Storage>
    class AnalogToDigitalPinImplBase
    {
    protected:
        ~AnalogToDigitalPinImplBase() = default;

    public:
        explicit AnalogToDigitalPinImplBase(infra::MemoryRange<Storage> samplesBuffer)
            : samplesBuffer(samplesBuffer)
        {}

        virtual void Measure(const infra::Function<void()>& onDone) = 0;

    protected:
        infra::MemoryRange<Storage> samplesBuffer;
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
        Impl::Measure([this]()
            {
                for (std::size_t index = 0; index < this->samples.size(); index++)
                    this->samples[index] = infra::Quantity<Conversion, Storage>(Impl::samplesBuffer[index]);

                this->onDone();
            });
    }
}

#endif
