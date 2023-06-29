#include "infra/syntax/Json.hpp"
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, std::size_t size)
{
    FuzzedDataProvider provider(data, size);
    auto objectString = provider.ConsumeRandomLengthString();
    infra::JsonObject object(objectString);

    for (auto o : object)
    {}

    return 0;
}
