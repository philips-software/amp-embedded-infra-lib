#ifndef INFRA_PROXY_CREATOR_HPP
#define INFRA_PROXY_CREATOR_HPP

#include "infra/util/Function.hpp"
#include "infra/util/Optional.hpp"

namespace infra
{
    template<class T, class... ConstructionArgs>
    class CreatorBase;

    template<class T, class... ConstructionArgs>
    class ProxyCreator
    {
    public:
        ProxyCreator(CreatorBase<T, ConstructionArgs...>& creator, ConstructionArgs... args);
        ~ProxyCreator();

        T& operator*();
        const T& operator*() const;
        T* operator->();
        const T* operator->() const;

    private:
        CreatorBase<T, ConstructionArgs...>& creator;
    };

    template<class T, class... ConstructionArgs>
    class ProxyCreator<CreatorBase<T, ConstructionArgs...>&>
        : public ProxyCreator<T, ConstructionArgs...>
    {
    public:
        using ProxyCreator<T, ConstructionArgs...>::ProxyCreator;
    };

    template<class... ConstructionArgs>
    class ProxyCreator<void, ConstructionArgs...>
    {
    public:
        ProxyCreator(CreatorBase<void, ConstructionArgs...>& creator, ConstructionArgs... args);
        ~ProxyCreator();

    private:
        CreatorBase<void, ConstructionArgs...>& creator;
    };

    template<class T, class... ConstructionArgs>
    class DelayedProxyCreator
    {
    public:
        explicit DelayedProxyCreator(CreatorBase<T, ConstructionArgs...>& creator);
        ~DelayedProxyCreator();

        void Emplace(ConstructionArgs... args);
        void Destroy();

        T& operator*();
        const T& operator*() const;
        T* operator->();
        const T* operator->() const;

    private:
        CreatorBase<T, ConstructionArgs...>& creator;
    };

    template<class T, class U, class... ConstructionArgs>
    class Creator;

    template<class T, class... ConstructionArgs>
    class CreatorBase
    {
    public:
        template<class Concrete>
            using WithCreator = Creator<T, Concrete, ConstructionArgs...>;

        using ProxyCreator = infra::ProxyCreator<T, ConstructionArgs...>;

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

        template<class T2, class... ConstructionArgs2>
        friend class DelayedProxyCreator;
    };

    template<class... ConstructionArgs>
    class CreatorBase<void, ConstructionArgs...>
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

        template<class T2, class... ConstructionArgs2>
        friend class DelayedProxyCreator;
    };

    template<class T, class U, class... ConstructionArgs>
    class Creator
        : public CreatorBase<T, ConstructionArgs...>
    {
    public:
        Creator();
        explicit Creator(infra::Function<void(infra::Optional<U>&, ConstructionArgs...)> emplaceFunction);

        virtual void Emplace(ConstructionArgs... args) override;
        virtual void Destroy() override;

        U& operator*();
        const U& operator*() const;
        U* operator->();
        const U* operator->() const;

    protected:
        virtual T& Get() override;
        virtual const T& Get() const override;
        U& GetObject();
        const U& GetObject() const;

    private:
        infra::Optional<U> object;
        infra::Function<void(infra::Optional<U>& object, ConstructionArgs... args)> emplaceFunction;
    };

    template<class U, class... ConstructionArgs>
    class Creator<void, U, ConstructionArgs...>
        : public CreatorBase<void, ConstructionArgs...>
    {
    public:
        Creator();
        explicit Creator(infra::Function<void(infra::Optional<U>&, ConstructionArgs...)> emplaceFunction);

        virtual void Emplace(ConstructionArgs... args) override;
        virtual void Destroy() override;

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
    class CreatorExternal
        : public CreatorBase<T, ConstructionArgs...>
    {
    public:
        explicit CreatorExternal(const infra::Function<T&(ConstructionArgs...)>& emplaceFunction, const infra::Function<void()>& destroyFunction);

        virtual void Emplace(ConstructionArgs... args) override;
        virtual void Destroy() override;

    protected:
        virtual T& Get() override;
        virtual const T& Get() const override;

    private:
        infra::Function<T&(ConstructionArgs... args)> emplaceFunction;
        infra::Function<void()> destroyFunction;
        T* object = nullptr;
    };

    ////    Implementation    ////

    template<class T, class... ConstructionArgs>
    ProxyCreator<T, ConstructionArgs...>::ProxyCreator(CreatorBase<T, ConstructionArgs...>& creator, ConstructionArgs... args)
        : creator(creator)
    {
        creator.Emplace(args...);
    }

    template<class T, class... ConstructionArgs>
    ProxyCreator<T, ConstructionArgs...>::~ProxyCreator()
    {
        creator.Destroy();
    }

    template<class T, class... ConstructionArgs>
    T& ProxyCreator<T, ConstructionArgs...>::operator*()
    {
        return creator.Get();
    }

    template<class T, class... ConstructionArgs>
    const T& ProxyCreator<T, ConstructionArgs...>::operator*() const
    {
        return creator.Get();
    }

    template<class T, class... ConstructionArgs>
    T* ProxyCreator<T, ConstructionArgs...>::operator->()
    {
        return &creator.Get();
    }

    template<class T, class... ConstructionArgs>
    const T* ProxyCreator<T, ConstructionArgs...>::operator->() const
    {
        return &creator.Get();
    }

    template<class... ConstructionArgs>
    ProxyCreator<void, ConstructionArgs...>::ProxyCreator(CreatorBase<void, ConstructionArgs...>& creator, ConstructionArgs... args)
        : creator(creator)
    {
        creator.Emplace(args...);
    }

    template<class... ConstructionArgs>
    ProxyCreator<void, ConstructionArgs...>::~ProxyCreator()
    {
        creator.Destroy();
    }

    template<class T, class... ConstructionArgs>
    DelayedProxyCreator<T, ConstructionArgs...>::DelayedProxyCreator(CreatorBase<T, ConstructionArgs...>& creator)
        : creator(creator)
    {}

    template<class T, class... ConstructionArgs>
    DelayedProxyCreator<T, ConstructionArgs...>::~DelayedProxyCreator()
    {
        creator.Destroy();
    }

    template<class T, class... ConstructionArgs>
    void DelayedProxyCreator<T, ConstructionArgs...>::Emplace(ConstructionArgs... args)
    {
        creator.Emplace(args...);
    }

    template<class T, class... ConstructionArgs>
    void DelayedProxyCreator<T, ConstructionArgs...>::Destroy()
    {
        creator.Destroy();
    }

    template<class T, class... ConstructionArgs>
    T& DelayedProxyCreator<T, ConstructionArgs...>::operator*()
    {
        return creator.Get();
    }

    template<class T, class... ConstructionArgs>
    const T& DelayedProxyCreator<T, ConstructionArgs...>::operator*() const
    {
        return creator.Get();
    }

    template<class T, class... ConstructionArgs>
    T* DelayedProxyCreator<T, ConstructionArgs...>::operator->()
    {
        return &creator.Get();
    }

    template<class T, class... ConstructionArgs>
    const T* DelayedProxyCreator<T, ConstructionArgs...>::operator->() const
    {
        return &creator.Get();
    }

    template<class T, class U, class... ConstructionArgs>
    Creator<T, U, ConstructionArgs...>::Creator()
    {
        emplaceFunction = [](infra::Optional<U>& object, ConstructionArgs... args) { object.Emplace(args...); };
    }

    template<class T, class U, class... ConstructionArgs>
    Creator<T, U, ConstructionArgs...>::Creator(infra::Function<void(infra::Optional<U>&, ConstructionArgs...)> emplaceFunction)
        : emplaceFunction(emplaceFunction)
    {}

    template<class T, class U, class... ConstructionArgs>
    void Creator<T, U, ConstructionArgs...>::Emplace(ConstructionArgs... args)
    {
        assert(!object);
        emplaceFunction(object, args...);
    }

    template<class T, class U, class... ConstructionArgs>
    void Creator<T, U, ConstructionArgs...>::Destroy()
    {
        object = none;
    }

    template<class T, class U, class... ConstructionArgs>
    U& Creator<T, U, ConstructionArgs...>::operator*()
    {
        return GetObject();
    }

    template<class T, class U, class... ConstructionArgs>
    const U& Creator<T, U, ConstructionArgs...>::operator*() const
    {
        return GetObject();
    }

    template<class T, class U, class... ConstructionArgs>
    U* Creator<T, U, ConstructionArgs...>::operator->()
    {
        return &GetObject();
    }

    template<class T, class U, class... ConstructionArgs>
    const U* Creator<T, U, ConstructionArgs...>::operator->() const
    {
        return &GetObject();
    }

    template<class T, class U, class... ConstructionArgs>
    T& Creator<T, U, ConstructionArgs...>::Get()
    {
        return *object;
    }

    template<class T, class U, class... ConstructionArgs>
    const T& Creator<T, U, ConstructionArgs...>::Get() const
    {
        return *object;
    }

    template<class T, class U, class... ConstructionArgs>
    U& Creator<T, U, ConstructionArgs...>::GetObject()
    {
        return *object;
    }

    template<class T, class U, class... ConstructionArgs>
    const U& Creator<T, U, ConstructionArgs...>::GetObject() const
    {
        return *object;
    }

    template<class U, class... ConstructionArgs>
    Creator<void, U, ConstructionArgs...>::Creator()
    {
        emplaceFunction = [](infra::Optional<U>& object, ConstructionArgs... args) { object.Emplace(args...); };
    }

    template<class U, class... ConstructionArgs>
    Creator<void, U, ConstructionArgs...>::Creator(infra::Function<void(infra::Optional<U>&, ConstructionArgs...)> emplaceFunction)
        : emplaceFunction(emplaceFunction)
    {}

    template<class U, class... ConstructionArgs>
    void Creator<void, U, ConstructionArgs...>::Emplace(ConstructionArgs... args)
    {
        assert(!object);
        emplaceFunction(object, args...);
    }

    template<class U, class... ConstructionArgs>
    void Creator<void, U, ConstructionArgs...>::Destroy()
    {
        object = none;
    }

    template<class U, class... ConstructionArgs>
    U& Creator<void, U, ConstructionArgs...>::operator*()
    {
        return GetObject();
    }

    template<class U, class... ConstructionArgs>
    const U& Creator<void, U, ConstructionArgs...>::operator*() const
    {
        return GetObject();
    }

    template<class U, class... ConstructionArgs>
    U* Creator<void, U, ConstructionArgs...>::operator->()
    {
        return &GetObject();
    }

    template<class U, class... ConstructionArgs>
    const U* Creator<void, U, ConstructionArgs...>::operator->() const
    {
        return &GetObject();
    }

    template<class U, class... ConstructionArgs>
    U& Creator<void, U, ConstructionArgs...>::GetObject()
    {
        return *object;
    }

    template<class U, class... ConstructionArgs>
    const U& Creator<void, U, ConstructionArgs...>::GetObject() const
    {
        return *object;
    }

    template<class T, class... ConstructionArgs>
    CreatorExternal<T, ConstructionArgs...>::CreatorExternal(const infra::Function<T&(ConstructionArgs...)>& emplaceFunction, const infra::Function<void()>& destroyFunction)
        : emplaceFunction(emplaceFunction)
        , destroyFunction(destroyFunction)
    {}

    template<class T, class... ConstructionArgs>
    void CreatorExternal<T, ConstructionArgs...>::Emplace(ConstructionArgs... args)
    {
        assert(object == nullptr);
        object = &emplaceFunction(args...);
    }

    template<class T, class... ConstructionArgs>
    void CreatorExternal<T, ConstructionArgs...>::Destroy()
    {
        assert(object != nullptr);
        object = nullptr;
        destroyFunction();
    }

    template<class T, class... ConstructionArgs>
    T& CreatorExternal<T, ConstructionArgs...>::Get()
    {
        return *object;
    }

    template<class T, class... ConstructionArgs>
    const T& CreatorExternal<T, ConstructionArgs...>::Get() const
    {
        return *object;
    }
}

#endif
