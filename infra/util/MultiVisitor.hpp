#ifndef INFRA_MULTI_VISITOR_HPP
#define INFRA_MULTI_VISITOR_HPP

// The MultiVisitor utility allows combining multiple visitor lambdas or functors into a single visitor object.
// This is particularly useful when working with std::variant and std::visit, enabling handling of multiple types
// with different behaviors in a clean and concise manner.
//
// Example usage:
//
// std::variant<int, std::string, float, double> value = 42;
//
// std::visit(MultiVisitor{
//     [](int i) { std::cout << "int: " << i << '\n'; },
//     [](const std::string& s) { std::cout << "string: " << s << '\n'; },
//     [](auto a) { std::cout << "auto: " << a << '\n'; }
// }, value);
//
// In this example, the MultiVisitor combines three different lambdas to handle int, std::string, and any other type (using auto).
// When std::visit is called with the MultiVisitor, it will invoke the appropriate lambda based on the type held by the variant.
//
// The auto lambda acts as a catch-all due to C++ overload resolution rules:
// - When std::visit invokes the visitor's operator(), the compiler considers all available overloads
// - More specific overloads (like `operator()(int)` or `operator()(const std::string&)`) are preferred
//   over generic template overloads during overload resolution
// - The auto lambda generates a templated `operator()(T)` which matches any type
// - During overload resolution, exact matches and conversions are ranked higher than template instantiations
// - Therefore, if a variant holds an int, the `operator()(int)` is selected over the templated version
// - If a variant holds a type without a specific overload (e.g., float), the templated operator() from
//   the auto lambda is instantiated and called as a fallback
// - This makes the auto lambda a catch-all for any types not explicitly handled by other overloads

namespace infra
{
    template<typename... Ts>
    struct MultiVisitor : Ts...
    {
        using Ts::operator()...;
    };

    // CTAD (Class template argument deduction) for MultiVisitor
    template<typename... Ts>
    MultiVisitor(Ts...) -> MultiVisitor<Ts...>;
}

#endif
