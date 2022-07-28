#ifndef INFRA_FIXED_POINT_HPP
#define INFRA_FIXED_POINT_HPP

namespace infra
{
    struct Scaled {};
    const Scaled scaled;
    struct Unscaled {};
    const Unscaled unscaled;

    template<class T, T Factor>
    struct FixedPoint
    {
        FixedPoint() = default;
        FixedPoint(T integerPart, T fractionalPart);
        FixedPoint(Scaled, T v);
        FixedPoint(Unscaled, T v);

        T AsUnscaled() const;
        T IntegerPart() const;
        T FractionalPart() const;
        T Rounded() const;

        FixedPoint& operator+=(const FixedPoint& x);
        FixedPoint& operator-=(const FixedPoint& x);
        FixedPoint& operator*=(const FixedPoint& x);
        FixedPoint& operator/=(const FixedPoint& x);
        FixedPoint& operator*=(T x);
        FixedPoint& operator/=(T x);
        FixedPoint operator-() const;

    private:
        T value = 0;
    };

    ////    Implementation    ////

    template<class T, T Factor>
    FixedPoint<T, Factor>::FixedPoint(T integerPart, T fractionalPart)
        : value(integerPart * Factor + fractionalPart)
    {}

    template<class T, T Factor>
    FixedPoint<T, Factor>::FixedPoint(Scaled, T v)
        : value(v * Factor)
    {}

    template<class T, T Factor>
    FixedPoint<T, Factor>::FixedPoint(Unscaled, T v)
        : value(v)
    {}

    template<class T, T Factor>
    T FixedPoint<T, Factor>::AsUnscaled() const { return value; }
    template<class T, T Factor>
    T FixedPoint<T, Factor>::IntegerPart() const { return value / Factor; }
    template<class T, T Factor>
    T FixedPoint<T, Factor>::FractionalPart() const { return value % Factor; }
    template<class T, T Factor>
    T FixedPoint<T, Factor>::Rounded() const { return value >= 0 ? (value + Factor / 2) / Factor : (value - Factor / 2) / Factor; }

    template<class T, T Factor>
    FixedPoint<T, Factor>& FixedPoint<T, Factor>::operator+=(const FixedPoint& x) { value += x.value; return *this; }
    template<class T, T Factor>
    FixedPoint<T, Factor>& FixedPoint<T, Factor>::operator-=(const FixedPoint& x) { value -= x.value; return *this; }
    template<class T, T Factor>
    FixedPoint<T, Factor>& FixedPoint<T, Factor>::operator*=(const FixedPoint& x) { value *= x.value; value /= Factor; return *this; }
    template<class T, T Factor>
    FixedPoint<T, Factor>& FixedPoint<T, Factor>::operator/=(const FixedPoint& x) { value *= Factor; value /= x.value; return *this; }
    template<class T, T Factor>
    FixedPoint<T, Factor>& FixedPoint<T, Factor>::operator*=(T x) { value *= x; return *this; }
    template<class T, T Factor>
    FixedPoint<T, Factor>& FixedPoint<T, Factor>::operator/=(T x) { value /= x; return *this; }
    template<class T, T Factor>
    FixedPoint<T, Factor> FixedPoint<T, Factor>::operator-() const { FixedPoint result; result.value = -value; return result; }

    template<class T, T Factor>
    FixedPoint<T, Factor> operator+(FixedPoint<T, Factor> x, const FixedPoint<T, Factor>& y) { return x += y; }
    template<class T, T Factor>
    FixedPoint<T, Factor> operator-(FixedPoint<T, Factor> x, const FixedPoint<T, Factor>& y) { return x -= y; }
    template<class T, T Factor>
    FixedPoint<T, Factor> operator*(FixedPoint<T, Factor> x, const FixedPoint<T, Factor>& y) { return x *= y; }
    template<class T, T Factor>
    FixedPoint<T, Factor> operator/(FixedPoint<T, Factor> x, const FixedPoint<T, Factor>& y) { return x /= y; }
    template<class T, T Factor>
    FixedPoint<T, Factor> operator*(FixedPoint<T, Factor> x, T y) { return x *= y; }
    template<class T, T Factor>
    FixedPoint<T, Factor> operator*(T x, FixedPoint<T, Factor> y) { return y *= x; }
    template<class T, T Factor>
    FixedPoint<T, Factor> operator/(FixedPoint<T, Factor> x, T y) { return x /= y; }
    template<class T, T Factor>
    FixedPoint<T, Factor> operator/(T x, FixedPoint<T, Factor> y) { return FixedPoint<T, Factor>(scaled, x) /= y; }

    template<class T, T Factor>
    bool operator==(const FixedPoint<T, Factor>& x, const FixedPoint<T, Factor>& y) { return x.AsUnscaled() == y.AsUnscaled(); }
    template<class T, T Factor>
    bool operator!=(const FixedPoint<T, Factor>& x, const FixedPoint<T, Factor>& y) { return !(x == y); }
    template<class T, T Factor>
    bool operator<(const FixedPoint<T, Factor>& x, const FixedPoint<T, Factor>& y) { return x.AsUnscaled() < y.AsUnscaled(); }
    template<class T, T Factor>
    bool operator>(const FixedPoint<T, Factor>& x, const FixedPoint<T, Factor>& y) { return y < x; }
    template<class T, T Factor>
    bool operator<=(const FixedPoint<T, Factor>& x, const FixedPoint<T, Factor>& y) { return !(y < x); }
    template<class T, T Factor>
    bool operator>=(const FixedPoint<T, Factor>& x, const FixedPoint<T, Factor>& y) { return !(x < y); }
    template<class T, T Factor>
    bool operator==(const FixedPoint<T, Factor>& x, const T& y) { return x.AsUnscaled() == y * Factor; }
    template<class T, T Factor>
    bool operator!=(const FixedPoint<T, Factor>& x, const T& y) { return x.AsUnscaled() != y * Factor; }
    template<class T, T Factor>
    bool operator<(const FixedPoint<T, Factor>& x, const T& y) { return x.AsUnscaled() < y * Factor; }
    template<class T, T Factor>
    bool operator>(const FixedPoint<T, Factor>& x, const T& y) { return x.AsUnscaled() > y * Factor; }
    template<class T, T Factor>
    bool operator<=(const FixedPoint<T, Factor>& x, const T& y) { return x.AsUnscaled() <= y * Factor; }
    template<class T, T Factor>
    bool operator>=(const FixedPoint<T, Factor>& x, const T& y) { return x.AsUnscaled() >= y * Factor; }
    template<class T, T Factor>
    bool operator==(const T& x, const FixedPoint<T, Factor>& y) { return x * Factor == y.AsUnscaled(); }
    template<class T, T Factor>
    bool operator!=(const T& x, const FixedPoint<T, Factor>& y) { return x * Factor != y.AsUnscaled(); }
    template<class T, T Factor>
    bool operator<(const T& x, const FixedPoint<T, Factor>& y) { return x * Factor < y.AsUnscaled(); }
    template<class T, T Factor>
    bool operator>(const T& x, const FixedPoint<T, Factor>& y) { return x * Factor > y.AsUnscaled(); }
    template<class T, T Factor>
    bool operator<=(const T& x, const FixedPoint<T, Factor>& y) { return x * Factor <= y.AsUnscaled(); }
    template<class T, T Factor>
    bool operator>=(const T& x, const FixedPoint<T, Factor>& y) { return x * Factor >= y.AsUnscaled(); }
}

#endif
