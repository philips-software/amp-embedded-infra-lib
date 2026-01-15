#ifndef INFRA_TRANSFORM_OPTIONAL_HPP
#define INFRA_TRANSFORM_OPTIONAL_HPP

#include <optional>

namespace infra
{
    template<class T, class F>
    auto TransformOptional(const std::optional<T>& value, F&& transformation) -> std::optional<decltype(transformation(*value))>
    {
        if (value != std::nullopt)
            return std::make_optional(transformation(*value));
        else
            return std::nullopt;
    }
}

#endif
