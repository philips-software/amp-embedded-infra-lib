#ifndef INFRA_POLYMORPHIC_VARIANT_HPP
#define INFRA_POLYMORPHIC_VARIANT_HPP

#include "infra/util/StaticStorage.hpp"
#include "infra/util/Variant.hpp"

namespace infra
{
    template<class Base, class... T>
    class PolymorphicVariant
    {
    public:
        static const std::size_t size = sizeof...(T);

        PolymorphicVariant();
        PolymorphicVariant(const PolymorphicVariant& other);
        template<class... T2>
            explicit PolymorphicVariant(const PolymorphicVariant<T2...>& other);
        template<class U>
            explicit PolymorphicVariant(const U& v);
        template<class U, class... Args>
            PolymorphicVariant(InPlaceType<U>, Args&&... args);

        PolymorphicVariant& operator=(const PolymorphicVariant& other);
        template<class... T2>
            PolymorphicVariant& operator=(const PolymorphicVariant<T2...>& other);
        template<class U>
            PolymorphicVariant& operator=(const U& v);

        ~PolymorphicVariant();

        template<class U, class... Args>
            U& Emplace(Args&&... args);

        const Base& Get() const;
        Base& Get();

        const Base& operator*() const;
        Base& operator*();
        const Base* operator->() const;
        Base* operator->();

        bool operator==(const PolymorphicVariant& other) const;
        bool operator!=(const PolymorphicVariant& other) const;
        bool operator<(const PolymorphicVariant& other) const;
        bool operator>(const PolymorphicVariant& other) const;
        bool operator<=(const PolymorphicVariant& other) const;
        bool operator>=(const PolymorphicVariant& other) const;

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

    private:
        template<std::size_t Index, class Visitor, class Variant, class Enable>
            friend struct detail::ApplyVisitorHelper;
        template<std::size_t Index, class Visitor, class Variant, class Enable>
            friend struct detail::ApplySameTypeVisitorHelper;

        template<class Visitor, class Variant>
            friend typename Visitor::ResultType ApplySameTypeVisitor(Visitor& visitor, Variant& variant1, Variant& variant2);

        std::size_t Which() const;

        template<std::size_t Index>
            const typename TypeAtIndex<Index, T...>::Type& GetAtIndex() const;
        template<std::size_t Index>
            typename TypeAtIndex<Index, T...>::Type& GetAtIndex();

    private:
        template<class Base2, class... T2>
        friend struct detail::ConstructPolymorphicVisitor;

        template<class U, class... Args>
            U& ConstructInEmptyVariant(Args&&... args);
        void Destruct();

    private:
        std::size_t dataIndex = 0;
        StaticStorageForInheritanceTree<Base, T...> storage;
    };

   
    template<class Base, class... T>
    struct MakePolymorphicVariantOver;

    ////    Implementation    ////

    template<class Base, class... T>
    PolymorphicVariant<Base, T...>::PolymorphicVariant()
    {
        ConstructInEmptyVariant<typename Front<T...>::Type>();
    }

    template<class Base, class... T>
    PolymorphicVariant<Base, T...>::PolymorphicVariant(const PolymorphicVariant& other)
    {
        detail::ConstructPolymorphicVisitor<Base, T...> visitor(*this);
        ApplyVisitor(visitor, other);
    }

    template<class Base, class... T>
    template<class... T2>
    PolymorphicVariant<Base, T...>::PolymorphicVariant(const PolymorphicVariant<T2...>& other)
    {
        detail::ConstructPolymorphicVisitor<Base, T...> visitor(*this);
        ApplyVisitor(visitor, other);
    }

    template<class Base, class... T>
    template<class U>
    PolymorphicVariant<Base, T...>::PolymorphicVariant(const U& v)
    {
        ConstructInEmptyVariant<U>(v);
    }

    template<class Base, class... T>
    template<class U, class... Args>
    PolymorphicVariant<Base, T...>::PolymorphicVariant(InPlaceType<U>, Args&&... args)
    {
        ConstructInEmptyVariant<U>(std::forward<Args>(args)...);
    }

    template<class Base, class... T>
    PolymorphicVariant<Base, T...>& PolymorphicVariant<Base, T...>::operator=(const PolymorphicVariant& other)
    {
        if (this != &other)
        {
            detail::CopyPolymorphicVisitor<Base, T...> visitor(*this);
            ApplyVisitor(visitor, other);
        }

        return *this;
    }

    template<class Base, class... T>
    template<class... T2>
    PolymorphicVariant<Base, T...>& PolymorphicVariant<Base, T...>::operator=(const PolymorphicVariant<T2...>& other)
    {
        detail::CopyPolymorphicVisitor<Base, T...> visitor(*this);
        ApplyVisitor(visitor, other);

        return *this;
    }

    template<class Base, class... T>
    template<class U>
    PolymorphicVariant<Base, T...>& PolymorphicVariant<Base, T...>::operator=(const U& v)
    {
        Destruct();
        ConstructInEmptyVariant<U>(v);

        return *this;
    }

    template<class Base, class... T>
    template<class U, class... Args>
    U& PolymorphicVariant<Base, T...>::Emplace(Args&&... args)
    {
        Destruct();
        return ConstructInEmptyVariant<U>(std::forward<Args>(args)...);
    }

    template<class Base, class... T>
    PolymorphicVariant<Base, T...>::~PolymorphicVariant()
    {
        Destruct();
    }

    template<class Base, class... T>
    const Base& PolymorphicVariant<Base, T...>::Get() const
    {
        return *storage;
    }

    template<class Base, class... T>
    Base& PolymorphicVariant<Base, T...>::Get()
    {
        return *storage;
    }

    template<class Base, class... T>
    const Base& PolymorphicVariant<Base, T...>::operator*() const
    {
        return *storage;
    }

    template<class Base, class... T>
    Base& PolymorphicVariant<Base, T...>::operator*()
    {
        return *storage;
    }

    template<class Base, class... T>
    const Base* PolymorphicVariant<Base, T...>::operator->() const
    {
        return &*storage;
    }

    template<class Base, class... T>
    Base* PolymorphicVariant<Base, T...>::operator->()
    {
        return &*storage;
    }

    template<class Base, class... T>
    bool PolymorphicVariant<Base, T...>::operator==(const PolymorphicVariant& other) const
    {
        if (Which() != other.Which())
            return false;

        detail::EqualVisitor visitor;
        return ApplySameTypeVisitor(visitor, *this, other);
    }

    template<class Base, class... T>
    bool PolymorphicVariant<Base, T...>::operator!=(const PolymorphicVariant& other) const
    {
        return !(*this == other);
    }

    template<class Base, class... T>
    bool PolymorphicVariant<Base, T...>::operator<(const PolymorphicVariant& other) const
    {
        if (Which() != other.Which())
            return Which() < other.Which();

        detail::LessThanVisitor visitor;
        return ApplySameTypeVisitor(visitor, *this, other);
    }

    template<class Base, class... T>
    bool PolymorphicVariant<Base, T...>::operator>(const PolymorphicVariant& other) const
    {
        return other < *this;
    }

    template<class Base, class... T>
    bool PolymorphicVariant<Base, T...>::operator<=(const PolymorphicVariant& other) const
    {
        return !(other < *this);
    }

    template<class Base, class... T>
    bool PolymorphicVariant<Base, T...>::operator>=(const PolymorphicVariant& other) const
    {
        return !(*this < other);
    }

    template<class Base, class... T>
    template<class U>
    typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type PolymorphicVariant<Base, T...>::operator==(const U& other) const
    {
        if (Which() != IndexInTypeList<U, T...>::value)
            return false;

        return GetAtIndex<IndexInTypeList<U, T...>::value>() == other;
    }

    template<class Base, class... T>
    template<class U>
    typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type PolymorphicVariant<Base, T...>::operator!=(const U& other) const
    {
        return !(*this == other);
    }

    template<class Base, class... T>
    template<class U>
    typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type PolymorphicVariant<Base, T...>::operator<(const U& other) const
    {
        if (Which() != IndexInTypeList<U, T...>::value)
            return Which() < IndexInTypeList<U, T...>::value;

        return GetAtIndex<IndexInTypeList<U, T...>::value>() < other;
    }

    template<class Base, class... T>
    template<class U>
    typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type PolymorphicVariant<Base, T...>::operator>(const U& other) const
    {
        if (Which() != IndexInTypeList<U, T...>::value)
            return Which() > IndexInTypeList<U, T...>::value;

        return other < GetAtIndex<IndexInTypeList<U, T...>::value>();
    }

    template<class Base, class... T>
    template<class U>
    typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type PolymorphicVariant<Base, T...>::operator<=(const U& other) const
    {
        return !(*this > other);
    }

    template<class Base, class... T>
    template<class U>
    typename std::enable_if<ExistsInTypeList<U, T...>::value, bool>::type PolymorphicVariant<Base, T...>::operator>=(const U& other) const
    {
        return !(*this < other);
    }

    template<class Base, class... T>
    std::size_t PolymorphicVariant<Base, T...>::Which() const
    {
        return dataIndex;
    }

    template<class Base, class... T>
    template<std::size_t Index>
    const typename TypeAtIndex<Index, T...>::Type& PolymorphicVariant<Base, T...>::GetAtIndex() const
    {
        return static_cast<const typename TypeAtIndex<Index, T...>::Type&>(Get());
    }

    template<class Base, class... T>
    template<std::size_t Index>
    typename TypeAtIndex<Index, T...>::Type& PolymorphicVariant<Base, T...>::GetAtIndex()
    {
        return static_cast<typename TypeAtIndex<Index, T...>::Type&>(Get());
    }

    template<class Base, class... T>
    template<class U, class... Args>
    U& PolymorphicVariant<Base, T...>::ConstructInEmptyVariant(Args&&... args)
    {
        dataIndex = IndexInTypeList<U, T...>::value;
        storage.template Construct<U>(std::forward<Args>(args)...);
        return reinterpret_cast<U&>(*storage);
    }

    template<class Base, class... T>
    void PolymorphicVariant<Base, T...>::Destruct()
    {
        storage.Destruct();
    }

    template<class Base, class... T>
    struct MakePolymorphicVariantOver<Base, List<T...>>
    {
        typedef PolymorphicVariant<Base, T...> Type;
    };

}

#endif
