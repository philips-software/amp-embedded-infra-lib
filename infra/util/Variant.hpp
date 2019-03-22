#ifndef INFRA_VARIANT_HPP
#define INFRA_VARIANT_HPP

#include "infra/util/Optional.hpp"
#include "infra/util/VariantDetail.hpp"
#include <type_traits>

namespace infra
{
    struct AtIndex {};
    const AtIndex atIndex;

    template<class... T>
    class Variant
    {
    public:
        static const std::size_t size = sizeof...(T);

        Variant();
        Variant(const Variant& other);
        template<class... T2>                                                                   //TICS !INT#001
            Variant(const Variant<T2...>& other);
        template<class U>                                                                       //TICS !INT#001
            Variant(const U& v, typename std::enable_if<ExistsInTypeList<U, T...>::value>::type* = 0);                                                                //TICS !INT#001
        template<class U, class... Args>
            explicit Variant(InPlaceType<U>, Args&&... args);
        template<class... Args>
            Variant(AtIndex, std::size_t index, Args&&... args);

        Variant& operator=(const Variant& other);
        template<class... T2>
            Variant& operator=(const Variant<T2...>& other);
        template<class U>
            Variant& operator=(const U& v);

        ~Variant();

        template<class U, class... Args>
            U& Emplace(Args&&... args);

        template<class U>
            const U& Get() const;
        template<class U>
            U& Get();

        template<std::size_t Index>
            const typename TypeAtIndex<Index, T...>::Type& GetAtIndex() const;
        template<std::size_t Index>
            typename TypeAtIndex<Index, T...>::Type& GetAtIndex();

        std::size_t Which() const;
        template<class U>
            bool Is() const;

        bool operator==(const Variant& other) const;
        bool operator!=(const Variant& other) const;
        bool operator<(const Variant& other) const;
        bool operator>(const Variant& other) const;
        bool operator<=(const Variant& other) const;
        bool operator>=(const Variant& other) const;

        template<class U>
            typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type operator==(const U& other) const;
        template<class U>
            typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type operator!=(const U& other) const;
        template<class U>
            typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type operator<(const U& other) const;
        template<class U>
            typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type operator>(const U& other) const;
        template<class U>
            typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type operator<=(const U& other) const;
        template<class U>
            typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type operator>=(const U& other) const;

        template<class U, class... Args>
            U& ConstructInEmptyVariant(Args&&... args);

        template<class... Args>
            void ConstructByIndexInEmptyVariant(std::size_t index, Args&&... args);

    private:
        void Destruct();

    private:
        std::size_t dataIndex = 0;
        typename std::aligned_storage<MaxSizeOfTypes<T...>::value, MaxAlignmentOfTypes<T...>::value>::type data;

        template<class... T2>
        friend struct detail::ConstructVisitor;
    };

    template<class... T>
    struct MakeVariantOver;

    template<class Visitor, class Variant>
        typename Visitor::ResultType ApplyVisitor(Visitor& visitor, Variant& variant);
    template<class Visitor, class Variant>
        typename Visitor::ResultType ApplyVisitor(Visitor& visitor, Variant& variant1, Variant& variant2);
    template<class Visitor, class Variant>
        typename Visitor::ResultType ApplySameTypeVisitor(Visitor& visitor, Variant& variant1, Variant& variant2);

    ////    Implementation    ////

    template<class... T>
    Variant<T...>::Variant()
    {
        ConstructInEmptyVariant<typename Front<T...>::Type>();
    }

    template<class... T>
    Variant<T...>::Variant(const Variant& other)
    {
        detail::ConstructVisitor<T...> visitor(*this);
        ApplyVisitor(visitor, other);
    }

    template<class... T>
    template<class... T2>
    Variant<T...>::Variant(const Variant<T2...>& other)
    {
        detail::ConstructVisitor<T...> visitor(*this);
        ApplyVisitor(visitor, other);
    }

    template<class... T>
    template<class U>
    Variant<T...>::Variant(const U& v, typename std::enable_if<ExistsInTypeList<U, T...>::value>::type*)
    {
        ConstructInEmptyVariant<U>(v);
    }

    template<class... T>
    template<class U, class... Args>
    Variant<T...>::Variant(InPlaceType<U>, Args&&... args)
    {
        ConstructInEmptyVariant<U>(std::forward<Args>(args)...);
    }

    template<class... T>
    template<class... Args>
    Variant<T...>::Variant(AtIndex, std::size_t index, Args&&... args)
    {
        ConstructByIndexInEmptyVariant(index, std::forward<Args>(args)...);
    }

    template<class... T>
    Variant<T...>& Variant<T...>::operator=(const Variant& other)
    {
        if (this != &other)                                                                                         //TICS !CON#007
        {
            detail::CopyVisitor<T...> visitor(*this);
            ApplyVisitor(visitor, other);
        }

        return *this;
    }

    template<class... T>
    template<class... T2>
    Variant<T...>& Variant<T...>::operator=(const Variant<T2...>& other)
    {
        detail::CopyVisitor<T...> visitor(*this);
        ApplyVisitor(visitor, other);

        return *this;
    }

    template<class... T>
    template<class U>
    Variant<T...>& Variant<T...>::operator=(const U& v)
    {
        Destruct();
        ConstructInEmptyVariant<U>(v);

        return *this;
    }

    template<class... T>
    template<class U, class... Args>
    U& Variant<T...>::Emplace(Args&&... args)
    {
        Destruct();
        return ConstructInEmptyVariant<U>(std::forward<Args>(args)...);
    }

    template<class... T>
    Variant<T...>::~Variant()
    {
        Destruct();
    }

    template<class... T>
    template<class U>
    const U& Variant<T...>::Get() const
    {
        really_assert((dataIndex == IndexInTypeList<U, T...>::value));
        return reinterpret_cast<const U&>(data);                                                                        //TICS !CON#007
    }

    template<class... T>
    template<class U>
    U& Variant<T...>::Get()
    {
        really_assert((dataIndex == IndexInTypeList<U, T...>::value));
        return reinterpret_cast<U&>(data);                                                                              //TICS !CON#007
    }

    template<class... T>
    template<std::size_t Index>
    const typename TypeAtIndex<Index, T...>::Type& Variant<T...>::GetAtIndex() const
    {
        return Get<typename TypeAtIndex<Index, T...>::Type>();                                                          //TICS !CON#007
    }

    template<class... T>
    template<std::size_t Index>
    typename TypeAtIndex<Index, T...>::Type& Variant<T...>::GetAtIndex()
    {
        return Get<typename TypeAtIndex<Index, T...>::Type>();                                                          //TICS !CON#007
    }

    template<class... T>
    std::size_t Variant<T...>::Which() const
    {
        return dataIndex;
    }

    template<class... T>
    template<class U>
    bool Variant<T...>::Is() const
    {
        return Which() == IndexInTypeList<U, T...>::value;                                                      //TICS !CON#007
    }

    template<class... T>
    bool Variant<T...>::operator==(const Variant& other) const
    {
        if (Which() != other.Which())                                                                           //TICS !CON#007
            return false;

        detail::EqualVisitor visitor;
        return ApplySameTypeVisitor(visitor, *this, other);                                                     //TICS !CON#007
    }

    template<class... T>
    bool Variant<T...>::operator!=(const Variant& other) const
    {
        return !(*this == other);                                                                               //TICS !CON#007
    }

    template<class... T>
    bool Variant<T...>::operator<(const Variant& other) const
    {
        if (Which() != other.Which())
            return Which() < other.Which();

        detail::LessThanVisitor visitor;
        return ApplySameTypeVisitor(visitor, *this, other);
    }

    template<class... T>
    bool Variant<T...>::operator>(const Variant& other) const
    {
        return other < *this;
    }

    template<class... T>
    bool Variant<T...>::operator<=(const Variant& other) const
    {
        return !(other < *this);
    }

    template<class... T>
    bool Variant<T...>::operator>=(const Variant& other) const
    {
        return !(*this < other);
    }

    template<class... T>
    template<class U>
    typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type Variant<T...>::operator==(const U& other) const
    {
        if (Which() != IndexInTypeList<U, T...>::value)
            return false;

        return GetAtIndex<IndexInTypeList<U, T...>::value>() == other;
    }

    template<class... T>
    template<class U>
    typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type Variant<T...>::operator!=(const U& other) const
    {
        return !(*this == other);
    }

    template<class... T>
    template<class U>
    typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type Variant<T...>::operator<(const U& other) const
    {
        if (Which() != IndexInTypeList<U, T...>::value)
            return Which() < IndexInTypeList<U, T...>::value;

        return GetAtIndex<IndexInTypeList<U, T...>::value>() < other;
    }

    template<class... T>
    template<class U>
    typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type Variant<T...>::operator>(const U& other) const
    {
        if (Which() != IndexInTypeList<U, T...>::value)
            return Which() > IndexInTypeList<U, T...>::value;

        return other < GetAtIndex<IndexInTypeList<U, T...>::value>();
    }

    template<class... T>
    template<class U>
    typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type Variant<T...>::operator<=(const U& other) const
    {
        return !(*this > other);
    }

    template<class... T>
    template<class U>
    typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type Variant<T...>::operator>=(const U& other) const
    {
        return !(*this < other);
    }

    template<class... T>
    template<class U, class... Args>
    U& Variant<T...>::ConstructInEmptyVariant(Args&&... args)
    {
        dataIndex = IndexInTypeList<U, T...>::value;
        return *new(&data) U(std::forward<Args>(args)...);                                                                  //TICS !OAL#011: This is placement-new
    }

    template<class... T>
    template<class... Args>
    void Variant<T...>::ConstructByIndexInEmptyVariant(std::size_t index, Args&&... args)
    {
        dataIndex = index;
        detail::ConstructAtIndexHelper<T...>::Construct(*this, index, std::forward<Args>(args)...);
    }

    template<class... T>
    void Variant<T...>::Destruct()                                                                                  //TICS !INT#006
    {
        detail::DestroyVisitor visitor;
        ApplyVisitor(visitor, *this);
    }

    template<class Visitor, class Variant>
    typename Visitor::ResultType ApplyVisitor(Visitor& visitor, Variant& variant)
    {
        detail::ApplyVisitorHelper<0, Visitor, Variant> helper;
        return helper(visitor, variant);
    }

    template<class Visitor, class Variant>
    typename Visitor::ResultType ApplyVisitor(Visitor& visitor, Variant& variant1, Variant& variant2)
    {
        detail::ApplyVisitorHelper2<0, Visitor, Variant> helper;
        return helper(visitor, variant1, variant2);
    }

    template<class Visitor, class Variant>
    typename Visitor::ResultType ApplySameTypeVisitor(Visitor& visitor, Variant& variant1, Variant& variant2)
    {
        really_assert(variant1.Which() == variant2.Which());
        detail::ApplySameTypeVisitorHelper<0, Visitor, Variant> helper;
        return helper(visitor, variant1, variant2);                                                                     //TICS !CON#007
    }

    template<class... T>
    struct MakeVariantOver<List<T...>>
    {
        typedef Variant<T...> Type;
    };

}

#endif
