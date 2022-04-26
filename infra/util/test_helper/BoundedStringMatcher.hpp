#ifndef INFRA_BOUNDED_STRING_MATCHER_HPP
#define INFRA_BOUNDED_STRING_MATCHER_HPP

#include "gmock/gmock.h"
#include "infra/util/BoundedString.hpp"

namespace testing
{
    template<class T>
    class Matcher<infra::BoundedStringBase<T>>
        : public internal::MatcherBase<infra::BoundedStringBase<T>>
    {
    public:
        Matcher() = default;

        explicit Matcher(const MatcherInterface<const infra::BoundedStringBase<T>&>* impl)
            : internal::MatcherBase<infra::BoundedStringBase<T>>(impl)
        {}

        explicit Matcher(const MatcherInterface<infra::BoundedStringBase<T>>* impl)
            : internal::MatcherBase<infra::BoundedStringBase<T>>(impl)
        {}

        template<class M, class = typename std::remove_reference<M>::type::is_gtest_matcher>
        Matcher(M&& m)
            : internal::MatcherBase<infra::BoundedStringBase<T>>(std::forward<M>(m))
        {}

        Matcher(infra::BoundedStringBase<T> s)
        {
            *this = Eq(std::string(s.data(), s.size()));
        }

        Matcher(const char* s)
        {
            *this = Eq(std::string(s));
        }
    };

    template<std::size_t Size>
    class Matcher<const infra::BoundedStringBase<char>::WithStorage<Size>&>
        : public internal::MatcherBase<infra::BoundedStringBase<char>::WithStorage<Size>>
    {
    public:
        Matcher() = default;

        explicit Matcher(const MatcherInterface<const infra::BoundedStringBase<char>::WithStorage<Size>&>* impl)
            : internal::MatcherBase<infra::BoundedStringBase<char>::WithStorage<Size>>(impl)
        {}

        explicit Matcher(const MatcherInterface<infra::BoundedStringBase<char>::WithStorage<Size>>* impl)
            : internal::MatcherBase<infra::BoundedStringBase<char>::WithStorage<Size>>(impl)
        {}

        template<class M, class = typename std::remove_reference<M>::type::is_gtest_matcher>
        Matcher(M&& m)
            : internal::MatcherBase<infra::BoundedStringBase<char>::WithStorage<Size>>(std::forward<M>(m))
        {}

        Matcher(infra::BoundedStringBase<char>::WithStorage<Size> s)
        {
            *this = Eq(std::string(s.data(), s.size()));
        }

        Matcher(const char* s)
        {
            *this = Eq(std::string(s));
        }
    };

    template<std::size_t Size>
    class Matcher<const infra::BoundedStringBase<const char>::WithStorage<Size>&>
        : public internal::MatcherBase<infra::BoundedStringBase<const char>::WithStorage<Size>>
    {
    public:
        Matcher() = default;

        explicit Matcher(const MatcherInterface<const infra::BoundedStringBase<const char>::WithStorage<Size>&>* impl)
            : internal::MatcherBase<infra::BoundedStringBase<const char>::WithStorage<Size>>(impl)
        {
        }

        explicit Matcher(const MatcherInterface<infra::BoundedStringBase<const char>::WithStorage<Size>>* impl)
            : internal::MatcherBase<infra::BoundedStringBase<const char>::WithStorage<Size>>(impl)
        {
        }

        Matcher(infra::BoundedStringBase<const char>::WithStorage<Size> s)
        {
            *this = Eq(std::string(s.data(), s.size()));
        }

        Matcher(const char* s)
        {
            *this = Eq(std::string(s));
        }
    };
}

#endif
