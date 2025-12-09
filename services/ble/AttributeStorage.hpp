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
            really_assert(handle != 0);
            really_assert(!Find(handle).has_value());
            auto allocated = Allocate(size);
            std::fill(allocated.begin(), allocated.end(), 0);
            entries.emplace_back(handle, allocated);
        }

        bool Store(services::AttAttribute::Handle handle, infra::ConstByteRange data)
        {
            auto allocated = Find(handle);

            if (!allocated.has_value() || allocated->size() < data.size())
                return false;

            std::copy(data.begin(), data.end(), allocated->begin());

            return true;
        }

        std::optional<infra::ConstByteRange> Get(services::AttAttribute::Handle handle) const
        {
            return Find(handle);
        }

    private:
        infra::ByteRange Allocate(std::size_t size)
        {
            really_assert(storage.size() >= size);
            auto result = infra::MakeRange(storage.begin(), storage.begin() + size);
            storage.pop_front(size);
            return result;
        }

        std::optional<infra::ByteRange> Find(services::AttAttribute::Handle handle) const
        {
            for (auto& entry : entries)
                if (entry.handle == handle)
                    return entry.data;
            return std::nullopt;
        }

    private:
        infra::BoundedVector<AttributeStorageEntry>& entries;
        infra::ByteRange storage;
    };
}
