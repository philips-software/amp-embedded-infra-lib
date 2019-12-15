#ifndef INFRA_OPTIONAL_HPP
#define INFRA_OPTIONAL_HPP

//  Optional<T> is a class that either holds an object of type T, or it does not. With
//  this class, you can delay construction of an object, or hasten destruction. The storage
//  needed is allocated inside of the optional. Basically, sizeof(Optional<T>) == sizeof(T) + 1.
//  It uses pointer-like operations for access. Examples:
//  
//  Optional<MyClass> opt(inPlace, 2, 3, 4); // 2, 3, 4 is forwarded to the constructor of MyClass
//
//  if (opt)                        // Test whether opt contains a value
//      opt->MyFunc();              // Use * and -> to access the value
//  opt = nullptr;                  // Reset the optional by assigning nullptr
//  if (!opt)
//      opt.Emplace(2, 3, 4);       // Late construction by explicitly calling Emplace
//  opt = MyClass(2, 3, 4);         // Use the copy constructor for construction

#include "infra/util/StaticStorage.hpp"
#include <cassert>
#include <memory>
#include <type_traits>

namespace infra
{
    extern const struct None {} none;
    extern const struct InPlace {} inPlace;

    template<class T>
        struct InPlaceType {};

    template<class T>
    class Optional
    {
    public:
        constexpr Optional() = default;

        Optional(const Optional& other);
        Optional(Optional&& other);
        Optional(None);                                             //TICS !INT#001
        template<class U>
            explicit Optional(const Optional<U>& other);
        template<class... Args>
            explicit Optional(InPlace, Args&&... args);

        ~Optional();

        Optional& operator=(const Optional& other);
        Optional& operator=(Optional&& other);
        Optional& operator=(const None&);
        Optional& operator=(const T& value);
        Optional& operator=(T&& value);

        template<class... Args>
            void Emplace(Args&&... args);
        template<class U, class... Args>
            void Emplace(std::initializer_list<U> list, Args&&... args);

        const T& operator*() const;
        T& operator*();
        const T* operator->() const;
        T* operator->();

        explicit operator bool() const;

        bool operator!() const;
        bool operator==(const Optional& other) const;
        bool operator!=(const Optional& other) const;
        friend bool operator==(const Optional& x, const None& y) { return !x; }
        friend bool operator!=(const Optional& x, const None& y) { return !(x == y); }
        friend bool operator==(const None& x, const Optional& y) { return y == x; }
        friend bool operator!=(const None& x, const Optional& y) { return y != x; }
        friend bool operator==(const Optional& x, const T& y) { return x && *x == y; }
        friend bool operator!=(const Optional& x, const T& y) { return !(x == y); }
        friend bool operator==(const T& x, const Optional& y) { return y == x; }
        friend bool operator!=(const T& x, const Optional& y) { return y != x; }

        template<class U>
            T ValueOr(U&& value) const;
        T ValueOrDefault() const;

    private:
        void Reset();

    private:
        bool initialized = false;
        StaticStorage<T> data;
    };

    template<class T> 
        Optional<typename std::decay<T>::type> MakeOptional(T&& value);

    template<class T, std::size_t ExtraSize>
    class OptionalForPolymorphicObjects
    {
    public:
        OptionalForPolymorphicObjects() = default;

        OptionalForPolymorphicObjects(const OptionalForPolymorphicObjects& other) = delete;
        template<class Derived, std::size_t OtherExtraSize>                                             //TICS !INT#001
            OptionalForPolymorphicObjects(const OptionalForPolymorphicObjects<Derived, OtherExtraSize>& other, typename std::enable_if<std::is_base_of<T, Derived>::value>::type* = 0);
        OptionalForPolymorphicObjects(None);                                                            //TICS !INT#001
        template<class Derived, class... Args>
            OptionalForPolymorphicObjects(InPlaceType<Derived>, Args&&... args);

        ~OptionalForPolymorphicObjects();

        OptionalForPolymorphicObjects& operator=(const OptionalForPolymorphicObjects& other) = delete;
        template<class Derived, std::size_t OtherExtraSize>
            typename std::enable_if<std::is_base_of<T, Derived>::value, OptionalForPolymorphicObjects&>::type
            operator=(const OptionalForPolymorphicObjects<Derived, OtherExtraSize>& other);
        OptionalForPolymorphicObjects& operator=(const None&);
        OptionalForPolymorphicObjects& operator=(const T& value);
        OptionalForPolymorphicObjects& operator=(T&& value);

        template<class Derived, class... Args>
            void Emplace(Args&&... args);

        const T& operator*() const;
        T& operator*();
        const T* operator->() const;
        T* operator->();

        explicit operator bool() const;

        bool operator!() const;
        bool operator==(const OptionalForPolymorphicObjects& other) const;
        bool operator!=(const OptionalForPolymorphicObjects& other) const;
        friend bool operator==(const OptionalForPolymorphicObjects& x, const None& y) { return !x; }
        friend bool operator!=(const OptionalForPolymorphicObjects& x, const None& y) { return !(x == y); }
        friend bool operator==(const None& x, const OptionalForPolymorphicObjects& y) { return y == x; }
        friend bool operator!=(const None& x, const OptionalForPolymorphicObjects& y) { return y != x; }
        friend bool operator==(const OptionalForPolymorphicObjects& x, const T& y) { return x && *x == y; }
        friend bool operator!=(const OptionalForPolymorphicObjects& x, const T& y) { return !(x == y); }
        friend bool operator==(const T& x, const OptionalForPolymorphicObjects& y) { return y == x; }
        friend bool operator!=(const T& x, const OptionalForPolymorphicObjects& y) { return y != x; }

    private:
        void Reset();

    private:
        bool initialized = false;
        StaticStorageForPolymorphicObjects<T, ExtraSize> data;
    };
        
    ////    Implementation    ////

    template<class T>
    Optional<T>::Optional(const Optional& other)
    {
        if (other)
            Emplace(*other);
    }

    template<class T>
    Optional<T>::Optional(Optional&& other)
    {
        if (other)
        {
            Emplace(std::move(*other));
            other.Reset();
        }
    }

    template<class T>
    Optional<T>::Optional(None)
    {}

    template<class T>
    template<class U>
    Optional<T>::Optional(const Optional<U>& other)
    {
        if (other)
            Emplace(*other);
    }

    template<class T>
    template<class... Args>
    Optional<T>::Optional(InPlace, Args&&... args)
    {
        Emplace(std::forward<Args>(args)...);
    }

    template<class T>
    Optional<T>::~Optional()
    {
        Reset();
    }

    template<class T>
    Optional<T>& Optional<T>::operator=(const Optional& other)
    {
        if (this != &other)
        {
            Reset();
            if (other)
                Emplace(*other);
        }

        return *this;
    }

    template<class T>
    Optional<T>& Optional<T>::operator=(Optional&& other)
    {
        Reset();
        if (other)
        {
            Emplace(std::move(*other));
            other.Reset();
        }

        return *this;
    }

    template<class T>
    Optional<T>& Optional<T>::operator=(const None&)
    {
        Reset();
        return *this;
    }

    template<class T>
    Optional<T>& Optional<T>::operator=(const T& value)
    {
        Reset();
        Emplace(value);
        return *this;
    }

    template<class T>
    Optional<T>& Optional<T>::operator=(T&& value)
    {
        Reset();
        Emplace(std::forward<T>(value));
        return *this;
    }

    template<class T>
    const T& Optional<T>::operator*() const
    {
        assert(initialized);
        return *data;
    }

    template<class T>
    T& Optional<T>::operator*()
    {
        assert(initialized);
        return *data;
    }

    template<class T>
    const T* Optional<T>::operator->() const
    {
        assert(initialized);
        return &*data;
    }

    template<class T>
    T* Optional<T>::operator->()
    {
        assert(initialized);
        return &*data;
    }

    template<class T>
    Optional<T>::operator bool() const
    {
        return initialized;
    }

    template<class T>
    bool Optional<T>::operator!() const
    {
        return !initialized;
    }

    template<class T>
    bool Optional<T>::operator==(const Optional& other) const
    {
        if (initialized && other.initialized)
            return **this == *other;
        else
            return initialized == other.initialized;
    }

    template<class T>
    bool Optional<T>::operator!=(const Optional& other) const
    {
        return !(*this == other);
    }

    template<class T>
    template<class U>
    T Optional<T>::ValueOr(U&& value) const
    {
        if (initialized)
            return **this;
        else
            return std::move(value);
    }

    template<class T>
    T Optional<T>::ValueOrDefault() const
    {
        if (initialized)
            return **this;
        else
            return T();
    }

    template<class T>
    template<class... Args>
    void Optional<T>::Emplace(Args&&... args)
    {
        Reset();
        initialized = true;
        data.Construct(std::forward<Args>(args)...);
    }

    template<class T>
    template<class U, class... Args>
    void Optional<T>::Emplace(std::initializer_list<U> list, Args&&... args)
    {
        Reset();
        initialized = true;
        data.Construct(list, std::forward<Args>(args)...);
    }

    template<class T>
    void Optional<T>::Reset()
    {
        if (initialized)
        {
            data.Destruct();
            initialized = false;
        }
    }

    template<class T> 
    Optional<typename std::decay<T>::type> MakeOptional(T&& value)
    {
        return Optional<typename std::decay<T>::type>(inPlace, std::forward<T>(value));
    }

    template<class T, class F>
    auto TransformOptional(const infra::Optional<T>& value, F transformation) -> infra::Optional<decltype(transformation(*value))>
    {
        if (value != infra::none)
            return infra::MakeOptional(transformation(*value));
        else
            return infra::none;
    }

    template<class T, std::size_t ExtraSize>
    template<class Derived, std::size_t OtherExtraSize>
    OptionalForPolymorphicObjects<T, ExtraSize>::OptionalForPolymorphicObjects(const OptionalForPolymorphicObjects<Derived, OtherExtraSize>& other
            , typename std::enable_if<std::is_base_of<T, Derived>::value>::type*)
    {
        if (other)
            Emplace<Derived>(*other);
    }

    template<class T, std::size_t ExtraSize>
    OptionalForPolymorphicObjects<T, ExtraSize>::OptionalForPolymorphicObjects(None)
    {}

    template<class T, std::size_t ExtraSize>
    template<class Derived, class... Args>
    OptionalForPolymorphicObjects<T, ExtraSize>::OptionalForPolymorphicObjects(InPlaceType<Derived>, Args&&... args)
    {
        Emplace<T>(std::forward<Args>(args)...);
    }

    template<class T, std::size_t ExtraSize>
    OptionalForPolymorphicObjects<T, ExtraSize>::~OptionalForPolymorphicObjects()
    {
        Reset();
    }

    template<class T, std::size_t ExtraSize>
    template<class Derived, std::size_t OtherExtraSize>
    typename std::enable_if<std::is_base_of<T, Derived>::value, OptionalForPolymorphicObjects<T, ExtraSize>&>::type
        OptionalForPolymorphicObjects<T, ExtraSize>::operator=(const OptionalForPolymorphicObjects<Derived, OtherExtraSize>& other)
    {
        Reset();
        if (other)
            Emplace<Derived>(*other);
        return *this;
    }

    template<class T, std::size_t ExtraSize>
    OptionalForPolymorphicObjects<T, ExtraSize>& OptionalForPolymorphicObjects<T, ExtraSize>::operator=(const None&)
    {
        Reset();
        return *this;
    }

    template<class T, std::size_t ExtraSize>
    OptionalForPolymorphicObjects<T, ExtraSize>& OptionalForPolymorphicObjects<T, ExtraSize>::operator=(const T& value)
    {
        Reset();
        Emplace<T>(value);
        return *this;
    }

    template<class T, std::size_t ExtraSize>
    OptionalForPolymorphicObjects<T, ExtraSize>& OptionalForPolymorphicObjects<T, ExtraSize>::operator=(T&& value)
    {
        Reset();
        Emplace<T>(std::forward<T>(value));
        return *this;
    }

    template<class T, std::size_t ExtraSize>
    const T& OptionalForPolymorphicObjects<T, ExtraSize>::operator*() const
    {
        return *data;
    }

    template<class T, std::size_t ExtraSize>
    T& OptionalForPolymorphicObjects<T, ExtraSize>::operator*()
    {
        return *data;
    }

    template<class T, std::size_t ExtraSize>
    const T* OptionalForPolymorphicObjects<T, ExtraSize>::operator->() const
    {
        return &*data;
    }

    template<class T, std::size_t ExtraSize>
    T* OptionalForPolymorphicObjects<T, ExtraSize>::operator->()
    {
        return &*data;
    }

    template<class T, std::size_t ExtraSize>
    OptionalForPolymorphicObjects<T, ExtraSize>::operator bool() const
    {
        return initialized;
    }

    template<class T, std::size_t ExtraSize>
    bool OptionalForPolymorphicObjects<T, ExtraSize>::operator!() const
    {
        return !initialized;
    }

    template<class T, std::size_t ExtraSize>
    bool OptionalForPolymorphicObjects<T, ExtraSize>::operator==(const OptionalForPolymorphicObjects& other) const
    {
        if (initialized && other.initialized)
            return **this == *other;
        else
            return initialized == other.initialized;
    }

    template<class T, std::size_t ExtraSize>
    bool OptionalForPolymorphicObjects<T, ExtraSize>::operator!=(const OptionalForPolymorphicObjects& other) const
    {
        return !(*this == other);
    }

    template<class T, std::size_t ExtraSize>
    template<class Derived, class... Args>
    void OptionalForPolymorphicObjects<T, ExtraSize>::Emplace(Args&&... args)
    {
        Reset();
        initialized = true;
        data.template Construct<Derived>(std::forward<Args>(args)...);
    }

    template<class T, std::size_t ExtraSize>
    void OptionalForPolymorphicObjects<T, ExtraSize>::Reset()
    {
        if (initialized)
        {
            data.Destruct();
            initialized = false;
        }
    }
}

#endif
