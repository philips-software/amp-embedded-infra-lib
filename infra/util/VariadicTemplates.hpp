#ifndef INFRA_VARIADIC_TEMPLATES_HPP
#define INFRA_VARIADIC_TEMPLATES_HPP

#include <cstddef>
#include <type_traits>

namespace infra
{
    struct NullType
    {};

    template<class Front_, class... Tail>
    struct Front
    {
        typedef Front_ Type;
    };

    template<class U, class... List>
    struct ExistsInTypeList;

    template<class U, class... List>
    struct IndexInTypeList;

    template<std::size_t Index, class... List>
    struct TypeAtIndex;

    template<std::size_t x, std::size_t y>
    struct Max
        : std::integral_constant<std::size_t, (x > y ? x : y)>
    {};

    template<std::size_t x, std::size_t y>
    struct Min
        : std::integral_constant<std::size_t, (x < y ? x : y)>
    {};

    template<class... List>
    struct MaxSizeOfTypes;

    template<class... List>
    struct MaxAlignmentOfTypes;

    template<class... Types>
    struct MaxAlignmentType;

    template<class... T>
    struct List
    {};

    template<class ListA, class ListB>
    struct ListJoin;

    template<class List, class... Element>
    struct ListPushBack;

    template<class Element, class List>
    struct ListPushFront;

    template<class List, class Element>
    struct ExistsInList;

    template<class F>
    struct CanCall;

    ////    Implementation   ////

    template<class U, class Front, class... Tail>
    struct ExistsInTypeList<U, Front, Tail...>
        : std::integral_constant<bool, ExistsInTypeList<U, Tail...>::value>
    {};

    template<class U, class... Tail>
    struct ExistsInTypeList<U, U, Tail...>
        : std::integral_constant<bool, true>
    {};

    template<class U>
    struct ExistsInTypeList<U>
        : std::integral_constant<bool, false>
    {};

    template<class U, class Front, class... Tail>
    struct IndexInTypeList<U, Front, Tail...>
        : std::integral_constant<std::size_t, IndexInTypeList<U, Tail...>::value + 1>
    {};

    template<class U, class... Tail>
    struct IndexInTypeList<U, U, Tail...>
        : std::integral_constant<std::size_t, 0>
    {};

    template<std::size_t Index, class Front, class... Tail>
    struct TypeAtIndex<Index, Front, Tail...>
    {
        typedef typename TypeAtIndex<Index - 1, Tail...>::Type Type;
    };

    template<class Front, class... Tail>
    struct TypeAtIndex<0, Front, Tail...>
    {
        typedef Front Type;
    };

    template<class Front, class... Tail>
    struct MaxSizeOfTypes<Front, Tail...>
        : std::integral_constant<std::size_t, Max<sizeof(Front), MaxSizeOfTypes<Tail...>::value>::value>
    {};

    template<class Front>
    struct MaxSizeOfTypes<Front>
        : std::integral_constant<std::size_t, sizeof(Front)>
    {};

    template<class Front, class... Tail>
    struct MaxAlignmentOfTypes<Front, Tail...>
        : std::integral_constant<std::size_t, Max<std::alignment_of<Front>::value, MaxAlignmentOfTypes<Tail...>::value>::value>
    {};

    template<class Front>
    struct MaxAlignmentOfTypes<Front>
        : std::integral_constant<std::size_t, std::alignment_of<Front>::value>
    {};

    template<class Front, class... Tail>
    struct MaxAlignmentType<Front, Tail...>
        : std::integral_constant<std::size_t, Max<std::alignment_of<Front>::value, MaxAlignmentOfTypes<Tail...>::value>::value>
    {
        typedef typename MaxAlignmentType<Tail...>::Type MaxAlignmentTypeOfTail;
        typedef typename std::conditional<(sizeof(Front) > sizeof(MaxAlignmentTypeOfTail)), Front, MaxAlignmentTypeOfTail>::type Type;
    };

    template<class Front>
    struct MaxAlignmentType<Front>
    {
        typedef Front Type;
    };

    template<class... ListA, class... ListB>
    struct ListJoin<List<ListA...>, List<ListB...>>
    {
        typedef List<ListA..., ListB...> Type;
    };

    template<class... ListElements, class... Elements>
    struct ListPushBack<List<ListElements...>, Elements...>
    {
        typedef List<ListElements..., Elements...> Type;
    };

    template<class Element, class... ListElements>
    struct ListPushFront<Element, List<ListElements...>>
    {
        typedef List<Element, ListElements...> Type;
    };

    template<class Element>
    struct ExistsInList<List<>, Element>
        : std::integral_constant<bool, false>
    {};

    template<class Element, class... Tail>
    struct ExistsInList<List<Element, Tail...>, Element>
        : std::integral_constant<bool, true>
    {};

    template<class Element, class Head, class... Tail>
    struct ExistsInList<List<Head, Tail...>, Element>
        : ExistsInList<List<Tail...>, Element>
    {};

    struct CanCallHelper
    {
        template<class ResultType, class... Args>
        static decltype(std::declval<ResultType>()(std::declval<Args>()...), std::true_type()) f(int);

        template<class ResultType, class... Args>
        static std::false_type f(...);
    };

    template<class ResultType, class... Args>
    struct CanCall<ResultType(Args...)>
        : decltype(CanCallHelper::f<ResultType, Args...>(0))
    {};
}

#endif
