#ifndef INFRA_SHARED_OPTIONAL_HPP
#define INFRA_SHARED_OPTIONAL_HPP

#include "infra/util/Function.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/SharedObjectAllocator.hpp"

namespace infra
{
    template<class T>
    class SharedOptional
        : protected SharedObjectDeleter
    {
    public:
        SharedOptional();
        SharedOptional(const SharedOptional& other) = delete;
        SharedOptional& operator=(const SharedOptional& other) = delete;
        ~SharedOptional();

        template<class... Args>
        SharedPtr<T> emplace(Args&&... args);

        SharedPtr<T> MakePtr();

        bool Allocatable() const;

        const T& operator*() const;
        T& operator*();
        const T* operator->() const;
        T* operator->();

        explicit operator bool() const;
        bool operator!() const;

    protected:
        void Destruct(const void* object) override;
        void Deallocate(void* control) override;

    private:
        infra::Optional<T> object;
        detail::SharedPtrControl control;
        bool allocatable = true;
    };

    template<class T, std::size_t AllocatableSize>
    class NotifyingSharedOptionalWithSize
        : public SharedOptional<T>
    {
    public:
        template<std::size_t NewAllocatableSize>
        using WithSize = NotifyingSharedOptionalWithSize<T, NewAllocatableSize>;

        NotifyingSharedOptionalWithSize() = default;
        explicit NotifyingSharedOptionalWithSize(const infra::Function<void(), AllocatableSize>& onAllocatable);

        void OnAllocatable(const infra::Function<void(), AllocatableSize>& newOnAllocatable);

    protected:
        void Deallocate(void* control) override;

    private:
        infra::Function<void(), AllocatableSize> onAllocatable;
    };

    template<class T>
    using NotifyingSharedOptional = NotifyingSharedOptionalWithSize<T, INFRA_DEFAULT_FUNCTION_EXTRA_SIZE>;

    ////    Implementation    ////

    template<class T>
    SharedOptional<T>::SharedOptional()
        : control(&object, this)
    {}

    template<class T>
    SharedOptional<T>::~SharedOptional()
    {
        really_assert(allocatable);
    }

    template<class T>
    template<class... Args>
    SharedPtr<T> SharedOptional<T>::emplace(Args&&... args)
    {
        assert(allocatable);
        allocatable = false;
        object.emplace(std::forward<Args>(args)...);
        return SharedPtr<T>(&control, &*object);
    }

    template<class T>
    SharedPtr<T> SharedOptional<T>::MakePtr()
    {
        return SharedPtr<T>(&control, &*object, noSharedFromThis);
    }

    template<class T>
    bool SharedOptional<T>::Allocatable() const
    {
        return allocatable;
    }

    template<class T>
    const T& SharedOptional<T>::operator*() const
    {
        return *object;
    }

    template<class T>
    T& SharedOptional<T>::operator*()
    {
        return *object;
    }

    template<class T>
    const T* SharedOptional<T>::operator->() const
    {
        return &*object;
    }

    template<class T>
    T* SharedOptional<T>::operator->()
    {
        return &*object;
    }

    template<class T>
    SharedOptional<T>::operator bool() const
    {
        return !!object;
    }

    template<class T>
    bool SharedOptional<T>::operator!() const
    {
        return !object;
    }

    template<class T>
    void SharedOptional<T>::Destruct(const void* object)
    {
        this->object = infra::none;
    }

    template<class T>
    void SharedOptional<T>::Deallocate(void* control)
    {
        allocatable = true;
    }

    template<class T, std::size_t AllocatableSize>
    NotifyingSharedOptionalWithSize<T, AllocatableSize>::NotifyingSharedOptionalWithSize(const infra::Function<void(), AllocatableSize>& onAllocatable)
        : onAllocatable(onAllocatable)
    {}

    template<class T, std::size_t AllocatableSize>
    void NotifyingSharedOptionalWithSize<T, AllocatableSize>::OnAllocatable(const infra::Function<void(), AllocatableSize>& newOnAllocatable)
    {
        onAllocatable = newOnAllocatable;
    }

    template<class T, std::size_t AllocatableSize>
    void NotifyingSharedOptionalWithSize<T, AllocatableSize>::Deallocate(void* control)
    {
        SharedOptional<T>::Deallocate(control);
        if (onAllocatable)
            onAllocatable();
    }
}

#endif
