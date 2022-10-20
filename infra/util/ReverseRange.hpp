#ifndef INFRA_REVERSE_RANGE_HPP
#define INFRA_REVERSE_RANGE_HPP

// This header provides one function: MakeReverseRange. This function is usable in for-loops to iterate
// over a container in reverse order.
//
// Example:
//     std::vector<int> numbers = ...;
//     for (int number: MakeReverseRange(numbers))
//         Print(number);

#include <utility>

namespace infra
{
    namespace detail
    {
        template<class T>
        struct DoublePair : std::pair<T, T>
        {
            DoublePair() = default;

            DoublePair(const T& x, const T& y)
                : std::pair<T, T>(x, y)
            {}
        };

        template<class T>
        T begin(const DoublePair<T>& p)
        {
            return p.first;
        }

        template<class T>
        T end(const DoublePair<T>& p)
        {
            return p.second;
        }
    }

    template<class Range>
    detail::DoublePair<typename Range::reverse_iterator> MakeReverseRange(Range& r)
    {
        return detail::DoublePair<typename Range::reverse_iterator>(r.rbegin(), r.rend());
    }
}

#endif
