#ifndef INFRA_SHARED_PTR_HPP
#define INFRA_SHARED_PTR_HPP

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <memory>
#include "infra/util/Function.hpp"
#include "infra/util/StaticStorage.hpp"

namespace infra
{
    template<class T>
    class SharedPtr;

    template<class T>
    class WeakPtr;

    class SharedObjectDeleter
    {
    protected:
        SharedObjectDeleter() = default;
        SharedObjectDeleter(const SharedObjectDeleter& other) = delete;
        SharedObjectDeleter& operator=(const SharedObjectDeleter& other) = delete;
        ~SharedObjectDeleter() = default;

    public:
        virtual void Destruct(const void* object) = 0;
        virtual void Deallocate(void* control) = 0;
    };

    namespace detail
    {
        class SharedPtrControl
        {
        public:
            SharedPtrControl(const void* object, SharedObjectDeleter* deleter);
            SharedPtrControl(const SharedPtrControl& other) = delete;
            SharedPtrControl& operator=(const SharedPtrControl& other) = delete;

            void IncreaseSharedCount();
            void DecreaseSharedCount();
            void IncreaseWeakCount();
            void DecreaseWeakCount();

            bool Expired() const;       // Returns whether no SharedPtrs still point to this object
            bool UnReferenced() const;  // Returns whether no SharedPtrs nor WeakPtrs still point to this object

        private:
            uint16_t sharedPtrCount = 0;
            uint16_t weakPtrCount = 0;
            const void* object = nullptr;
            SharedObjectDeleter* deleter = nullptr;
        };

        template<class T>
            void MakeSharedFromThis(SharedPtr<T>* object, typename T::EnableSharedFromThisType* = nullptr);
        inline void MakeSharedFromThis(void* object);
    }

    template<class T>
    class EnableSharedFromThis
    {
    protected:
        ~EnableSharedFromThis() = default;

    public:
        SharedPtr<T> SharedFromThis();
        SharedPtr<const T> SharedFromThis() const;
        WeakPtr<T> WeakFromThis();
        WeakPtr<const T> WeakFromThis() const;

        void MakeShared(SharedPtr<T> sharedPtr);

    public:
        using EnableSharedFromThisType = T;

    private:
        template<class U>
            friend void detail::MakeSharedFromThis(SharedPtr<U>* object, typename U::EnableSharedFromThisType*);

        WeakPtr<T> weakPtr;
    };

    template<class T>
    class SharedPtr
    {
    public:
        SharedPtr() = default;
        SharedPtr(std::nullptr_t);                                                                                      //TICS !INT#001
        SharedPtr(detail::SharedPtrControl* control, T* object);
        SharedPtr(const SharedPtr& other);
        SharedPtr(SharedPtr&& other);
        template<class U>
            SharedPtr(const WeakPtr<U>& other);                                                                         //TICS !INT#001
        template<class U>                                                                                               //TICS !INT#001
            SharedPtr(const SharedPtr<U>& other);
        template<class U>                                                                                               //TICS !INT#001
            SharedPtr(SharedPtr<U>&& other);
        SharedPtr& operator=(const SharedPtr& other);
        SharedPtr& operator=(SharedPtr&& other);
        SharedPtr& operator=(const WeakPtr<T>& other);
        SharedPtr& operator=(std::nullptr_t);
        ~SharedPtr();

        explicit operator bool() const;
        T* operator->() const;
        typename std::add_lvalue_reference<T>::type operator*() const;

        template<class U>
            bool operator==(const SharedPtr<U>& other) const;
        template<class U>
            bool operator!=(const SharedPtr<U>& other) const;
        bool operator==(std::nullptr_t) const;
        bool operator!=(std::nullptr_t) const;
        friend bool operator==(std::nullptr_t, const SharedPtr& ptr) { return ptr == nullptr; }
        friend bool operator!=(std::nullptr_t, const SharedPtr& ptr) { return ptr != nullptr; }

    private:
        void Reset(detail::SharedPtrControl* newControl, T* object);

    private:
        template<class U>
            friend class SharedPtr;
        template<class U>
            friend class WeakPtr;

        template<class U, class V>
            friend SharedPtr<U> StaticPointerCast(const SharedPtr<V>& sharedPtr);
        template<class U, class V>
            friend SharedPtr<U> StaticPointerCast(SharedPtr<V>&& sharedPtr);
        template<class U>
            friend SharedPtr<typename std::remove_const<U>::type> ConstCast(const SharedPtr<U>& sharedPtr);
        template<class U>
            friend SharedPtr<typename std::remove_const<U>::type> ConstCast(SharedPtr<U>&& sharedPtr);
        template<class U, class V>
            friend SharedPtr<U> MakeContainedSharedObject(U& object, const SharedPtr<V>& container);
        template<class U, class V>
            friend SharedPtr<U> MakeContainedSharedObject(U& object, SharedPtr<V>&& container);

    private:
        detail::SharedPtrControl* control = nullptr;
        T* object = nullptr;
    };

    template<class T>
    class WeakPtr
    {
    public:
        WeakPtr() = default;
        WeakPtr(const WeakPtr<T>& other);
        WeakPtr(WeakPtr<T>&& other);
        template<class U>                                                                                               //TICS !INT#001
            WeakPtr(const WeakPtr<U>& other);
        template<class U>
            WeakPtr(WeakPtr<U>&& other);                                                                                //TICS !INT#001
        WeakPtr(const SharedPtr<T>& sharedPtr);                                                                         //TICS !INT#001
        WeakPtr& operator=(const WeakPtr<T>& other);
        WeakPtr& operator=(WeakPtr<T>&& other);
        WeakPtr& operator=(const SharedPtr<T>& sharedPtr);
        ~WeakPtr();

        SharedPtr<T> lock() const;

        template<class U>
            bool operator==(const WeakPtr<U>& other) const;
        template<class U>
            bool operator!=(const WeakPtr<U>& other) const;
        bool operator==(std::nullptr_t) const;
        bool operator!=(std::nullptr_t) const;
        friend bool operator==(std::nullptr_t, const WeakPtr& ptr) { return ptr == nullptr; }
        friend bool operator!=(std::nullptr_t, const WeakPtr& ptr) { return ptr != nullptr; }
        template<class U>
            bool operator==(const SharedPtr<U>& other) const;
        template<class U>
            bool operator!=(const SharedPtr<U>& other) const;
        template<class U>
            friend bool operator==(const SharedPtr<U>& left, const WeakPtr& right) { return right == left; }
        template<class U>
            friend bool operator!=(const SharedPtr<U>& left, const WeakPtr& right) { return right != left; }

    private:
        void Reset(detail::SharedPtrControl* newControl, T* newObject);

        template<class U>
            friend class SharedPtr;
        template<class U>
            friend class WeakPtr;

    private:
        detail::SharedPtrControl* control = nullptr;
        T* object = nullptr;
    };

    class AccessedBySharedPtr
        : private SharedObjectDeleter
    {
    public:
        AccessedBySharedPtr(const infra::Function<void()>& onUnReferenced);
        ~AccessedBySharedPtr();

        template<class T>
            SharedPtr<T> MakeShared(T& value);

        void SetAction(const infra::Function<void()>& newOnUnReferenced);

    private:
        virtual void Destruct(const void* object) override;
        virtual void Deallocate(void* control) override;

    private:
        detail::SharedPtrControl control;
        infra::Function<void()> onUnReferenced;
    };

    template<class T>
        SharedPtr<T> UnOwnedSharedPtr(T& object);

    template<class T, class U>
        SharedPtr<T> MakeContainedSharedObject(T& object, const SharedPtr<U>& container);

    template<class T, class... Args>
        SharedPtr<T> MakeSharedOnHeap(Args&&... args);

    ////    Implementation    ////

    template<class T>
    SharedPtr<T>::SharedPtr(std::nullptr_t)
    {}

    template<class T>
    SharedPtr<T>::SharedPtr(detail::SharedPtrControl* control, T* object)
    {
        Reset(control, object);
        detail::MakeSharedFromThis(this);
    }

    template<class T>
    SharedPtr<T>::SharedPtr(const SharedPtr& other)
    {
        Reset(other.control, other.object);
    }

    template<class T>
    SharedPtr<T>::SharedPtr(SharedPtr&& other)
    {
        Reset(other.control, other.object);
        other.Reset(nullptr, nullptr);
    }

    template<class T>
    template<class U>
    SharedPtr<T>::SharedPtr(const WeakPtr<U>& other)
    {
        if (other.control && !other.control->Expired())
            Reset(other.control, other.object);
    }

    template<class T>
    template<class U>
    SharedPtr<T>::SharedPtr(const SharedPtr<U>& other)
    {
        Reset(other.control, other.object);
    }

    template<class T>
    template<class U>
    SharedPtr<T>::SharedPtr(SharedPtr<U>&& other)
    {
        Reset(other.control, other.object);
        other.Reset(nullptr, nullptr);
    }

    template<class T>
    SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& other)
    {
        if (this != &other)
            Reset(other.control, other.object);

        return *this;
    }

    template<class T>
    SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& other)
    {
        Reset(other.control, other.object);
        other.Reset(nullptr, nullptr);

        return *this;
    }

    template<class T>
    SharedPtr<T>& SharedPtr<T>::operator=(const WeakPtr<T>& other)
    {
        Reset(other.control, other.object);

        return *this;
    }

    template<class T>
    SharedPtr<T>& SharedPtr<T>::operator=(std::nullptr_t)
    {
        Reset(nullptr, nullptr);
        return *this;
    }

    template<class T>
    SharedPtr<T>::~SharedPtr()
    {
        Reset(nullptr, nullptr);
    }

    template<class T>
    SharedPtr<T>::operator bool() const
    {
        return control != nullptr;
    }

    template<class T>
    T* SharedPtr<T>::operator->() const
    {
        assert(object != nullptr);
        return object;
    }

    template<class T>
    typename std::add_lvalue_reference<T>::type SharedPtr<T>::operator*() const
    {
        assert(object != nullptr);
        return *object;
    }

    template<class T>
    template<class U>
    bool SharedPtr<T>::operator==(const SharedPtr<U>& other) const
    {
        return object == other.object;
    }

    template<class T>
    template<class U>
    bool SharedPtr<T>::operator!=(const SharedPtr<U>& other) const
    {
        return !(*this == other);
    }

    template<class T>
    bool SharedPtr<T>::operator==(std::nullptr_t) const
    {
        return object == nullptr;
    }

    template<class T>
    bool SharedPtr<T>::operator!=(std::nullptr_t) const
    {
        return !(*this == nullptr);
    }

    template<class T>
    void SharedPtr<T>::Reset(detail::SharedPtrControl* newControl, T* newObject)
    {
        detail::SharedPtrControl* oldControl = control;

        control = newControl;
        object = newObject;

        if (control)
            control->IncreaseSharedCount();

        if (oldControl)
            oldControl->DecreaseSharedCount();
    }

    template<class U, class T>
    SharedPtr<U> StaticPointerCast(const SharedPtr<T>& sharedPtr)
    {
        return SharedPtr<U>(sharedPtr.control, static_cast<U*>(sharedPtr.object));
    }

    template<class U, class T>
    SharedPtr<U> StaticPointerCast(SharedPtr<T>&& sharedPtr)
    {
        SharedPtr<U> result(sharedPtr.control, static_cast<U*>(sharedPtr.object));
        sharedPtr.Reset(nullptr, nullptr);
        return result;
    }

    template<class T>
    SharedPtr<typename std::remove_const<T>::type> ConstCast(const SharedPtr<T>& sharedPtr)
    {
        return SharedPtr<typename std::remove_const<T>::type>(sharedPtr.control, const_cast<typename std::remove_const<T>::type*>(sharedPtr.object));
    }

    template<class T>
    SharedPtr<typename std::remove_const<T>::type> ConstCast(SharedPtr<T>&& sharedPtr)
    {
        SharedPtr<typename std::remove_const<T>::type> result(sharedPtr.control, const_cast<typename std::remove_const<T>::type*>(sharedPtr.object));
        sharedPtr.Reset(nullptr, nullptr);
        return result;
    }

    template<class T>
    WeakPtr<T>::WeakPtr(const WeakPtr& other)
    {
        Reset(other.control, other.object);
    }

    template<class T>
    WeakPtr<T>::WeakPtr(WeakPtr&& other)
    {
        Reset(other.control, other.object);
        other.Reset(nullptr, nullptr);
    }

    template<class T>
    template<class U>
    WeakPtr<T>::WeakPtr(const WeakPtr<U>& other)
    {
        Reset(other.control, other.object);
    }

    template<class T>
    template<class U>
    WeakPtr<T>::WeakPtr(WeakPtr<U>&& other)
    {
        Reset(other.control, other.object);
        other.Reset(nullptr, nullptr);
    }

    template<class T>
    WeakPtr<T>::WeakPtr(const SharedPtr<T>& sharedPtr)
    {
        Reset(sharedPtr.control, sharedPtr.object);
    }

    template<class T>
    WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr& other)
    {
        Reset(other.control, other.object);

        return *this;
    }

    template<class T>
    WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr&& other)
    {
        Reset(other.control, other.object);
        other.Reset(nullptr, nullptr);

        return *this;
    }

    template<class T>
    WeakPtr<T>& WeakPtr<T>::operator=(const SharedPtr<T>& sharedPtr)
    {
        Reset(sharedPtr.control, sharedPtr.object);

        return *this;
    }

    template<class T>
    WeakPtr<T>::~WeakPtr()
    {
        Reset(nullptr, nullptr);
    }

    template<class T>
    SharedPtr<T> WeakPtr<T>::lock() const
    {
        return SharedPtr<T>(*this);
    }

    template<class T>
    template<class U>
    bool WeakPtr<T>::operator==(const WeakPtr<U>& other) const
    {
        return object == other.object;
    }

    template<class T>
    template<class U>
    bool WeakPtr<T>::operator!=(const WeakPtr<U>& other) const
    {
        return !(*this == other);
    }

    template<class T>
    bool WeakPtr<T>::operator==(std::nullptr_t) const
    {
        return object == nullptr;
    }

    template<class T>
    bool WeakPtr<T>::operator!=(std::nullptr_t) const
    {
        return !(*this == nullptr);
    }

    template<class T>
    template<class U>
    bool WeakPtr<T>::operator==(const SharedPtr<U>& other) const
    {
        return object == other.object;
    }

    template<class T>
    template<class U>
    bool WeakPtr<T>::operator!=(const SharedPtr<U>& other) const
    {
        return !(*this == other);
    }

    template<class T>
    void WeakPtr<T>::Reset(detail::SharedPtrControl* newControl, T* newObject)
    {
        if (control)
            control->DecreaseWeakCount();

        control = newControl;
        object = newObject;

        if (control)
            control->IncreaseWeakCount();
    }

    namespace detail
    {
        template<class T>
        void MakeSharedFromThis(SharedPtr<T>* object, typename T::EnableSharedFromThisType*)
        {
            static_cast<EnableSharedFromThis<typename T::EnableSharedFromThisType>&>(**object).MakeShared(*object);
        }

        inline void MakeSharedFromThis(void* object)
        {}
    }

    template<class T>
    SharedPtr<T> EnableSharedFromThis<T>::SharedFromThis()
    {
        return weakPtr;
    }

    template<class T>
    SharedPtr<const T> EnableSharedFromThis<T>::SharedFromThis() const
    {
        return weakPtr;
    }

    template<class T>
    WeakPtr<T> EnableSharedFromThis<T>::WeakFromThis()
    {
        return weakPtr;
    }

    template<class T>
    WeakPtr<const T> EnableSharedFromThis<T>::WeakFromThis() const
    {
        return weakPtr;
    }

    template<class T>
    void EnableSharedFromThis<T>::MakeShared(SharedPtr<T> sharedPtr)
    {
        weakPtr = sharedPtr;
    }

    namespace detail
    {
        class NullAllocator
            : public SharedObjectDeleter
        {
        public:
            virtual void Destruct(const void* object) override;
            virtual void Deallocate(void* control) override;
        };
    }

    template<class T>
    SharedPtr<T> AccessedBySharedPtr::MakeShared(T& value)
    {
        return SharedPtr<T>(&control, &value);
    }

    template<class T>
    SharedPtr<T> UnOwnedSharedPtr(T& object)
    {
        static detail::NullAllocator nullAllocator;
        static detail::SharedPtrControl control(nullptr, &nullAllocator);
        return SharedPtr<T>(&control, &object);
    }

    template<class T, class U>
    SharedPtr<T> MakeContainedSharedObject(T& object, const SharedPtr<U>& container)
    {
        return SharedPtr<T>(container.control, &object);
    }

    template<class T, class U>
    SharedPtr<T> MakeContainedSharedObject(T& object, SharedPtr<U>&& container)
    {
        SharedPtr<T> result(container.control, &object);
        container = nullptr;
        return result;
    }

    namespace detail
    {
        template<class T>
        class SharedObjectOnHeap
            : private SharedObjectDeleter
        {
        public:
            template<class... Args>
            SharedObjectOnHeap(Args&&... args)
                : control(&*object, this)
            {
                object.Construct(std::forward<Args>(args)...);
            }

            operator SharedPtr<T>()
            {
                return SharedPtr<T>(&control, &*object);
            }

        private:
            virtual void Destruct(const void* object) override
            {
                this->object.Destruct();
            }

            virtual void Deallocate(void* control) override
            {
                delete this;
            }

        private:
            StaticStorage<T> object;
            SharedPtrControl control;
        };
    }

    template<class T, class... Args>
    SharedPtr<T> MakeSharedOnHeap(Args&&... args)
    {
        return *new detail::SharedObjectOnHeap<T>(std::forward<Args>(args)...);
    }
}

#endif
