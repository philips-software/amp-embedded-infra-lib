#ifndef INFRA_INTEGER_SEQUENCE_HPP
#define INFRA_INTEGER_SEQUENCE_HPP

// The types inside this file are mirror types of std::integer_sequence, std::index_sequence, etc.
// Once enough compilers support the standard types, this header file can be refactored away.

namespace infra
{
    template<class T, T... Ints>
    class IntegerSequence
    {};

    template<std::size_t... I>
    using IndexSequence = IntegerSequence<std::size_t, I...>;

    template<std::size_t... Ns>
    struct MakeIntegerSequenceHelper;

    template<std::size_t I, std::size_t... Ns>
    struct MakeIntegerSequenceHelper<I, Ns...>
    {
        using type = typename MakeIntegerSequenceHelper<I - 1, I - 1, Ns...>::type;
    };

    template<std::size_t... Ns>
    struct MakeIntegerSequenceHelper<0, Ns...>
    {
        using type = IntegerSequence<std::size_t, Ns...>;
    };

    template<std::size_t N>
    using MakeIntegerSequence = typename MakeIntegerSequenceHelper<N>::type;

    template<std::size_t N>
    using MakeIndexSequence = MakeIntegerSequence<N>;
}

#endif
