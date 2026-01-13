#ifndef INFRA_VISITOR_HPP
#define INFRA_VISITOR_HPP

namespace infra
{
    template<typename... Ts>
    struct MultiVisitor : Ts...
    {
        using Ts::operator()...;
    };

    // CTAD for visitor
    template<typename... Ts>
    MultiVisitor(Ts...) -> MultiVisitor<Ts...>;
}

#endif
