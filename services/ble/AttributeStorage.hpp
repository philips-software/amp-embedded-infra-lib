#pragma once

#include "infra/util/BoundedVector.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "infra/util/WithStorage.hpp"
#include "services/ble/Gatt.hpp"
#include <cstddef>
#include <optional>

namespace services
{
    class AttributeStorage
    {
        struct AttributeStorageEntry
        {
            explicit AttributeStorageEntry(services::AttAttribute::Handle handle, infra::ByteRange data)
                : handle(handle)
                , data(data)
            {}

            services::AttAttribute::Handle handle;
            infra::ByteRange data;
            std::size_t populated = 0;
        };

    public:
        template<std::size_t NumberOfEntries, std::size_t storageSize>
        using WithStorage = infra::WithStorage<
            infra::WithStorage<AttributeStorage, infra::BoundedVector<AttributeStorageEntry>::WithMaxSize<NumberOfEntries>>,
            std::array<uint8_t, storageSize>>;

        explicit AttributeStorage(infra::BoundedVector<AttributeStorageEntry>& entries, infra::ByteRange storage)
            : entries(entries)
            , storage(storage)
        {}

        void Create(services::AttAttribute::Handle handle, std::size_t size)
        {
            // TODO(HW): What to do with zero size?
            really_assert(Find(handle) == nullptr);
            really_assert(handle != 0);
            auto allocated = Allocate(size);
            std::fill(allocated.begin(), allocated.end(), 0);
            entries.emplace_back(handle, allocated);
        }

        bool Write(services::AttAttribute::Handle handle, infra::ConstByteRange data)
        {
            auto entry = Find(handle);

            if (entry == nullptr || entry->data.size() < data.size())
                return false;

            std::copy(data.begin(), data.end(), entry->data.begin());
            entry->populated = data.size();

            return true;
        }

        std::optional<infra::ConstByteRange> Read(services::AttAttribute::Handle handle) const
        {
            auto entry = Find(handle);
            if (entry == nullptr)
                return std::nullopt;
            return infra::ConstByteRange(entry->data.begin(), entry->data.begin() + entry->populated);
        }

    private:
        std::size_t roundToAlignedSize(std::size_t size)
        {
            constexpr std::size_t alignment = alignof(std::max_align_t);
            return ((size + alignment - 1) / alignment) * alignment;
        }

        infra::ByteRange Allocate(std::size_t size)
        {
            really_assert(storage.size() >= size);
            auto result = infra::MakeRange(storage.begin(), storage.begin() + size);
            storage.pop_front(roundToAlignedSize(size));
            return result;
        }

        AttributeStorageEntry* Find(services::AttAttribute::Handle handle) const
        {
            for (auto& entry : entries)
                if (entry.handle == handle)
                    return &entry;
            return nullptr;
        }

    private:
        infra::BoundedVector<AttributeStorageEntry>& entries;
        infra::ByteRange storage;
    };
}
