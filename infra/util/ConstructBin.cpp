#include "infra/util/ConstructBin.hpp"

namespace infra
{
    ConstructBin& ConstructBin::operator()(const std::vector<uint8_t>& v)
    {
        contents.insert(contents.end(), v.begin(), v.end());
        return *this;
    }

    ConstructBin& ConstructBin::operator()(const std::string& v)
    {
        contents.insert(contents.end(), reinterpret_cast<const uint8_t*>(v.data()), reinterpret_cast<const uint8_t*>(v.data()) + v.size());
        return *this;
    }

    ConstructBin& ConstructBin::operator()(uint8_t v)
    {
        contents.push_back(v);
        return *this;
    }

    ConstructBin& ConstructBin::operator()(std::initializer_list<uint8_t> v)
    {
        contents.insert(contents.end(), v.begin(), v.end());
        return *this;
    }

    std::vector<uint8_t> ConstructBin::Vector() const
    {
        return contents;
    }

    ConstByteRange ConstructBin::Range() const
    {
        return MakeRange(contents);
    }
}
