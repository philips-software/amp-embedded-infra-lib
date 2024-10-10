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
//      opt.emplace(2, 3, 4);       // Late construction by explicitly calling emplace
//  opt = MyClass(2, 3, 4);         // Use the copy constructor for construction

#include "infra/util/StaticStorage.hpp"
#include <optional>
#include <utility>

namespace infra
{
    // clang-format off
    using None = std::nullopt_t;
    inline const None none = std::nullopt;

    using InPlace = std::in_place_t;
    inline const InPlace inPlace = std::in_place;

    template<class T>
    using InPlaceType = std::in_place_type_t<T>;

    // clang-format on

    template<class T>
    using Optional = std::optional<T>;

    template<class T, class F>
    auto TransformOptional(const std::optional<T>& value, F transformation) -> std::optional<decltype(transformation(*value))>;

    template<class T, std::size_t ExtraSize>
    class OptionalForPolymorphicObjects
    {
    public:
        OptionalForPolymorphicObjects() = default;

        OptionalForPolymorphicObjects(const OptionalForPolymorphicObjects& other) = delete;
        template<class Derived, std::size_t OtherExtraSize>
        OptionalForPolymorphicObjects(const OptionalForPolymorphicObjects<Derived, OtherExtraSize>& other, typename std::enable_if<std::is_base_of<T, Derived>::value>::type* = 0);
        OptionalForPolymorphicObjects(None);
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
        void emplace(Args&&... args);

        const T& operator*() const;
        T& operator*();
        const T* operator->() const;
        T* operator->();

        explicit operator bool() const;

        bool operator!() const;
        bool operator==(const OptionalForPolymorphicObjects& other) const;
        bool operator!=(const OptionalForPolymorphicObjects& other) const;

        friend bool operator==(const OptionalForPolymorphicObjects& x, const None&)
        {
            return !x;
        }

        friend bool operator!=(const OptionalForPolymorphicObjects& x, const None& y)
        {
            return !(x == y);
        }

        friend bool operator==(const None& x, const OptionalForPolymorphicObjects& y)
        {
            return y == x;
        }

        friend bool operator!=(const None& x, const OptionalForPolymorphicObjects& y)
        {
            return y != x;
        }

        friend bool operator==(const OptionalForPolymorphicObjects& x, const T& y)
        {
            return x && *x == y;
        }

        friend bool operator!=(const OptionalForPolymorphicObjects& x, const T& y)
        {
            return !(x == y);
        }

        friend bool operator==(const T& x, const OptionalForPolymorphicObjects& y)
        {
            return y == x;
        }

        friend bool operator!=(const T& x, const OptionalForPolymorphicObjects& y)
        {
            return y != x;
        }

    private:
        void Reset();

    private:
        bool initialized = false;
        StaticStorageForPolymorphicObjects<T, ExtraSize> data;
    };

    ////    Implementation    ////

    template<class T, class F>
    auto TransformOptional(const infra::Optional<T>& value, F transformation) -> infra::Optional<decltype(transformation(*value))>
    {
        if (value != infra::none)
            return std::make_optional(transformation(*value));
        else
            return infra::none;
    }

    template<class T, std::size_t ExtraSize>
    template<class Derived, std::size_t OtherExtraSize>
    OptionalForPolymorphicObjects<T, ExtraSize>::OptionalForPolymorphicObjects(const OptionalForPolymorphicObjects<Derived, OtherExtraSize>& other, typename std::enable_if<std::is_base_of<T, Derived>::value>::type*)
    {
        if (other)
            emplace<Derived>(*other);
    }

    template<class T, std::size_t ExtraSize>
    OptionalForPolymorphicObjects<T, ExtraSize>::OptionalForPolymorphicObjects(None)
    {}

    template<class T, std::size_t ExtraSize>
    template<class Derived, class... Args>
    OptionalForPolymorphicObjects<T, ExtraSize>::OptionalForPolymorphicObjects(InPlaceType<Derived>, Args&&... args)
    {
        emplace<T>(std::forward<Args>(args)...);
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
            emplace<Derived>(*other);
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
        emplace<T>(value);
        return *this;
    }

    template<class T, std::size_t ExtraSize>
    OptionalForPolymorphicObjects<T, ExtraSize>& OptionalForPolymorphicObjects<T, ExtraSize>::operator=(T&& value)
    {
        Reset();
        emplace<T>(std::forward<T>(value));
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
    void OptionalForPolymorphicObjects<T, ExtraSize>::emplace(Args&&... args)
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
