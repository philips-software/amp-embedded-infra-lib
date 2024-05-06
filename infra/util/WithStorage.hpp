#ifndef INFRA_WITH_STORAGE_HPP
#define INFRA_WITH_STORAGE_HPP

#include "infra/util/Optional.hpp" // For std::in_place_t
#include <utility>

namespace infra
{
    namespace detail
    {
        template<class StorageType, class Base>
        class StorageHolder;
    }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4503)
#endif

    template<class Base, class StorageType>
    class WithStorage
        : private detail::StorageHolder<StorageType, Base> // Inherit from StorageHolder so that the storage gets constructed before the base
        , public Base
    {
    public:
        WithStorage();
        template<class StorageArg, class... Args>
        WithStorage(std::in_place_t, StorageArg&& storageArg, Args&&... args);
        template<class T>
        WithStorage(std::in_place_t, std::initializer_list<T> initializerList);
        template<class Arg>
        WithStorage(Arg&& arg, std::enable_if_t<!std::is_same_v<WithStorage, std::remove_cv_t<std::remove_reference_t<Arg>>>, std::nullptr_t> = nullptr);
        template<class Arg0, class Arg1, class... Args>
        WithStorage(Arg0&& arg0, Arg1&& arg1, Args&&... args);
        template<class T, class... Args>
        WithStorage(std::initializer_list<T> initializerList, Args&&... args);

        WithStorage(const WithStorage& other);
        WithStorage(WithStorage&& other) noexcept;

        ~WithStorage() = default;

        WithStorage& operator=(const WithStorage& other);
        WithStorage& operator=(WithStorage&& other) noexcept;

        template<class T>
        WithStorage& operator=(T&& value);

        const StorageType& Storage() const;
        StorageType& Storage();

        friend void swap(WithStorage& x, WithStorage& y) noexcept
        {
            using std::swap;

            swap(static_cast<Base&>(x), static_cast<Base&>(y));
        }
    };

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    namespace detail
    {
        template<class StorageType, class Base>
        class StorageHolder
        {
        public:
            StorageHolder() = default;
            template<class Arg>
            StorageHolder(Arg&& arg, std::enable_if_t<!std::is_same_v<StorageHolder, std::remove_cv_t<std::remove_reference_t<Arg>>>, std::nullptr_t> = nullptr);
            template<class Arg0, class Arg1, class... Args>
            StorageHolder(Arg0&& arg0, Arg1&& arg1, Args&&... args);

            StorageType storage;
        };
    }

    ////    Implementation    ////

    template<class Base, class StorageType>
    WithStorage<Base, StorageType>::WithStorage()
        : Base(detail::StorageHolder<StorageType, Base>::storage)
    {}

    template<class Base, class StorageType>
    template<class StorageArg, class... Args>
    WithStorage<Base, StorageType>::WithStorage(std::in_place_t, StorageArg&& storageArg, Args&&... args)
        : detail::StorageHolder<StorageType, Base>(std::forward<StorageArg>(storageArg))
        , Base(detail::StorageHolder<StorageType, Base>::storage, std::forward<Args>(args)...)
    {}

    template<class Base, class StorageType>
    template<class T>
    WithStorage<Base, StorageType>::WithStorage(std::in_place_t, std::initializer_list<T> initializerList)
        : detail::StorageHolder<StorageType, Base>(initializerList)
        , Base(detail::StorageHolder<StorageType, Base>::storage)
    {}

    template<class Base, class StorageType>
    template<class Arg>
    WithStorage<Base, StorageType>::WithStorage(Arg&& arg, std::enable_if_t<!std::is_same_v<WithStorage, std::remove_cv_t<std::remove_reference_t<Arg>>>, std::nullptr_t>)
        : Base(detail::StorageHolder<StorageType, Base>::storage, std::forward<Arg>(arg))
    {}

    template<class Base, class StorageType>
    template<class Arg0, class Arg1, class... Args>
    WithStorage<Base, StorageType>::WithStorage(Arg0&& arg0, Arg1&& arg1, Args&&... args)
        : Base(detail::StorageHolder<StorageType, Base>::storage, std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Args>(args)...)
    {}

    template<class Base, class StorageType>
    template<class T, class... Args>
    WithStorage<Base, StorageType>::WithStorage(std::initializer_list<T> initializerList, Args&&... args)
        : Base(detail::StorageHolder<StorageType, Base>::storage, initializerList, std::forward<Args>(args)...)
    {}

    template<class Base, class StorageType>
    WithStorage<Base, StorageType>::WithStorage(const WithStorage& other)
        : detail::StorageHolder<StorageType, Base>()
        , Base(detail::StorageHolder<StorageType, Base>::storage, other)
    {}

    template<class Base, class StorageType>
    WithStorage<Base, StorageType>::WithStorage(WithStorage&& other) noexcept
        : detail::StorageHolder<StorageType, Base>()
        , Base(detail::StorageHolder<StorageType, Base>::storage, std::move(other))
    {}

    template<class Base, class StorageType>
    WithStorage<Base, StorageType>& WithStorage<Base, StorageType>::operator=(const WithStorage& other)
    {
        this->AssignFromStorage(other);
        return *this;
    }

    template<class Base, class StorageType>
    WithStorage<Base, StorageType>& WithStorage<Base, StorageType>::operator=(WithStorage&& other) noexcept
    {
        this->AssignFromStorage(std::move(other));
        return *this;
    }

    template<class Base, class StorageType>
    template<class T>
    WithStorage<Base, StorageType>& WithStorage<Base, StorageType>::operator=(T&& value)
    {
        this->AssignFromStorage(std::forward<T>(value));
        return *this;
    }

    template<class Base, class StorageType>
    const StorageType& WithStorage<Base, StorageType>::Storage() const
    {
        return detail::StorageHolder<StorageType, Base>::storage;
    }

    template<class Base, class StorageType>
    StorageType& WithStorage<Base, StorageType>::Storage()
    {
        return detail::StorageHolder<StorageType, Base>::storage;
    }

    namespace detail
    {
        template<class StorageType, class Base>
        template<class Arg>
        StorageHolder<StorageType, Base>::StorageHolder(Arg&& arg, std::enable_if_t<!std::is_same_v<StorageHolder, std::remove_cv_t<std::remove_reference_t<Arg>>>, std::nullptr_t>)
            : storage(std::forward<Arg>(arg))
        {}

        template<class StorageType, class Base>
        template<class Arg0, class Arg1, class... Args>
        StorageHolder<StorageType, Base>::StorageHolder(Arg0&& arg0, Arg1&& arg1, Args&&... args)
            : storage(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Args>(args)...)
        {}
    }
}

#endif
