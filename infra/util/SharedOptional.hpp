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
            SharedPtr<T> Emplace(Args&&... args);

        bool Allocatable() const;

        const T& operator*() const;
        T& operator*();
        const T* operator->() const;
        T* operator->();

        explicit operator bool() const;
        bool operator!() const;

    protected:
        virtual void Destruct(const void* object) override;
        virtual void Deallocate(void* control) override;

    private:
        infra::Optional<T> object;
        detail::SharedPtrControl control;
        bool allocatable = true;
    };

    template<class T>
    class NotifyingSharedOptional
        : public SharedOptional<T>
    {
    public:
        NotifyingSharedOptional() = default;
        explicit NotifyingSharedOptional(infra::Function<void()> onAllocatable);

        void OnAllocatable(infra::Function<void()> newOnAllocatable);

    protected:
        virtual void Deallocate(void* control) override;

    private:
        infra::Function<void()> onAllocatable;
    };

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
    SharedPtr<T> SharedOptional<T>::Emplace(Args&&... args)
    {
        assert(allocatable);
        allocatable = false;
        object.Emplace(std::forward<Args>(args)...);
        return SharedPtr<T>(&control, &*object);
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

    template<class T>
    NotifyingSharedOptional<T>::NotifyingSharedOptional(infra::Function<void()> onAllocatable)
        : onAllocatable(onAllocatable)
    {}

    template<class T>
    void NotifyingSharedOptional<T>::OnAllocatable(infra::Function<void()> newOnAllocatable)
    {
        onAllocatable = newOnAllocatable;
    }

    template<class T>
    void NotifyingSharedOptional<T>::Deallocate(void* control)
    {
        SharedOptional<T>::Deallocate(control);
        if (onAllocatable)
            onAllocatable();
    }
}

#endif
