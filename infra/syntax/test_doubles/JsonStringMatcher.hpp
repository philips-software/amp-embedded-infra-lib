#ifndef INFRA_JSON_STRING_MATCHER_HPP
#define INFRA_JSON_STRING_MATCHER_HPP

#include "gmock/gmock.h"
#include "infra/syntax/Json.hpp"
#include "infra/util/test_helper/BoundedStringMatcher.hpp"

namespace testing
{
    template<>
    class Matcher<infra::JsonString>
        : public internal::MatcherBase<infra::JsonString>
    {
    public:
        Matcher() = default;

        explicit Matcher(const MatcherInterface<infra::JsonString>* impl)
            : internal::MatcherBase<infra::JsonString>(impl)
        {}

        explicit Matcher(const MatcherInterface<const infra::JsonString&>* impl)
            : internal::MatcherBase<infra::JsonString>(impl)
        {}

        template<class M, class = typename std::remove_reference<M>::type::is_gtest_matcher>
        Matcher(M&& m)
            : internal::MatcherBase<infra::JsonString>(std::forward<M>(m))
        {}

        Matcher(infra::JsonString s)
        {
            *this = Eq(s.ToStdString());
        }

        Matcher(const char* s)
        {
            *this = Eq(infra::BoundedConstString(s));
        }

        Matcher(infra::BoundedConstString s)
        {
            *this = Eq(s);
        }
    };
}

#endif
