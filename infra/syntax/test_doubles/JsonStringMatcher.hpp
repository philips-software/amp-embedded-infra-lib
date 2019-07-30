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

        //Matcher(infra::JsonString s) { *this = Eq(std::string(s.data(), s.size())); }
        Matcher(const char* s) { *this = Eq(infra::BoundedConstString(s)); }
    };
}

#endif
