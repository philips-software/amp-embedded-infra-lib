#ifndef INFRA_COMPARE_MEMBERS_HPP
#define INFRA_COMPARE_MEMBERS_HPP

// This file contains classes which make comparison of objects with many datamembers easier. It is used like this:
//
// bool operator==(const MyClass& x, const MyClass& y)
// {
//     return infra::Equals()
//         (x.a, y.a)
//         (x.b, y.b);
// }
//
// bool operator<(const MyClass& x, const MyClass& y)
// {
//     return infra::LessThan()
//         (x.a, y.a)
//         (x.b, y.b);
// }

namespace infra
{
    class Equals
    {
    public:
        template<class T>
        Equals& operator()(const T& x, const T& y);

        operator bool() const;
        bool operator!() const;

    private:
        bool equal{ true };
    };

    class LessThan
    {
    public:
        template<class T>
        LessThan& operator()(const T& x, const T& y);

        operator bool() const;
        bool operator!() const;

    private:
        bool lessThan{ false };
        bool equal{ true };
    };

    ////    Implementation    ////

    template<class T>
    inline Equals& Equals::operator()(const T& x, const T& y)
    {
        equal &= x == y;
        return *this;
    }

    inline Equals::operator bool() const
    {
        return equal;
    }

    inline bool Equals::operator!() const
    {
        return !equal;
    }

    template<class T>
    inline LessThan& LessThan::operator()(const T& x, const T& y)
    {
        lessThan = lessThan || (equal && (x < y));
        equal = equal && (x == y);

        return *this;
    }

    inline LessThan::operator bool() const
    {
        return lessThan;
    }

    inline bool LessThan::operator!() const
    {
        return !lessThan;
    }
}

#endif
