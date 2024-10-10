#ifndef INFRA_PROXY_CREATOR_HPP
#define INFRA_PROXY_CREATOR_HPP

#include "infra/util/Function.hpp"
#include "infra/util/Optional.hpp"
#include <tuple>

namespace infra
{
    template<class T, class ConstructionArgs>
    class CreatorBase;

    template<class T, class... ConstructionArgs>
    class ProxyCreator;

    template<class T, class ConstructionArgs>
    class DelayedProxyCreator;

    template<class T, class U, class ConstructionArgs>
    class Creator;

    template<class T, class ConstructionArgs>
    class CreatorExternal;

    template<class T, class... ConstructionArgs>
    class ProxyCreator<T, void(ConstructionArgs...)>
    {
    public:
        ProxyCreator(CreatorBase<T, void(ConstructionArgs...)>& creator, ConstructionArgs... args);
        ProxyCreator(const ProxyCreator& other) = delete;
        ProxyCreator& operator=(const ProxyCreator& other) = delete;
        ~ProxyCreator();

        T& operator*();
        const T& operator*() const;
        T* operator->();
        const T* operator->() const;

    private:
        CreatorBase<T, void(ConstructionArgs...)>& creator;
    };

    template<class T, class... ConstructionArgs>
    class ProxyCreator<CreatorBase<T, void(ConstructionArgs...)>&>
        : public ProxyCreator<T, void(ConstructionArgs...)>
    {
    public:
        using ProxyCreator<T, void(ConstructionArgs...)>::ProxyCreator;
    };

    template<class... ConstructionArgs>
    class ProxyCreator<void, void(ConstructionArgs...)>
    {
    public:
        ProxyCreator(CreatorBase<void, void(ConstructionArgs...)>& creator, ConstructionArgs... args);
        ProxyCreator(const ProxyCreator& other) = delete;
        ProxyCreator& operator=(const ProxyCreator& other) = delete;
        ~ProxyCreator();

    private:
        CreatorBase<void, void(ConstructionArgs...)>& creator;
    };

    template<class T, class... ConstructionArgs>
    class DelayedProxyCreator<T, void(ConstructionArgs...)>
    {
    public:
        explicit DelayedProxyCreator(CreatorBase<T, void(ConstructionArgs...)>& creator);
        DelayedProxyCreator(const DelayedProxyCreator& other) = delete;
        DelayedProxyCreator& operator=(const DelayedProxyCreator& other) = delete;
        ~DelayedProxyCreator();

        void Emplace(ConstructionArgs... args);
        void Destroy();

        T& operator*();
        const T& operator*() const;
        T* operator->();
        const T* operator->() const;

    private:
        CreatorBase<T, void(ConstructionArgs...)>& creator;
    };

    template<class T, class... ConstructionArgs>
    class CreatorBase<T, void(ConstructionArgs...)>
    {
    public:
        template<class Concrete>
        using WithCreator = Creator<T, Concrete, void(ConstructionArgs...)>;

        using ProxyCreator = infra::ProxyCreator<T, void(ConstructionArgs...)>;
        using DelayedProxyCreator = infra::DelayedProxyCreator<T, void(ConstructionArgs...)>;

    protected:
        CreatorBase() = default;
        ~CreatorBase() = default;

    protected:
        virtual T& Get() = 0;
        virtual const T& Get() const = 0;

        virtual void Emplace(ConstructionArgs... args) = 0;
        virtual void Destroy() = 0;

    private:
        template<class T2, class... ConstructionArgs2>
        friend class infra::ProxyCreator;

        template<class T2, class ConstructionArgs2>
        friend class infra::DelayedProxyCreator;
    };

    template<class... ConstructionArgs>
    class CreatorBase<void, void(ConstructionArgs...)>
    {
    protected:
        CreatorBase() = default;
        ~CreatorBase() = default;

    protected:
        virtual void Emplace(ConstructionArgs... args) = 0;
        virtual void Destroy() = 0;

    private:
        template<class T2, class... ConstructionArgs2>
        friend class ProxyCreator;

        template<class T2, class ConstructionArgs2>
        friend class DelayedProxyCreator;
    };

    template<class T, class U, class... ConstructionArgs>
    class Creator<T, U, void(ConstructionArgs...)>
        : public CreatorBase<T, void(ConstructionArgs...)>
    {
    public:
        Creator();
        explicit Creator(infra::Function<void(infra::Optional<U>&, ConstructionArgs...)> emplaceFunction);

        void Emplace(ConstructionArgs... args) override;
        void Destroy() override;

        U& operator*();
        const U& operator*() const;
        U* operator->();
        const U* operator->() const;

    protected:
        T& Get() override;
        const T& Get() const override;
        U& GetObject();
        const U& GetObject() const;

    private:
        infra::Optional<U> object;
        infra::Function<void(infra::Optional<U>& object, ConstructionArgs... args)> emplaceFunction;
    };

    template<class U, class... ConstructionArgs>
    class Creator<void, U, void(ConstructionArgs...)>
        : public CreatorBase<void, void(ConstructionArgs...)>
    {
    public:
        Creator();
        explicit Creator(infra::Function<void(infra::Optional<U>&, ConstructionArgs...)> emplaceFunction);

        void Emplace(ConstructionArgs... args) override;
        void Destroy() override;

        U& operator*();
        const U& operator*() const;
        U* operator->();
        const U* operator->() const;

    protected:
        U& GetObject();
        const U& GetObject() const;

    private:
        infra::Optional<U> object;
        infra::Function<void(infra::Optional<U>& object, ConstructionArgs... args)> emplaceFunction;
    };

    template<class T, class... ConstructionArgs>
    class CreatorExternal<T, void(ConstructionArgs...)>
        : public CreatorBase<T, void(ConstructionArgs...)>
    {
    public:
        explicit CreatorExternal(const infra::Function<T&(ConstructionArgs...)>& emplaceFunction, const infra::Function<void()>& destroyFunction);

        void Emplace(ConstructionArgs... args) override;
        void Destroy() override;

    protected:
        T& Get() override;
        const T& Get() const override;

    private:
        infra::Function<T&(ConstructionArgs... args)> emplaceFunction;
        infra::Function<void()> destroyFunction;
        T* object = nullptr;
    };

    template<class... T, class... ConstructionArgs>
    class CreatorExternal<std::tuple<T...>, void(ConstructionArgs...)>
        : public CreatorBase<std::tuple<T...>, void(ConstructionArgs...)>
    {
    public:
        explicit CreatorExternal(const infra::Function<std::tuple<T...>(ConstructionArgs...)>& emplaceFunction, const infra::Function<void()>& destroyFunction);

        void Emplace(ConstructionArgs... args) override;
        void Destroy() override;

    protected:
        std::tuple<T...>& Get() override;
        const std::tuple<T...>& Get() const override;

    private:
        infra::Function<std::tuple<T...>(ConstructionArgs... args)> emplaceFunction;
        infra::Function<void()> destroyFunction;
        infra::Optional<std::tuple<T...>> object;
    };

    template<class... ConstructionArgs>
    class CreatorExternal<void, void(ConstructionArgs...)>
        : public CreatorBase<void, void(ConstructionArgs...)>
    {
    public:
        explicit CreatorExternal(const infra::Function<void(ConstructionArgs...)>& emplaceFunction, const infra::Function<void()>& destroyFunction);

        void Emplace(ConstructionArgs... args) override;
        void Destroy() override;

    private:
        infra::Function<void(ConstructionArgs... args)> emplaceFunction;
        infra::Function<void()> destroyFunction;
    };

    ////    Implementation    ////

    template<class T, class... ConstructionArgs>
    ProxyCreator<T, void(ConstructionArgs...)>::ProxyCreator(CreatorBase<T, void(ConstructionArgs...)>& creator, ConstructionArgs... args)
        : creator(creator)
    {
        creator.Emplace(args...);
    }

    template<class T, class... ConstructionArgs>
    ProxyCreator<T, void(ConstructionArgs...)>::~ProxyCreator()
    {
        creator.Destroy();
    }

    template<class T, class... ConstructionArgs>
    T& ProxyCreator<T, void(ConstructionArgs...)>::operator*()
    {
        return creator.Get();
    }

    template<class T, class... ConstructionArgs>
    const T& ProxyCreator<T, void(ConstructionArgs...)>::operator*() const
    {
        return creator.Get();
    }

    template<class T, class... ConstructionArgs>
    T* ProxyCreator<T, void(ConstructionArgs...)>::operator->()
    {
        return &creator.Get();
    }

    template<class T, class... ConstructionArgs>
    const T* ProxyCreator<T, void(ConstructionArgs...)>::operator->() const
    {
        return &creator.Get();
    }

    template<class... ConstructionArgs>
    ProxyCreator<void, void(ConstructionArgs...)>::ProxyCreator(CreatorBase<void, void(ConstructionArgs...)>& creator, ConstructionArgs... args)
        : creator(creator)
    {
        creator.Emplace(args...);
    }

    template<class... ConstructionArgs>
    ProxyCreator<void, void(ConstructionArgs...)>::~ProxyCreator()
    {
        creator.Destroy();
    }

    template<class T, class... ConstructionArgs>
    DelayedProxyCreator<T, void(ConstructionArgs...)>::DelayedProxyCreator(CreatorBase<T, void(ConstructionArgs...)>& creator)
        : creator(creator)
    {}

    template<class T, class... ConstructionArgs>
    DelayedProxyCreator<T, void(ConstructionArgs...)>::~DelayedProxyCreator()
    {
        creator.Destroy();
    }

    template<class T, class... ConstructionArgs>
    void DelayedProxyCreator<T, void(ConstructionArgs...)>::Emplace(ConstructionArgs... args)
    {
        creator.Emplace(args...);
    }

    template<class T, class... ConstructionArgs>
    void DelayedProxyCreator<T, void(ConstructionArgs...)>::Destroy()
    {
        creator.Destroy();
    }

    template<class T, class... ConstructionArgs>
    T& DelayedProxyCreator<T, void(ConstructionArgs...)>::operator*()
    {
        return creator.Get();
    }

    template<class T, class... ConstructionArgs>
    const T& DelayedProxyCreator<T, void(ConstructionArgs...)>::operator*() const
    {
        return creator.Get();
    }

    template<class T, class... ConstructionArgs>
    T* DelayedProxyCreator<T, void(ConstructionArgs...)>::operator->()
    {
        return &creator.Get();
    }

    template<class T, class... ConstructionArgs>
    const T* DelayedProxyCreator<T, void(ConstructionArgs...)>::operator->() const
    {
        return &creator.Get();
    }

    template<class T, class U, class... ConstructionArgs>
    Creator<T, U, void(ConstructionArgs...)>::Creator()
    {
        emplaceFunction = [](infra::Optional<U>& object, ConstructionArgs... args)
        {
            object.emplace(args...);
        };
    }

    template<class T, class U, class... ConstructionArgs>
    Creator<T, U, void(ConstructionArgs...)>::Creator(infra::Function<void(infra::Optional<U>&, ConstructionArgs...)> emplaceFunction)
        : emplaceFunction(emplaceFunction)
    {}

    template<class T, class U, class... ConstructionArgs>
    void Creator<T, U, void(ConstructionArgs...)>::Emplace(ConstructionArgs... args)
    {
        assert(!object);
        emplaceFunction(object, args...);
    }

    template<class T, class U, class... ConstructionArgs>
    void Creator<T, U, void(ConstructionArgs...)>::Destroy()
    {
        object = none;
    }

    template<class T, class U, class... ConstructionArgs>
    U& Creator<T, U, void(ConstructionArgs...)>::operator*()
    {
        return GetObject();
    }

    template<class T, class U, class... ConstructionArgs>
    const U& Creator<T, U, void(ConstructionArgs...)>::operator*() const
    {
        return GetObject();
    }

    template<class T, class U, class... ConstructionArgs>
    U* Creator<T, U, void(ConstructionArgs...)>::operator->()
    {
        return &GetObject();
    }

    template<class T, class U, class... ConstructionArgs>
    const U* Creator<T, U, void(ConstructionArgs...)>::operator->() const
    {
        return &GetObject();
    }

    template<class T, class U, class... ConstructionArgs>
    T& Creator<T, U, void(ConstructionArgs...)>::Get()
    {
        return *object;
    }

    template<class T, class U, class... ConstructionArgs>
    const T& Creator<T, U, void(ConstructionArgs...)>::Get() const
    {
        return *object;
    }

    template<class T, class U, class... ConstructionArgs>
    U& Creator<T, U, void(ConstructionArgs...)>::GetObject()
    {
        return *object;
    }

    template<class T, class U, class... ConstructionArgs>
    const U& Creator<T, U, void(ConstructionArgs...)>::GetObject() const
    {
        return *object;
    }

    template<class U, class... ConstructionArgs>
    Creator<void, U, void(ConstructionArgs...)>::Creator()
    {
        emplaceFunction = [](infra::Optional<U>& object, ConstructionArgs... args)
        {
            object.emplace(args...);
        };
    }

    template<class U, class... ConstructionArgs>
    Creator<void, U, void(ConstructionArgs...)>::Creator(infra::Function<void(infra::Optional<U>&, ConstructionArgs...)> emplaceFunction)
        : emplaceFunction(emplaceFunction)
    {}

    template<class U, class... ConstructionArgs>
    void Creator<void, U, void(ConstructionArgs...)>::Emplace(ConstructionArgs... args)
    {
        assert(!object);
        emplaceFunction(object, args...);
    }

    template<class U, class... ConstructionArgs>
    void Creator<void, U, void(ConstructionArgs...)>::Destroy()
    {
        object = none;
    }

    template<class U, class... ConstructionArgs>
    U& Creator<void, U, void(ConstructionArgs...)>::operator*()
    {
        return GetObject();
    }

    template<class U, class... ConstructionArgs>
    const U& Creator<void, U, void(ConstructionArgs...)>::operator*() const
    {
        return GetObject();
    }

    template<class U, class... ConstructionArgs>
    U* Creator<void, U, void(ConstructionArgs...)>::operator->()
    {
        return &GetObject();
    }

    template<class U, class... ConstructionArgs>
    const U* Creator<void, U, void(ConstructionArgs...)>::operator->() const
    {
        return &GetObject();
    }

    template<class U, class... ConstructionArgs>
    U& Creator<void, U, void(ConstructionArgs...)>::GetObject()
    {
        return *object;
    }

    template<class U, class... ConstructionArgs>
    const U& Creator<void, U, void(ConstructionArgs...)>::GetObject() const
    {
        return *object;
    }

    template<class T, class... ConstructionArgs>
    CreatorExternal<T, void(ConstructionArgs...)>::CreatorExternal(const infra::Function<T&(ConstructionArgs...)>& emplaceFunction, const infra::Function<void()>& destroyFunction)
        : emplaceFunction(emplaceFunction)
        , destroyFunction(destroyFunction)
    {}

    template<class T, class... ConstructionArgs>
    void CreatorExternal<T, void(ConstructionArgs...)>::Emplace(ConstructionArgs... args)
    {
        assert(object == nullptr);
        object = &emplaceFunction(args...);
    }

    template<class T, class... ConstructionArgs>
    void CreatorExternal<T, void(ConstructionArgs...)>::Destroy()
    {
        assert(object != nullptr);
        object = nullptr;
        destroyFunction();
    }

    template<class T, class... ConstructionArgs>
    T& CreatorExternal<T, void(ConstructionArgs...)>::Get()
    {
        return *object;
    }

    template<class T, class... ConstructionArgs>
    const T& CreatorExternal<T, void(ConstructionArgs...)>::Get() const
    {
        return *object;
    }

    template<class... T, class... ConstructionArgs>
    CreatorExternal<std::tuple<T...>, void(ConstructionArgs...)>::CreatorExternal(const infra::Function<std::tuple<T...>(ConstructionArgs...)>& emplaceFunction, const infra::Function<void()>& destroyFunction)
        : emplaceFunction(emplaceFunction)
        , destroyFunction(destroyFunction)
    {}

    template<class... T, class... ConstructionArgs>
    void CreatorExternal<std::tuple<T...>, void(ConstructionArgs...)>::Emplace(ConstructionArgs... args)
    {
        assert(object == infra::none);
        object.emplace(emplaceFunction(args...));
    }

    template<class... T, class... ConstructionArgs>
    void CreatorExternal<std::tuple<T...>, void(ConstructionArgs...)>::Destroy()
    {
        assert(object != infra::none);
        object = infra::none;
        destroyFunction();
    }

    template<class... T, class... ConstructionArgs>
    std::tuple<T...>& CreatorExternal<std::tuple<T...>, void(ConstructionArgs...)>::Get()
    {
        return *object;
    }

    template<class... T, class... ConstructionArgs>
    const std::tuple<T...>& CreatorExternal<std::tuple<T...>, void(ConstructionArgs...)>::Get() const
    {
        return *object;
    }

    template<class... ConstructionArgs>
    CreatorExternal<void, void(ConstructionArgs...)>::CreatorExternal(const infra::Function<void(ConstructionArgs...)>& emplaceFunction, const infra::Function<void()>& destroyFunction)
        : emplaceFunction(emplaceFunction)
        , destroyFunction(destroyFunction)
    {}

    template<class... ConstructionArgs>
    void CreatorExternal<void, void(ConstructionArgs...)>::Emplace(ConstructionArgs... args)
    {
        emplaceFunction(args...);
    }

    template<class... ConstructionArgs>
    void CreatorExternal<void, void(ConstructionArgs...)>::Destroy()
    {
        destroyFunction();
    }
}

#endif
