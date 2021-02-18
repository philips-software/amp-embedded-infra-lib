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

    ConstructBin& ConstructBin::Repeat(std::size_t amount, uint8_t v)
    {
        contents.insert(contents.end(), amount, v);
        return *this;
    }

    ConstructBin& ConstructBin::TextStream(const std::function<void(infra::TextOutputStream stream)>& callback)
    {
        callback(infra::StdVectorOutputStream(contents) << infra::text);
        return *this;
    }

    std::vector<uint8_t> ConstructBin::Vector() const
    {
        return contents;
    }

    std::string ConstructBin::String() const
    {
        return infra::ByteRangeAsStdString(infra::MakeRange(contents));
    }

    ConstByteRange ConstructBin::Range() const
    {
        return MakeRange(contents);
    }
}
