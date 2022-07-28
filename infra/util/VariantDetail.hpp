#ifndef INFRA_VARIANT_DETAIL_HPP
#define INFRA_VARIANT_DETAIL_HPP

#include "infra/util/ReallyAssert.hpp"
#include "infra/util/VariadicTemplates.hpp"
#include <cstdlib>
#include <type_traits>
#include <utility>

namespace infra
{
    template<class R>
    class StaticVisitor
    {
    protected:
        StaticVisitor() = default;
        ~StaticVisitor() = default;

    public:
        typedef R ResultType;
    };

    template<class... T>
    class Variant;

    template<class Base, class... T>
    class PolymorphicVariant;

    namespace detail
    {
        template<std::size_t Index, class Visitor, class Variant, class Enable = void>
        struct ApplyVisitorHelper
        {
            typename Visitor::ResultType operator()(Visitor& visitor, Variant& variant);
        };

        template<std::size_t Index, class Visitor, class Variant, class Enable = void>
        struct ApplyVisitorHelper2
        {
            typename Visitor::ResultType operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2);

            template<class T>
                typename Visitor::ResultType VisitSecond(Visitor& visitor, const T& v1, const Variant& variant2);
        };

        template<std::size_t Index, class Visitor, class Variant, class Enable = void>
        struct ApplySameTypeVisitorHelper
        {
            typename Visitor::ResultType operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2);
        };

        template<class... T>
        struct ConstructVisitor
            : StaticVisitor<void>
        {
            explicit ConstructVisitor(Variant<T...>& aVariant);

            template<class T2>
                void operator()(const T2& v);

        private:
            Variant<T...>& variant;
        };

        template<class... T>
        struct CopyVisitor
            : StaticVisitor<void>
        {
            explicit CopyVisitor(Variant<T...>& aVariant);

            template<class T2>
                void operator()(const T2& v);

        private:
            Variant<T...>& variant;
        };

        struct DestroyVisitor
            : StaticVisitor<void>
        {
            template<class T>
                void operator()(T& x);
        };

        struct EqualVisitor
            : public StaticVisitor<bool>
        {
            template<class T>
                bool operator()(const T& x, const T& y) const;
            template<class T, class U>
                bool operator()(const T& x, const U& y) const;
        };

        struct LessThanVisitor
            : StaticVisitor<bool>
        {
            template<class T>
                bool operator()(const T& x, const T& y) const;
            template<class T, class U>
                bool operator()(const T& x, const U& y) const;
        };

        template<class... Args>
        struct ConstructAtIndexHelper;

        template<class Base, class... T>
        struct ConstructPolymorphicVisitor
            : StaticVisitor<void>
        {
            explicit ConstructPolymorphicVisitor(PolymorphicVariant<Base, T...>& aVariant);

            template<class T2>
                void operator()(const T2& v);

        private:
            PolymorphicVariant<Base, T...>& variant;
        };

        template<class Base, class... T>
        struct CopyPolymorphicVisitor
            : StaticVisitor<void>
        {
            explicit CopyPolymorphicVisitor(PolymorphicVariant<Base, T...>& aVariant);

            template<class T2>
                void operator()(const T2& v);

        private:
            PolymorphicVariant<Base, T...>& variant;
        };

        ////    Implementation    ////

        template<std::size_t Index, class Visitor, class Variant>
        struct ApplyVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index == Variant::size>::type>
        {
            typename Visitor::ResultType operator()(Visitor& visitor, Variant& variant);
        };

        template<std::size_t Index, class Visitor, class Variant>
        struct ApplyVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index + 1 == Variant::size>::type>
        {
            typename Visitor::ResultType operator()(Visitor& visitor, Variant& variant);
        };

        template<std::size_t Index, class Visitor, class Variant>
        struct ApplyVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index + 2 == Variant::size>::type>
        {
            typename Visitor::ResultType operator()(Visitor& visitor, Variant& variant);
        };

        template<std::size_t Index, class Visitor, class Variant>
        struct ApplyVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index + 3 == Variant::size>::type>
        {
            typename Visitor::ResultType operator()(Visitor& visitor, Variant& variant);
        };

        template<std::size_t Index, class Visitor, class Variant>
        struct ApplyVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index + 4 == Variant::size>::type>
        {
            typename Visitor::ResultType operator()(Visitor& visitor, Variant& variant);
        };

        template<std::size_t Index, class Visitor, class Variant, class Enable>
        typename Visitor::ResultType ApplyVisitorHelper<Index, Visitor, Variant, Enable>::operator()(Visitor& visitor, Variant& variant)
        {
            if (variant.Which() == Index)
                return visitor(variant.template GetAtIndex<Index>());
            if (variant.Which() == Index + 1)
                return visitor(variant.template GetAtIndex<Index + 1>());
            if (variant.Which() == Index + 2)
                return visitor(variant.template GetAtIndex<Index + 2>());
            if (variant.Which() == Index + 3)
                return visitor(variant.template GetAtIndex<Index + 3>());
            if (variant.Which() == Index + 4)
                return visitor(variant.template GetAtIndex<Index + 4>());

            return ApplyVisitorHelper<Index + 5, Visitor, Variant>()(visitor, variant);
        }

        template<std::size_t Index, class Visitor, class Variant>
        typename Visitor::ResultType ApplyVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index == Variant::size>::type>::operator()(Visitor& visitor, Variant& variant)
        {
            std::abort();
        }
        
        template<std::size_t Index, class Visitor, class Variant>
        typename Visitor::ResultType ApplyVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index + 1 == Variant::size>::type>::operator()(Visitor& visitor, Variant& variant)
        {
            assert(variant.Which() == Index);

            return visitor(variant.template GetAtIndex<Index>());
        }

        template<std::size_t Index, class Visitor, class Variant>
        typename Visitor::ResultType ApplyVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index + 2 == Variant::size>::type>::operator()(Visitor& visitor, Variant& variant)
        {
            assert(variant.Which() >= Index && variant.Which() < Index + 2);
            if (variant.Which() == Index)
                return visitor(variant.template GetAtIndex<Index>());
            return visitor(variant.template GetAtIndex<Index + 1>());
        }

        template<std::size_t Index, class Visitor, class Variant>
        typename Visitor::ResultType ApplyVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index + 3 == Variant::size>::type>::operator()(Visitor& visitor, Variant& variant)
        {
            assert(variant.Which() >= Index && variant.Which() < Index + 3);
            if (variant.Which() == Index)
                return visitor(variant.template GetAtIndex<Index>());
            if (variant.Which() == Index + 1)
                return visitor(variant.template GetAtIndex<Index + 1>());
            return visitor(variant.template GetAtIndex<Index + 2>());
        }

        template<std::size_t Index, class Visitor, class Variant>
        typename Visitor::ResultType ApplyVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index + 4 == Variant::size>::type>::operator()(Visitor& visitor, Variant& variant)
        {
            assert(variant.Which() >= Index && variant.Which() < Index + 4);
            if (variant.Which() == Index)
                return visitor(variant.template GetAtIndex<Index>());
            if (variant.Which() == Index + 1)
                return visitor(variant.template GetAtIndex<Index + 1>());
            if (variant.Which() == Index + 2)
                return visitor(variant.template GetAtIndex<Index + 2>());
            return visitor(variant.template GetAtIndex<Index + 3>());
        }

        template<std::size_t Index, class Visitor, class Variant>
        struct ApplyVisitorHelper2<Index, Visitor, Variant, typename std::enable_if<Index == Variant::size>::type>
        {
            typename Visitor::ResultType operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2);

            template<class T>
                typename Visitor::ResultType VisitSecond(Visitor& visitor, const T& v1, const Variant& variant2);
        };

        template<std::size_t Index, class Visitor, class Variant>
        struct ApplyVisitorHelper2<Index, Visitor, Variant, typename std::enable_if<Index + 1 == Variant::size>::type>
        {
            typename Visitor::ResultType operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2);

            template<class T>
                typename Visitor::ResultType VisitSecond(Visitor& visitor, const T& v1, const Variant& variant2);
        };

        template<std::size_t Index, class Visitor, class Variant>
        struct ApplyVisitorHelper2<Index, Visitor, Variant, typename std::enable_if<Index + 2 == Variant::size>::type>
        {
            typename Visitor::ResultType operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2);

            template<class T>
                typename Visitor::ResultType VisitSecond(Visitor& visitor, const T& v1, const Variant& variant2);
        };

        template<std::size_t Index, class Visitor, class Variant>
        struct ApplyVisitorHelper2<Index, Visitor, Variant, typename std::enable_if<Index + 3 == Variant::size>::type>
        {
            typename Visitor::ResultType operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2);

            template<class T>
                typename Visitor::ResultType VisitSecond(Visitor& visitor, const T& v1, const Variant& variant2);
        };

        template<std::size_t Index, class Visitor, class Variant>
        struct ApplyVisitorHelper2<Index, Visitor, Variant, typename std::enable_if<Index + 4 == Variant::size>::type>
        {
            typename Visitor::ResultType operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2);

            template<class T>
            typename Visitor::ResultType VisitSecond(Visitor& visitor, const T& v1, const Variant& variant2);
        };

        template<std::size_t Index, class Visitor, class Variant, class Enable>
        typename Visitor::ResultType ApplyVisitorHelper2<Index, Visitor, Variant, Enable>::operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2)
        {
            if (variant1.Which() == Index)
                return ApplyVisitorHelper2<0, Visitor, Variant>().VisitSecond(visitor, variant1.template GetAtIndex<Index>(), variant2);
            if (variant1.Which() == Index + 1)
                return ApplyVisitorHelper2<0, Visitor, Variant>().VisitSecond(visitor, variant1.template GetAtIndex<Index + 1>(), variant2);
            if (variant1.Which() == Index + 2)
                return ApplyVisitorHelper2<0, Visitor, Variant>().VisitSecond(visitor, variant1.template GetAtIndex<Index + 2>(), variant2);
            if (variant1.Which() == Index + 3)
                return ApplyVisitorHelper2<0, Visitor, Variant>().VisitSecond(visitor, variant1.template GetAtIndex<Index + 3>(), variant2);
            if (variant1.Which() == Index + 4)
                return ApplyVisitorHelper2<0, Visitor, Variant>().VisitSecond(visitor, variant1.template GetAtIndex<Index + 4>(), variant2);

            return ApplyVisitorHelper2<Index + 5, Visitor, Variant>()(visitor, variant1, variant2);
        }

        template<std::size_t Index, class Visitor, class Variant, class Enable>
        template<class T>
        typename Visitor::ResultType ApplyVisitorHelper2<Index, Visitor, Variant, Enable>::VisitSecond(Visitor& visitor, const T& v1, const Variant& variant2)
        {
            if (variant2.Which() == Index)
                return visitor(v1, variant2.template GetAtIndex<Index>());
            if (variant2.Which() == Index + 1)
                return visitor(v1, variant2.template GetAtIndex<Index + 1>());
            if (variant2.Which() == Index + 2)
                return visitor(v1, variant2.template GetAtIndex<Index + 2>());
            if (variant2.Which() == Index + 3)
                return visitor(v1, variant2.template GetAtIndex<Index + 3>());
            if (variant2.Which() == Index + 4)
                return visitor(v1, variant2.template GetAtIndex<Index + 4>());

            return ApplyVisitorHelper2<Index + 5, Visitor, Variant>().VisitSecond(visitor, v1, variant2);
        }

        template<std::size_t Index, class Visitor, class Variant>
        typename Visitor::ResultType ApplyVisitorHelper2<Index, Visitor, Variant, typename std::enable_if<Index == Variant::size>::type>::operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2)
        {
            std::abort();
        }

        template<std::size_t Index, class Visitor, class Variant>
        template<class T>
        typename Visitor::ResultType ApplyVisitorHelper2<Index, Visitor, Variant, typename std::enable_if<Index == Variant::size>::type>::VisitSecond(Visitor& visitor, const T& v1, const Variant& variant2)
        {
            std::abort();
        }

        template<std::size_t Index, class Visitor, class Variant>
        typename Visitor::ResultType ApplyVisitorHelper2<Index, Visitor, Variant, typename std::enable_if<Index + 1 == Variant::size>::type>::operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2)
        {
            if (variant1.Which() == Index)
                return ApplyVisitorHelper2<0, Visitor, Variant>().VisitSecond(visitor, variant1.template GetAtIndex<Index>(), variant2);

            std::abort();
        }

        template<std::size_t Index, class Visitor, class Variant>
        template<class T>
        typename Visitor::ResultType ApplyVisitorHelper2<Index, Visitor, Variant, typename std::enable_if<Index + 1 == Variant::size>::type>::VisitSecond(Visitor& visitor, const T& v1, const Variant& variant2)
        {
            assert(variant2.Which() == Index);
            return visitor(v1, variant2.template GetAtIndex<Index>());
        }

        template<std::size_t Index, class Visitor, class Variant>
        typename Visitor::ResultType ApplyVisitorHelper2<Index, Visitor, Variant, typename std::enable_if<Index + 2 == Variant::size>::type>::operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2)
        {
            assert(variant1.Which() >= Index && variant1.Which() < Index + 2);
            if (variant1.Which() == Index)
                return ApplyVisitorHelper2<0, Visitor, Variant>().VisitSecond(visitor, variant1.template GetAtIndex<Index>(), variant2);
            return ApplyVisitorHelper2<0, Visitor, Variant>().VisitSecond(visitor, variant1.template GetAtIndex<Index + 1>(), variant2);
        }

        template<std::size_t Index, class Visitor, class Variant>
        template<class T>
        typename Visitor::ResultType ApplyVisitorHelper2<Index, Visitor, Variant, typename std::enable_if<Index + 2 == Variant::size>::type>::VisitSecond(Visitor& visitor, const T& v1, const Variant& variant2)
        {
            assert(variant2.Which() >= Index && variant2.Which() < Index + 2);
            if (variant2.Which() == Index)
                return visitor(v1, variant2.template GetAtIndex<Index>());
            return visitor(v1, variant2.template GetAtIndex<Index + 1>());
        }

        template<std::size_t Index, class Visitor, class Variant>
        typename Visitor::ResultType ApplyVisitorHelper2<Index, Visitor, Variant, typename std::enable_if<Index + 3 == Variant::size>::type>::operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2)
        {
            assert(variant1.Which() >= Index && variant1.Which() < Index + 3);
            if (variant1.Which() == Index)
                return ApplyVisitorHelper2<0, Visitor, Variant>().VisitSecond(visitor, variant1.template GetAtIndex<Index>(), variant2);
            if (variant1.Which() == Index + 1)
                return ApplyVisitorHelper2<0, Visitor, Variant>().VisitSecond(visitor, variant1.template GetAtIndex<Index + 1>(), variant2);
            return ApplyVisitorHelper2<0, Visitor, Variant>().VisitSecond(visitor, variant1.template GetAtIndex<Index + 2>(), variant2);
        }

        template<std::size_t Index, class Visitor, class Variant>
        template<class T>
        typename Visitor::ResultType ApplyVisitorHelper2<Index, Visitor, Variant, typename std::enable_if<Index + 3 == Variant::size>::type>::VisitSecond(Visitor& visitor, const T& v1, const Variant& variant2)
        {
            assert(variant2.Which() >= Index && variant2.Which() < Index + 3);
            if (variant2.Which() == Index)
                return visitor(v1, variant2.template GetAtIndex<Index>());
            if (variant2.Which() == Index + 1)
                return visitor(v1, variant2.template GetAtIndex<Index + 1>());
            return visitor(v1, variant2.template GetAtIndex<Index + 2>());
        }

        template<std::size_t Index, class Visitor, class Variant>
        typename Visitor::ResultType ApplyVisitorHelper2<Index, Visitor, Variant, typename std::enable_if<Index + 4 == Variant::size>::type>::operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2)
        {
            assert(variant1.Which() >= Index && variant1.Which() < Index + 4);
            if (variant1.Which() == Index)
                return ApplyVisitorHelper2<0, Visitor, Variant>().VisitSecond(visitor, variant1.template GetAtIndex<Index>(), variant2);
            if (variant1.Which() == Index + 1)
                return ApplyVisitorHelper2<0, Visitor, Variant>().VisitSecond(visitor, variant1.template GetAtIndex<Index + 1>(), variant2);
            if (variant1.Which() == Index + 2)
                return ApplyVisitorHelper2<0, Visitor, Variant>().VisitSecond(visitor, variant1.template GetAtIndex<Index + 2>(), variant2);
            return ApplyVisitorHelper2<0, Visitor, Variant>().VisitSecond(visitor, variant1.template GetAtIndex<Index + 3>(), variant2);
        }

        template<std::size_t Index, class Visitor, class Variant>
        template<class T>
        typename Visitor::ResultType ApplyVisitorHelper2<Index, Visitor, Variant, typename std::enable_if<Index + 4 == Variant::size>::type>::VisitSecond(Visitor& visitor, const T& v1, const Variant& variant2)
        {
            assert(variant2.Which() >= Index && variant2.Which() < Index + 4);
            if (variant2.Which() == Index)
                return visitor(v1, variant2.template GetAtIndex<Index>());
            if (variant2.Which() == Index + 1)
                return visitor(v1, variant2.template GetAtIndex<Index + 1>());
            if (variant2.Which() == Index + 2)
                return visitor(v1, variant2.template GetAtIndex<Index + 2>());
            return visitor(v1, variant2.template GetAtIndex<Index + 3>());
        }

        template<std::size_t Index, class Visitor, class Variant>
        struct ApplySameTypeVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index == Variant::size>::type>
        {
            typename Visitor::ResultType operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2);
        };

        template<std::size_t Index, class Visitor, class Variant>
        struct ApplySameTypeVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index + 1 == Variant::size>::type>
        {
            typename Visitor::ResultType operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2);
        };

        template<std::size_t Index, class Visitor, class Variant>
        struct ApplySameTypeVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index + 2 == Variant::size>::type>
        {
            typename Visitor::ResultType operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2);
        };

        template<std::size_t Index, class Visitor, class Variant>
        struct ApplySameTypeVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index + 3 == Variant::size>::type>
        {
            typename Visitor::ResultType operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2);
        };

        template<std::size_t Index, class Visitor, class Variant>
        struct ApplySameTypeVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index + 4 == Variant::size>::type>
        {
            typename Visitor::ResultType operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2);
        };

        template<std::size_t Index, class Visitor, class Variant, class Enable>
        typename Visitor::ResultType ApplySameTypeVisitorHelper<Index, Visitor, Variant, Enable>::operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2)
        {
            if (variant1.Which() == Index)
                return visitor(variant1.template GetAtIndex<Index>(), variant2.template GetAtIndex<Index>());
            if (variant1.Which() == Index + 1)
                return visitor(variant1.template GetAtIndex<Index + 1>(), variant2.template GetAtIndex<Index + 1>());
            if (variant1.Which() == Index + 2)
                return visitor(variant1.template GetAtIndex<Index + 2>(), variant2.template GetAtIndex<Index + 2>());
            if (variant1.Which() == Index + 3)
                return visitor(variant1.template GetAtIndex<Index + 3>(), variant2.template GetAtIndex<Index + 3>());
            if (variant1.Which() == Index + 4)
                return visitor(variant1.template GetAtIndex<Index + 4>(), variant2.template GetAtIndex<Index + 4>());

            return ApplySameTypeVisitorHelper<Index + 5, Visitor, Variant>()(visitor, variant1, variant2);
        }

        template<std::size_t Index, class Visitor, class Variant>
        typename Visitor::ResultType ApplySameTypeVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index == Variant::size>::type>::operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2)
        {
            std::abort();
        }

        template<std::size_t Index, class Visitor, class Variant>
        typename Visitor::ResultType ApplySameTypeVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index + 1 == Variant::size>::type>::operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2)
        {
            assert(variant1.Which() == Index);
            return visitor(variant1.template GetAtIndex<Index>(), variant2.template GetAtIndex<Index>());
        }

        template<std::size_t Index, class Visitor, class Variant>
        typename Visitor::ResultType ApplySameTypeVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index + 2 == Variant::size>::type>::operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2)
        {
            assert(variant1.Which() >= Index && variant1.Which() < Index + 2);
            if (variant1.Which() == Index)
                return visitor(variant1.template GetAtIndex<Index>(), variant2.template GetAtIndex<Index>());
            return visitor(variant1.template GetAtIndex<Index + 1>(), variant2.template GetAtIndex<Index + 1>());
        }

        template<std::size_t Index, class Visitor, class Variant>
        typename Visitor::ResultType ApplySameTypeVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index + 3 == Variant::size>::type>::operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2)
        {
            assert(variant1.Which() >= Index && variant1.Which() < Index + 3);
            if (variant1.Which() == Index)
                return visitor(variant1.template GetAtIndex<Index>(), variant2.template GetAtIndex<Index>());
            if (variant1.Which() == Index + 1)
                return visitor(variant1.template GetAtIndex<Index + 1>(), variant2.template GetAtIndex<Index + 1>());
            return visitor(variant1.template GetAtIndex<Index + 2>(), variant2.template GetAtIndex<Index + 2>());
        }

        template<std::size_t Index, class Visitor, class Variant>
        typename Visitor::ResultType ApplySameTypeVisitorHelper<Index, Visitor, Variant, typename std::enable_if<Index + 4 == Variant::size>::type>::operator()(Visitor& visitor, const Variant& variant1, const Variant& variant2)
        {
            assert(variant1.Which() >= Index && variant1.Which() < Index + 4);
            if (variant1.Which() == Index)
                return visitor(variant1.template GetAtIndex<Index>(), variant2.template GetAtIndex<Index>());
            if (variant1.Which() == Index + 1)
                return visitor(variant1.template GetAtIndex<Index + 1>(), variant2.template GetAtIndex<Index + 1>());
            if (variant1.Which() == Index + 2)
                return visitor(variant1.template GetAtIndex<Index + 2>(), variant2.template GetAtIndex<Index + 2>());
            return visitor(variant1.template GetAtIndex<Index + 3>(), variant2.template GetAtIndex<Index + 3>());
        }

        template<class T>
        void DestroyVisitor::operator()(T& x)
        {
            x.~T();
        }

        template<class... T>
        ConstructVisitor<T...>::ConstructVisitor(Variant<T...>& aVariant)
            : variant(aVariant)
        {}

        template<class... T>
        template<class T2>
        void ConstructVisitor<T...>::operator()(const T2& v)
        {
            variant.template ConstructInEmptyVariant<T2>(v);
        }

        template<class... T>
        CopyVisitor<T...>::CopyVisitor(Variant<T...>& aVariant)
            : variant(aVariant)
        {}

        template<class... T>
        template<class T2>
        void CopyVisitor<T...>::operator()(const T2& v)
        {
            variant = v;
        }

        template<class T>
        bool EqualVisitor::operator()(const T& x, const T& y) const
        {
            return x == y;
        }

        template<class T, class U>
        bool EqualVisitor::operator()(const T& x, const U& y) const
        {
            std::abort();
        }

        template<class T>
        bool LessThanVisitor::operator()(const T& x, const T& y) const
        {
            return x < y;
        }

        template<class T, class U>
        bool LessThanVisitor::operator()(const T& x, const U& y) const
        {
            std::abort();
        }

        template<class ArgHead, class... ArgTail>
        struct ConstructAtIndexHelper<ArgHead, ArgTail...>
        {
            template<class Variant, class... Args>
            static void Construct(Variant& x, std::size_t index, Args&&... args)
            {
                if (index == 0)
                    x.template ConstructInEmptyVariant<ArgHead>(std::forward<Args>(args)...);
                else
                    ConstructAtIndexHelper<ArgTail...>::Construct(x, index - 1, std::forward<Args>(args)...);
            }
        };

        template<>
        struct ConstructAtIndexHelper<>
        {
            template<class Variant, class... Args>
            static void Construct(Variant& x, std::size_t index, Args&&... args)
            {
                std::abort();
            }
        };

        template<class Base, class... T>
        ConstructPolymorphicVisitor<Base, T...>::ConstructPolymorphicVisitor(PolymorphicVariant<Base, T...>& aVariant)
            : variant(aVariant)
        {}

        template<class Base, class... T>
        template<class T2>
        void ConstructPolymorphicVisitor<Base, T...>::operator()(const T2& v)
        {
            variant.template ConstructInEmptyVariant<T2>(v);
        }

        template<class Base, class... T>
        CopyPolymorphicVisitor<Base, T...>::CopyPolymorphicVisitor(PolymorphicVariant<Base, T...>& aVariant)
            : variant(aVariant)
        {}

        template<class Base, class... T>
        template<class T2>
        void CopyPolymorphicVisitor<Base, T...>::operator()(const T2& v)
        {
            variant = v;
        }
    }
}

#endif
