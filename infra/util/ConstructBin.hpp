#ifndef INFRA_CONSTRUCT_BIN_HPP
#define INFRA_CONSTRUCT_BIN_HPP

#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/ByteRange.hpp"
#include <functional>
#include <string>
#include <vector>

namespace infra
{
    class ConstructBin
    {
    public:
        ConstructBin& operator()(const std::vector<uint8_t>& v);
        ConstructBin& operator()(const std::string& v);
        ConstructBin& operator()(uint8_t v);
        ConstructBin& operator()(std::initializer_list<uint8_t> v);

        ConstructBin& Repeat(std::size_t amount, uint8_t v);
        ConstructBin& TextStream(const std::function<void(infra::TextOutputStream stream)>& callback);

        template<class T>
        ConstructBin& Value(T v);

        std::vector<uint8_t> Vector() const;
        std::string String() const;
        ConstByteRange Range() const;

    private:
        std::vector<uint8_t> contents;
    };

    ////    Implementation    ////

    template<class T>
    ConstructBin& ConstructBin::Value(T v)
    {
        auto range = infra::MakeByteRange(v);
        contents.insert(contents.end(), range.begin(), range.end());
        return *this;
    }
}

#endif
