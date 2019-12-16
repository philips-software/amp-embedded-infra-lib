#ifndef INFRA_WITH_STORAGE_HPP
#define INFRA_WITH_STORAGE_HPP

#include "infra/util/Optional.hpp"  // For InPlace
#include <utility>

namespace infra
{
    namespace detail
    {
        template<class StorageType, class Base>
            class StorageHolder;
    }

#ifdef _MSC_VER                                                                                                         //TICS !POR#021
#pragma warning (push)                                                                                                  //TICS !POR#018
#pragma warning (disable: 4503)                                                                                         //TICS !POR#018
#endif

    template<class Base, class StorageType>
    class WithStorage
        : private detail::StorageHolder<StorageType, Base>    // Inherit from StorageHolder so that the storage gets constructed before the base
        , public Base
    {
    public:
        template<class StorageArg, class... Args>
            WithStorage(InPlace, StorageArg&& storageArg, Args&&... args);
        template<class... Args>                                                                                         //TICS !INT#001
            WithStorage(Args&&... args);
        template<class T, class... Args>
            WithStorage(std::initializer_list<T> initializerList, Args&&... args);

        WithStorage(const WithStorage& other);
        WithStorage(WithStorage&& other);

        WithStorage& operator=(const WithStorage& other);
        WithStorage& operator=(WithStorage&& other);

        template<class T>
            WithStorage& operator=(T&& value);

        const StorageType& Storage() const;
        StorageType& Storage();

        friend void swap(WithStorage& x, WithStorage& y) { using std::swap; swap(static_cast<Base&>(x), static_cast<Base&>(y)); }
    };

#ifdef _MSC_VER                                                                                                         //TICS !POR#021
#pragma warning (pop)                                                                                                   //TICS !POR#018
#endif

    namespace detail
    {
        template<class StorageType, class Base>
        class StorageHolder
        {
        public:
            StorageHolder() = default;

            template<class Arg0, class... Args>
                StorageHolder(Arg0&& arg0, Args&&... args);

            StorageType storage;                                                                                        //TICS !INT#002
        };
    }

    ////    Implementation    ////

    template<class Base, class StorageType>
    template<class StorageArg, class... Args>
    WithStorage<Base, StorageType>::WithStorage(InPlace, StorageArg&& storageArg, Args&&... args)
        : detail::StorageHolder<StorageType, Base>(std::forward<StorageArg>(storageArg))
        , Base(detail::StorageHolder<StorageType, Base>::storage, std::forward<Args>(args)...)
    {}

    template<class Base, class StorageType>
    template<class... Args>
    WithStorage<Base, StorageType>::WithStorage(Args&&... args)
        : Base(detail::StorageHolder<StorageType, Base>::storage, std::forward<Args>(args)...)
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
    WithStorage<Base, StorageType>::WithStorage(WithStorage&& other)
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
    WithStorage<Base, StorageType>& WithStorage<Base, StorageType>::operator=(WithStorage&& other)
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
        template<class Arg0, class... Args>
        StorageHolder<StorageType, Base>::StorageHolder(Arg0&& arg0, Args&&... args)
            : storage(std::forward<Arg0>(arg0), std::forward<Args>(args)...)
        {}
    }
}

#endif
