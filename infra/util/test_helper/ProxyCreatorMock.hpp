#ifndef INFRA_PROXY_CREATOR_MOCK_HPP
#define INFRA_PROXY_CREATOR_MOCK_HPP

#include "infra/util/ProxyCreator.hpp"
#include "gmock/gmock.h"

namespace infra
{
    template<class T, class ConstructionArgs = void>
    class CreatorMock;

    template<class T, class... ConstructionArgs>
    class CreatorMock<CreatorBase<T, void(ConstructionArgs...)>&>
        : public CreatorMock<T, void(ConstructionArgs...)>
    {
    public:
        using CreatorMock<T, void(ConstructionArgs...)>::CreatorMock;
    };

    template<class T>
    class CreatorMock<T, void()>
        : public CreatorBase<T, void()>
    {
    public:
        CreatorMock(T& createdObject)
            : createdObject(createdObject)
        {}

        MOCK_METHOD0(Constructed, void());
        MOCK_METHOD0(Destructed, void());

    protected:
        virtual T& Get() override
        {
            return createdObject;
        }

        virtual const T& Get() const override
        {
            return createdObject;
        }

        virtual void Emplace() override
        {
            Constructed();
        }

        virtual void Destroy() override
        {
            Destructed();
        }

    private:
        T& createdObject;
    };

    template<class T, class ConstructionArg0>
    class CreatorMock<T, void(ConstructionArg0)>
        : public CreatorBase<T, void(ConstructionArg0)>
    {
    public:
        CreatorMock(T& createdObject)
            : createdObject(createdObject)
        {}

        MOCK_METHOD1_T(Constructed, void(ConstructionArg0));
        MOCK_METHOD0(Destructed, void());

    protected:
        virtual T& Get() override
        {
            return createdObject;
        }

        virtual const T& Get() const override
        {
            return createdObject;
        }

        virtual void Emplace(ConstructionArg0 arg0) override
        {
            Constructed(arg0);
        }

        virtual void Destroy() override
        {
            Destructed();
        }

    private:
        T& createdObject;
    };

    template<class T, class ConstructionArg0, class ConstructionArg1>
    class CreatorMock<T, void(ConstructionArg0, ConstructionArg1)>
        : public CreatorBase<T, void(ConstructionArg0, ConstructionArg1)>
    {
    public:
        CreatorMock(T& createdObject)
            : createdObject(createdObject)
        {}

        MOCK_METHOD2_T(Constructed, void(ConstructionArg0, ConstructionArg1));
        MOCK_METHOD0(Destructed, void());

    protected:
        virtual T& Get() override
        {
            return createdObject;
        }

        virtual const T& Get() const override
        {
            return createdObject;
        }

        virtual void Emplace(ConstructionArg0 arg0, ConstructionArg1 arg1) override
        {
            Constructed(arg0, arg1);
        }

        virtual void Destroy() override
        {
            Destructed();
        }

    private:
        T& createdObject;
    };

    template<class T, class ConstructionArg0, class ConstructionArg1, class ConstructionArg2>
    class CreatorMock<T, void(ConstructionArg0, ConstructionArg1, ConstructionArg2)>
        : public CreatorBase<T, void(ConstructionArg0, ConstructionArg1, ConstructionArg2)>
    {
    public:
        CreatorMock(T& createdObject)
            : createdObject(createdObject)
        {}

        MOCK_METHOD3_T(Constructed, void(ConstructionArg0, ConstructionArg1, ConstructionArg2));
        MOCK_METHOD0(Destructed, void());

    protected:
        virtual T& Get() override
        {
            return createdObject;
        }

        virtual const T& Get() const override
        {
            return createdObject;
        }

        virtual void Emplace(ConstructionArg0 arg0, ConstructionArg1 arg1, ConstructionArg2 arg2) override
        {
            Constructed(arg0, arg1, arg2);
        }

        virtual void Destroy() override
        {
            Destructed();
        }

    private:
        T& createdObject;
    };

    template<class T, class ConstructionArg0, class ConstructionArg1, class ConstructionArg2, class ConstructionArg3>
    class CreatorMock<T, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3)>
        : public CreatorBase<T, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3)>
    {
    public:
        CreatorMock(T& createdObject)
            : createdObject(createdObject)
        {}

        MOCK_METHOD4_T(Constructed, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3));
        MOCK_METHOD0(Destructed, void());

    protected:
        virtual T& Get() override
        {
            return createdObject;
        }

        virtual const T& Get() const override
        {
            return createdObject;
        }

        virtual void Emplace(ConstructionArg0 arg0, ConstructionArg1 arg1, ConstructionArg2 arg2, ConstructionArg3 arg3) override
        {
            Constructed(arg0, arg1, arg2, arg3);
        }

        virtual void Destroy() override
        {
            Destructed();
        }

    private:
        T& createdObject;
    };

    template<class T, class ConstructionArg0, class ConstructionArg1, class ConstructionArg2, class ConstructionArg3, class ConstructionArg4>
    class CreatorMock<T, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3, ConstructionArg4)>
        : public CreatorBase<T, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3, ConstructionArg4)>
    {
    public:
        CreatorMock(T& createdObject)
            : createdObject(createdObject)
        {}

        MOCK_METHOD5_T(Constructed, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3, ConstructionArg4));
        MOCK_METHOD0(Destructed, void());

    protected:
        virtual T& Get() override
        {
            return createdObject;
        }

        virtual const T& Get() const override
        {
            return createdObject;
        }

        virtual void Emplace(ConstructionArg0 arg0, ConstructionArg1 arg1, ConstructionArg2 arg2, ConstructionArg3 arg3, ConstructionArg4 arg4) override
        {
            Constructed(arg0, arg1, arg2, arg3, arg4);
        }

        virtual void Destroy() override
        {
            Destructed();
        }

    private:
        T& createdObject;
    };

    template<class T, class ConstructionArg0, class ConstructionArg1, class ConstructionArg2, class ConstructionArg3, class ConstructionArg4, class ConstructionArg5>
    class CreatorMock<T, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3, ConstructionArg4, ConstructionArg5)>
        : public CreatorBase<T, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3, ConstructionArg4, ConstructionArg5)>
    {
    public:
        CreatorMock(T& createdObject)
            : createdObject(createdObject)
        {}

        MOCK_METHOD6_T(Constructed, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3, ConstructionArg4, ConstructionArg5));
        MOCK_METHOD0(Destructed, void());

    protected:
        virtual T& Get() override
        {
            return createdObject;
        }

        virtual const T& Get() const override
        {
            return createdObject;
        }

        virtual void Emplace(ConstructionArg0 arg0, ConstructionArg1 arg1, ConstructionArg2 arg2, ConstructionArg3 arg3, ConstructionArg4 arg4, ConstructionArg5 arg5) override
        {
            Constructed(arg0, arg1, arg2, arg3, arg4, arg5);
        }

        virtual void Destroy() override
        {
            Destructed();
        }

    private:
        T& createdObject;
    };

    template<>
    class CreatorMock<void, void()>
        : public CreatorBase<void, void()>
    {
    public:
        MOCK_METHOD0(Constructed, void());
        MOCK_METHOD0(Destructed, void());

    protected:
        virtual void Emplace() override
        {
            Constructed();
        }

        virtual void Destroy() override
        {
            Destructed();
        }
    };

    template<class ConstructionArg0>
    class CreatorMock<void, void(ConstructionArg0)>
        : public CreatorBase<void, void(ConstructionArg0)>
    {
    public:
        MOCK_METHOD1_T(Constructed, void(ConstructionArg0));
        MOCK_METHOD0(Destructed, void());

    protected:
        virtual void Emplace(ConstructionArg0 arg0) override
        {
            Constructed(arg0);
        }

        virtual void Destroy() override
        {
            Destructed();
        }
    };

    template<class ConstructionArg0, class ConstructionArg1>
    class CreatorMock<void, void(ConstructionArg0, ConstructionArg1)>
        : public CreatorBase<void, void(ConstructionArg0, ConstructionArg1)>
    {
    public:
        MOCK_METHOD2_T(Constructed, void(ConstructionArg0, ConstructionArg1));
        MOCK_METHOD0(Destructed, void());

    protected:
        virtual void Emplace(ConstructionArg0 arg0, ConstructionArg1 arg1) override
        {
            Constructed(arg0, arg1);
        }

        virtual void Destroy() override
        {
            Destructed();
        }
    };

    template<class ConstructionArg0, class ConstructionArg1, class ConstructionArg2>
    class CreatorMock<void, void(ConstructionArg0, ConstructionArg1, ConstructionArg2)>
        : public CreatorBase<void, void(ConstructionArg0, ConstructionArg1, ConstructionArg2)>
    {
    public:
        MOCK_METHOD3_T(Constructed, void(ConstructionArg0, ConstructionArg1, ConstructionArg2));
        MOCK_METHOD0(Destructed, void());

    protected:
        virtual void Emplace(ConstructionArg0 arg0, ConstructionArg1 arg1, ConstructionArg2 arg2) override
        {
            Constructed(arg0, arg1, arg2);
        }

        virtual void Destroy() override
        {
            Destructed();
        }
    };

    template<class ConstructionArg0, class ConstructionArg1, class ConstructionArg2, class ConstructionArg3>
    class CreatorMock<void, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3)>
        : public CreatorBase<void, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3)>
    {
    public:
        MOCK_METHOD4_T(Constructed, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3));
        MOCK_METHOD0(Destructed, void());

    protected:
        virtual void Emplace(ConstructionArg0 arg0, ConstructionArg1 arg1, ConstructionArg2 arg2, ConstructionArg3 arg3) override
        {
            Constructed(arg0, arg1, arg2, arg3);
        }

        virtual void Destroy() override
        {
            Destructed();
        }
    };

    template<class ConstructionArg0, class ConstructionArg1, class ConstructionArg2, class ConstructionArg3, class ConstructionArg4>
    class CreatorMock<void, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3, ConstructionArg4)>
        : public CreatorBase<void, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3, ConstructionArg4)>
    {
    public:
        MOCK_METHOD5_T(Constructed, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3, ConstructionArg4));
        MOCK_METHOD0(Destructed, void());

    protected:
        virtual void Emplace(ConstructionArg0 arg0, ConstructionArg1 arg1, ConstructionArg2 arg2, ConstructionArg3 arg3, ConstructionArg4 arg4) override
        {
            Constructed(arg0, arg1, arg2, arg3, arg4);
        }

        virtual void Destroy() override
        {
            Destructed();
        }
    };

    template<class ConstructionArg0, class ConstructionArg1, class ConstructionArg2, class ConstructionArg3, class ConstructionArg4, class ConstructionArg5>
    class CreatorMock<void, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3, ConstructionArg4, ConstructionArg5)>
        : public CreatorBase<void, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3, ConstructionArg4, ConstructionArg5)>
    {
    public:
        MOCK_METHOD5_T(Constructed, void(ConstructionArg0, ConstructionArg1, ConstructionArg2, ConstructionArg3, ConstructionArg4, ConstructionArg5));
        MOCK_METHOD0(Destructed, void());

    protected:
        virtual void Emplace(ConstructionArg0 arg0, ConstructionArg1 arg1, ConstructionArg2 arg2, ConstructionArg3 arg3, ConstructionArg4 arg4, ConstructionArg5 arg5) override
        {
            Constructed(arg0, arg1, arg2, arg3, arg4, arg5);
        }

        virtual void Destroy() override
        {
            Destructed();
        }
    };
}

#endif
