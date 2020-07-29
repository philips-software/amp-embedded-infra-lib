#ifndef HAL_DAC_HPP
#define HAL_DAC_HPP

#include "infra/util/Unit.hpp"

namespace hal
{
    template<class Unit, class Storage>
    class Dac
    {
    protected:
        Dac() = default;

        Dac(const Dac&) = delete;
        Dac& operator=(const Dac&) = delete;
        ~Dac() = default;

    public:
        virtual void SetOutput(infra::Quantity<Unit, Storage> value) = 0;
    };

    class DacImplBase
    {
    protected:
        ~DacImplBase() = default;

    public:
        virtual void SetOutput(uint16_t value) = 0;
    };

    template<class Conversion, class Unit, class Storage, class Impl>
    class DacConverter
        : public Dac<Unit, Storage>
        , public Impl
    {
    public:
        template<class... Args>
            DacConverter(Args&&... args);

        void SetOutput(infra::Quantity<Unit, Storage> value);
    };

    ////    Implementation    ////

    template<class Conversion, class Unit, class Storage, class Impl>
    template<class... Args>
    DacConverter<Conversion, Unit, Storage, Impl>::DacConverter(Args&&... args)
        : Impl(std::forward<Args>(args)...)
    {}

    template<class Conversion, class Unit, class Storage, class Impl>
    void DacConverter<Conversion, Unit, Storage, Impl>::SetOutput(infra::Quantity<Unit, Storage> value)
    {
        infra::Quantity<infra::Identity, Storage> convertedValue(value * infra::Quantity<Conversion, Storage>(1));
        Impl::SetOutput(convertedValue.Value());
    }
}

#endif
