#ifndef INFRA_UNIT_HPP
#define INFRA_UNIT_HPP

#include "infra/util/VariadicTemplates.hpp"
#include <cstdint>
#include <utility>

namespace infra
{
    template<uint64_t Value1, uint64_t Value2>
    struct StaticGCD
    {
        static const uint64_t value = StaticGCD<Value2, Value1 % Value2>::value;
    };

    template<uint64_t Value1>
    struct StaticGCD<Value1, 0>
    {
        static const uint64_t value = Value1;
    };

    template<uint64_t Value2>
    struct StaticGCD<0, Value2>
    {
        static const uint64_t value = Value2;
    };

    template<uint64_t N, uint64_t D>
    struct StaticRationalBase
    {
        static_assert(StaticGCD<N, D>::value == 1, "GCD must be 1");

        static const uint64_t n = N;
        static const uint64_t d = D;

        typedef StaticRationalBase<D, N> Inverse;

        template<class Other>
        using Mul = StaticRationalBase<n * Other::n / StaticGCD<n * Other::n, d * Other::d>::value, d * Other::d / StaticGCD<n * Other::n, d * Other::d>::value>;

        template<class Other>
        using Div = StaticRationalBase<n * Other::d / StaticGCD<n, Other::d>::value, d * Other::n / StaticGCD<d, Other::n>::value>;
    };

    template<uint64_t N, uint64_t D>
    using StaticRational = StaticRationalBase<N / StaticGCD<N, D>::value, D / StaticGCD<N, D>::value>;

    template<std::size_t Identifier, int Dimension>
    struct Unit
    {
        static const std::size_t identifier = Identifier;
        static const int dimension = Dimension;

        typedef Unit<Identifier, -Dimension> Inverse;
    };

    template<class UnitList>
    struct InverseEachUnit;

    template<class... Units>
    struct InverseEachUnit<List<Units...>>
    {
        template<class TheUnit>
        struct InverseUnit
        {
            typedef typename TheUnit::Inverse Type;
        };

        typedef List<typename InverseUnit<Units>::Type...> Type;
    };

    template<class... Units>
    struct MultiplyUnits;

    template<class Unit, class ScaleFactor>
    struct ScaleHelper;

    template<class UnitList_, class Factor_>
    struct UnitBase
    {
        typedef UnitList_ UnitList;
        typedef Factor_ Factor;

        template<class ScaleFactor>
        using Scale = typename ScaleHelper<UnitBase, ScaleFactor>::Type;

        using Inverse = UnitBase<typename InverseEachUnit<UnitList>::Type, typename Factor::Inverse>;

        template<class MulUnit>
        using Mul = typename MultiplyUnits<UnitBase, MulUnit>::Type;

        template<class DivUnit>
        using Div = Mul<typename DivUnit::Inverse>;
    };

    template<class Unit, class ScaleFactor>
    struct ScaleHelper
    {
        typedef UnitBase<typename Unit::UnitList, typename Unit::Factor::template Mul<ScaleFactor>> Type;
    };

    template<class UnitList1, class UnitList2, class Enable = void>
    struct MultiplyTwoUnitLists;

    template<class... Units>
    struct MultiplyUnits;

    template<class Unit>
    struct MultiplyUnits<Unit>
    {
        typedef Unit Type;
    };

    template<class Unit1, class Unit2, class... OtherUnits>
    struct MultiplyUnits<Unit1, Unit2, OtherUnits...>
    {
        typedef UnitBase<typename MultiplyUnits<typename MultiplyTwoUnitLists<typename Unit1::UnitList, typename Unit2::UnitList>::Type, OtherUnits...>::Type, typename Unit1::Factor::template Mul<typename Unit2::Factor> > Type;
    };

    template<class UnitList1, class UnitList2>
    struct UnitListSame
    {
        static const bool value = false;
    };

    template<class... UnitListElements1>
    struct UnitListSame<List<UnitListElements1...>, List<UnitListElements1...>>
    {
        static const bool value = true;
    };

    template<class Unit1, class Unit2>
    struct UnitSame
    {
        static const bool value = UnitListSame<typename Unit1::UnitList, typename Unit2::UnitList>::value;
    };

    template<>
    struct MultiplyTwoUnitLists<List<>, List<>>
    {
        typedef List<> Type;
    };

    template<class... Units>
    struct MultiplyTwoUnitLists<List<Units...>, List<>>
    {
        typedef List<Units...> Type;
    };

    template<class... Units>
    struct MultiplyTwoUnitLists<List<>, List<Units...>>
    {
        typedef List<Units...> Type;
    };

    template<class Unit1First, class... Unit1Others, class Unit2First, class... Unit2Others>
    struct MultiplyTwoUnitLists<List<Unit1First, Unit1Others...>, List<Unit2First, Unit2Others...>
        , typename std::enable_if<(Unit1First::identifier < Unit2First::identifier)>::type>
    {
        typedef typename ListPushFront<Unit1First, typename MultiplyTwoUnitLists<List<Unit1Others...>, List<Unit2First, Unit2Others...>>::Type>::Type Type;
    };

    template<class Unit1First, class... Unit1Others, class Unit2First, class... Unit2Others>
    struct MultiplyTwoUnitLists<List<Unit1First, Unit1Others...>, List<Unit2First, Unit2Others...>
        , typename std::enable_if<(Unit1First::identifier > Unit2First::identifier)>::type>
    {
        typedef typename ListPushFront<Unit2First, typename MultiplyTwoUnitLists<List<Unit1First, Unit1Others...>, List<Unit2Others...>>::Type>::Type Type;
    };

    template<class Unit1First, class... Unit1Others, class Unit2First, class... Unit2Others>
    struct MultiplyTwoUnitLists<List<Unit1First, Unit1Others...>, List<Unit2First, Unit2Others...>
        , typename std::enable_if<(Unit1First::identifier == Unit2First::identifier && Unit1First::dimension + Unit2First::dimension != 0)>::type>
    {
        template<class Base1, class Base2>
        struct AddUnit;

        template<std::size_t Identifier, int Power1, int Power2>
        struct AddUnit<Unit<Identifier, Power1>, Unit<Identifier, Power2>>
        {
            typedef Unit<Identifier, Power1 + Power2> Type;
        };

        template<std::size_t Identifier, int Power>
        struct AddUnit<Unit<Identifier, Power>, Unit<Identifier, Power>>
        {
            typedef Unit<Identifier, 2 * Power> Type;
        };

        typedef typename AddUnit<Unit1First, Unit2First>::Type AddResult;
        typedef typename MultiplyTwoUnitLists<List<Unit1Others...>, List<Unit2Others...>>::Type OtherTypes;

        typedef typename ListPushFront<AddResult, OtherTypes>::Type Type;
    };

    template<class Unit1First, class... Unit1Others, class Unit2First, class... Unit2Others>
    struct MultiplyTwoUnitLists<List<Unit1First, Unit1Others...>, List<Unit2First, Unit2Others...>
        , typename std::enable_if<(Unit1First::identifier == Unit2First::identifier && Unit1First::dimension + Unit2First::dimension == 0)>::type>
    {
        typedef typename MultiplyTwoUnitLists<List<Unit1Others...>, List<Unit2Others...>>::Type Type;
    };

    template<std::size_t Identifier>
    struct BaseUnit
        : UnitBase<List<Unit<Identifier, 1>>, StaticRational<1, 1>>
    {};

    struct Identity
        : UnitBase<List<>, StaticRational<1, 1>>
    {};

    template<class UnitType, class StorageType>
    class Quantity
    {
    public:
        Quantity();
        explicit Quantity(StorageType v);
        Quantity(const Quantity& other);
        template<class OtherUnit>                                                                                       //TICS !INT#001
            Quantity(Quantity<OtherUnit, StorageType> other, typename std::enable_if<UnitSame<OtherUnit, UnitType>::value>::type* = 0);

        Quantity& operator=(const Quantity& other);
        template<class OtherUnit>
            typename std::enable_if<UnitSame<OtherUnit, UnitType>::value, Quantity&>::type operator=(const Quantity<OtherUnit, StorageType>& other);

        StorageType Value() const;

        Quantity operator-() const;
        Quantity& operator+=(Quantity other);                                                                           //TICS !INT#008
        Quantity& operator-=(Quantity other);                                                                           //TICS !INT#008
        Quantity& operator*=(StorageType other);
        Quantity& operator/=(StorageType other);

        Quantity<typename UnitType::Inverse, StorageType> InverseDivide(StorageType other);

        template<class UnitTypeOther>
            Quantity<typename UnitType::template Mul<UnitTypeOther>, StorageType> operator*(Quantity<UnitTypeOther, StorageType> other) const;
        template<class UnitTypeOther>
            Quantity<typename UnitType::template Div<UnitTypeOther>, StorageType> operator/(Quantity<UnitTypeOther, StorageType> other) const;

        bool operator==(const Quantity& other) const;
        bool operator!=(const Quantity& other) const;
        bool operator<(const Quantity& other) const;
        bool operator>(const Quantity& other) const;
        bool operator<=(const Quantity& other) const;
        bool operator>=(const Quantity& other) const;

    private:
        template<class UnitTypeOther, class TypeOther>
            friend class Quantity;

        StorageType value;
    };

    template<class UnitType, class StorageType>
    Quantity<UnitType, StorageType>::Quantity()
        : value()
    {}

    template<class UnitType, class StorageType>
    Quantity<UnitType, StorageType>::Quantity(StorageType v)
        : value(v)
    {}

    template<class UnitType, class StorageType>
    Quantity<UnitType, StorageType>::Quantity(const Quantity& other)
        : value(other.value)
    {}

    template<class UnitType, class StorageType>
    template<class OtherUnit>
    Quantity<UnitType, StorageType>::Quantity(Quantity<OtherUnit, StorageType> other, typename std::enable_if<UnitSame<OtherUnit, UnitType>::value>::type*)
        : value(other.value * OtherUnit::Factor::n * UnitType::Factor::d / OtherUnit::Factor::d / UnitType::Factor::n)
    {}

    template<class UnitType, class StorageType>
    Quantity<UnitType, StorageType>& Quantity<UnitType, StorageType>::operator=(const Quantity& other)
    {
        value = other.value;
        return *this;
    }

    template<class UnitType, class StorageType>
    template<class OtherUnit>
    typename std::enable_if<UnitSame<OtherUnit, UnitType>::value, Quantity<UnitType, StorageType>&>::type Quantity<UnitType, StorageType>::operator=(const Quantity<OtherUnit, StorageType>& other)
    {
        value = other.value * OtherUnit::Factor::n * UnitType::Factor::d / OtherUnit::Factor::d / UnitType::Factor::n;
        return *this;
    }

    template<class UnitType, class StorageType>
    StorageType Quantity<UnitType, StorageType>::Value() const
    {
        return value;
    }

    template<class UnitType, class StorageType>
    Quantity<UnitType, StorageType> Quantity<UnitType, StorageType>::operator-() const
    {
        return Quantity<UnitType, StorageType>(-value);
    }

    template<class UnitType, class StorageType>
    Quantity<UnitType, StorageType>& Quantity<UnitType, StorageType>::operator+=(Quantity<UnitType, StorageType> other)
    {
        value += other.value;
        return *this;
    }

    template<class UnitType, class StorageType>
    Quantity<UnitType, StorageType>& Quantity<UnitType, StorageType>::operator-=(Quantity<UnitType, StorageType> other)
    {
        value -= other.value;
        return *this;
    }

    template<class UnitType, class StorageType>
    Quantity<UnitType, StorageType>& Quantity<UnitType, StorageType>::operator*=(StorageType other)
    {
        value *= other;
        return *this;
    }

    template<class UnitType, class StorageType>
    Quantity<UnitType, StorageType>& Quantity<UnitType, StorageType>::operator/=(StorageType other)
    {
        value /= other;
        return *this;
    }

    template<class UnitType, class StorageType>
    Quantity<typename UnitType::Inverse, StorageType> Quantity<UnitType, StorageType>::InverseDivide(StorageType other)
    {
        return Quantity<typename UnitType::Inverse, StorageType>(other / value);
    }

    template<class UnitType, class StorageType>
    Quantity<UnitType, StorageType> operator+(Quantity<UnitType, StorageType> first, Quantity<UnitType, StorageType> second)
    {
        Quantity<UnitType, StorageType> result(first);
        result += second;
        return result;
    }

    template<class UnitType, class StorageType>
    Quantity<UnitType, StorageType> operator-(Quantity<UnitType, StorageType> first, Quantity<UnitType, StorageType> second)
    {
        Quantity<UnitType, StorageType> result(first);
        result -= second;
        return result;
    }

    template<class UnitType, class StorageType>
    template<class UnitTypeOther>
    Quantity<typename UnitType::template Mul<UnitTypeOther>, StorageType> Quantity<UnitType, StorageType>::operator*(Quantity<UnitTypeOther, StorageType> other) const
    {
        return Quantity<typename UnitType::template Mul<UnitTypeOther>, StorageType>(value * other.value);
    }

    template<class UnitType, class StorageType>
    Quantity<UnitType, StorageType> operator*(Quantity<UnitType, StorageType> first, StorageType second)
    {
        Quantity<UnitType, StorageType> result(first);
        result *= second;
        return result;
    }

    template<class UnitType, class StorageType>
    Quantity<UnitType, StorageType> operator*(StorageType first, Quantity<UnitType, StorageType> second)
    {
        Quantity<UnitType, StorageType> result(second);
        result *= first;
        return result;
    }

    template<class UnitType, class StorageType>
    template<class UnitTypeOther>
    Quantity<typename UnitType::template Div<UnitTypeOther>, StorageType> Quantity<UnitType, StorageType>::operator/(Quantity<UnitTypeOther, StorageType> other) const
    {
        return Quantity<typename UnitType::template Div<UnitTypeOther>, StorageType>(value / other.value);
    }

    template<class UnitType, class StorageType>
    Quantity<typename UnitType::Inverse, StorageType> operator/(StorageType value, Quantity<UnitType, StorageType> quantity)
    {
        return quantity.InverseDivide(value);
    }

    template<class UnitType, class StorageType>
    Quantity<UnitType, StorageType> operator/(Quantity<UnitType, StorageType> quantity, StorageType value)
    {
        Quantity<UnitType, StorageType> result(quantity);
        result /= value;
        return result;
    }

    template<class UnitType, class StorageType>
    bool Quantity<UnitType, StorageType>::operator==(const Quantity& other) const
    {
        return value == other.value;
    }

    template<class UnitType, class StorageType>
    bool Quantity<UnitType, StorageType>::operator!=(const Quantity& other) const
    {
        return value != other.value;
    }

    template<class UnitType, class StorageType>
    bool Quantity<UnitType, StorageType>::operator<(const Quantity& other) const
    {
        return value < other.value;
    }

    template<class UnitType, class StorageType>
    bool Quantity<UnitType, StorageType>::operator>(const Quantity& other) const
    {
        return value > other.value;
    }

    template<class UnitType, class StorageType>
    bool Quantity<UnitType, StorageType>::operator<=(const Quantity& other) const
    {
        return value <= other.value;
    }

    template<class UnitType, class StorageType>
    bool Quantity<UnitType, StorageType>::operator>=(const Quantity& other) const
    {
        return value >= other.value;
    }

    typedef BaseUnit<1> Meter;
    typedef Meter::Scale<StaticRational<1, 1000>> MilliMeter;

    typedef BaseUnit<2> Second;
    typedef Second::Scale<StaticRational<60, 1>> Minute;
    typedef Minute::Scale<StaticRational<60, 1>> Hour;
    typedef Hour::Scale<StaticRational<24, 1>> Day;
    typedef Second::Scale<StaticRational<1, 1000>> MilliSecond;
    typedef MilliSecond::Scale<StaticRational<1, 1000>> MicroSecond;

    typedef Second::Div<Meter>::Inverse MeterPerSecond;

    typedef BaseUnit<3> Ohm;
    typedef Ohm::Scale<StaticRational<1000, 1>> KiloOhm;
    typedef BaseUnit<4> Ampere;
    typedef Ampere::Scale<StaticRational<1, 1000>> MilliAmpere;
    typedef MilliAmpere::Scale<StaticRational<1, 1000>> MicroAmpere;
    typedef Ohm::Mul<Ampere> Volt;
    typedef Volt::Scale<StaticRational<1, 1000>> MilliVolt;
    typedef MilliVolt::Scale<StaticRational<1, 1000>> MicroVolt;
    typedef Ampere::Mul<Volt> Watt;
    typedef Watt::Mul<Second> Joule;
    typedef Joule::Scale<StaticRational<3600, 1000>> KiloWattHour;

    typedef BaseUnit<4> Celsius;
}

#endif
