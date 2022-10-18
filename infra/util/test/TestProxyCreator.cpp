#include "infra/util/ProxyCreator.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gmock/gmock.h"

class PeripheralInterface
{
public:
    virtual void Send() = 0;
};

class Peripheral
    : public PeripheralInterface
{
public:
    void Send()
    {
        SendMock();
    }

    MOCK_METHOD0(SendMock, void());
};

class PeripheralWithTwoParameters
    : public PeripheralInterface
{
public:
    PeripheralWithTwoParameters(int x, int y)
        : x(x)
        , y(y)
    {}

    void Send()
    {
        SendMock();
    }

    MOCK_METHOD0(SendMock, void());
    int x;
    int y;
};

TEST(ProxyCreatorTest, CreatePeripheral)
{
    infra::Creator<PeripheralInterface, Peripheral, void()> creator;

    infra::ProxyCreator<PeripheralInterface, void()> creatorProxy(creator);
}

TEST(ProxyCreatorTest, CreatePeripheralWithParameterGivenByProxy)
{
    infra::Creator<PeripheralInterface, PeripheralWithTwoParameters, void(int, int)> creator;

    infra::ProxyCreator<PeripheralInterface, void(int, int)> creatorProxy(creator, 5, 6);
    EXPECT_EQ(5, creator->x);
}

TEST(ProxyCreatorTest, CreatePeripheralWithParameterGivenByCreator)
{
    infra::Creator<PeripheralInterface, PeripheralWithTwoParameters, void()> creator([](infra::Optional<PeripheralWithTwoParameters>& object)
        { object.Emplace(5, 6); });

    infra::ProxyCreator<PeripheralInterface, void()> creatorProxy(creator);
    EXPECT_EQ(5, creator->x);
}

TEST(ProxyCreatorTest, CreatePeripheralWithParameterGivenByProxyAndCreator)
{
    infra::Creator<PeripheralInterface, PeripheralWithTwoParameters, void(int)> creator([](infra::Optional<PeripheralWithTwoParameters>& object, int x)
        { object.Emplace(x, 6); });

    infra::ProxyCreator<PeripheralInterface, void(int)> creatorProxy(creator, 5);
    EXPECT_EQ(5, creator->x);
    EXPECT_EQ(6, creator->y);
}

TEST(ProxyCreatorTest, CreatePeripheralWithParameterGivenByCreatorAndProxy)
{
    infra::Creator<PeripheralInterface, PeripheralWithTwoParameters, void(int)> creator([](infra::Optional<PeripheralWithTwoParameters>& object, int y)
        { object.Emplace(5, y); });

    infra::ProxyCreator<PeripheralInterface, void(int)> creatorProxy(creator, 6);
    EXPECT_EQ(5, creator->x);
    EXPECT_EQ(6, creator->y);
}

TEST(ProxyCreatorTest, AccessPeripheral)
{
    infra::Creator<PeripheralInterface, Peripheral, void()> creator;

    infra::ProxyCreator<PeripheralInterface, void()> creatorProxy(creator);

    EXPECT_CALL(*creator, SendMock());
    creatorProxy->Send();
}

TEST(ProxyCreatorTest, CreatePeripheralWithVoidAccess)
{
    infra::Creator<void, PeripheralWithTwoParameters, void(int, int)> creator;

    infra::ProxyCreator<void, void(int, int)> creatorProxy(creator, 5, 6);
    EXPECT_EQ(5, creator->x);
}

TEST(ProxyCreatorTest, CreatePeripheralByCreatorExternal)
{
    infra::MockCallback<void()> construction;
    infra::MockCallback<void()> destruction;

    PeripheralWithTwoParameters peripheral(5, 6);
    infra::CreatorExternal<PeripheralInterface, void()> creator([&peripheral, &construction]() -> PeripheralWithTwoParameters&
        { construction.callback(); return peripheral; },
        [&destruction]()
        { destruction.callback(); });

    EXPECT_CALL(construction, callback());
    infra::ProxyCreator<PeripheralInterface, void()> creatorProxy(creator);

    EXPECT_CALL(peripheral, SendMock());
    creatorProxy->Send();

    EXPECT_CALL(destruction, callback());
}

TEST(ProxyCreatorTest, CreateVoidByCreatorExternal)
{
    infra::MockCallback<void()> construction;
    infra::MockCallback<void()> destruction;

    PeripheralWithTwoParameters peripheral(5, 6);
    infra::CreatorExternal<void, void()> creator([&peripheral, &construction]()
        { construction.callback(); },
        [&destruction]()
        { destruction.callback(); });

    EXPECT_CALL(construction, callback());
    infra::ProxyCreator<void, void()> creatorProxy(creator);

    EXPECT_CALL(destruction, callback());
}

TEST(ProxyCreatorTest, CreateTuplePeripheralByCreatorExternal)
{
    infra::MockCallback<void()> construction;
    infra::MockCallback<void()> destruction;

    struct X
    {
        PeripheralWithTwoParameters peripheral1{ 5, 6 };
        PeripheralWithTwoParameters peripheral2{ 7, 8 };
    } x;

    infra::CreatorExternal<std::tuple<PeripheralInterface&, PeripheralInterface&>, void()> creator(
        [&x, &construction]() -> std::tuple<PeripheralInterface&, PeripheralInterface&>
        {
            construction.callback();
            return std::make_tuple(std::ref(x.peripheral1), std::ref(x.peripheral2));
        },
        [&destruction]()
        {
            destruction.callback();
        });

    EXPECT_CALL(construction, callback());
    infra::ProxyCreator<std::tuple<PeripheralInterface&, PeripheralInterface&>, void()> creatorProxy(creator);

    EXPECT_CALL(x.peripheral1, SendMock());
    std::get<0>(*creatorProxy).Send();

    EXPECT_CALL(x.peripheral2, SendMock());
    std::get<1>(*creatorProxy).Send();

    EXPECT_CALL(destruction, callback());
}

TEST(DelayedProxyCreatorTest, CreatePeripheral)
{
    infra::Creator<PeripheralInterface, Peripheral, void()> creator;

    infra::DelayedProxyCreator<PeripheralInterface, void()> creatorProxy(creator);
    creatorProxy.Emplace();
    creatorProxy.Destroy();
}

TEST(DelayedProxyCreatorTest, CreatePeripheralWithParameterGivenByProxy)
{
    infra::Creator<PeripheralInterface, PeripheralWithTwoParameters, void(int, int)> creator;

    infra::DelayedProxyCreator<PeripheralInterface, void(int, int)> creatorProxy(creator);
    creatorProxy.Emplace(5, 6);
    EXPECT_EQ(5, creator->x);
}

TEST(DelayedProxyCreatorTest, CreatePeripheralWithParameterGivenByCreator)
{
    infra::Creator<PeripheralInterface, PeripheralWithTwoParameters, void()> creator([](infra::Optional<PeripheralWithTwoParameters>& object)
        { object.Emplace(5, 6); });

    infra::DelayedProxyCreator<PeripheralInterface, void()> creatorProxy(creator);
    creatorProxy.Emplace();
    EXPECT_EQ(5, creator->x);
}

TEST(DelayedProxyCreatorTest, CreatePeripheralWithParameterGivenByProxyAndCreator)
{
    infra::Creator<PeripheralInterface, PeripheralWithTwoParameters, void(int)> creator([](infra::Optional<PeripheralWithTwoParameters>& object, int x)
        { object.Emplace(x, 6); });

    infra::DelayedProxyCreator<PeripheralInterface, void(int)> creatorProxy(creator);
    creatorProxy.Emplace(5);
    EXPECT_EQ(5, creator->x);
    EXPECT_EQ(6, creator->y);
}

TEST(DelayedProxyCreatorTest, CreatePeripheralWithParameterGivenByCreatorAndProxy)
{
    infra::Creator<PeripheralInterface, PeripheralWithTwoParameters, void(int)> creator([](infra::Optional<PeripheralWithTwoParameters>& object, int y)
        { object.Emplace(5, y); });

    infra::DelayedProxyCreator<PeripheralInterface, void(int)> creatorProxy(creator);
    creatorProxy.Emplace(6);
    EXPECT_EQ(5, creator->x);
    EXPECT_EQ(6, creator->y);
}

TEST(DelayedProxyCreatorTest, AccessPeripheral)
{
    infra::Creator<PeripheralInterface, Peripheral, void()> creator;

    infra::DelayedProxyCreator<PeripheralInterface, void()> creatorProxy(creator);
    creatorProxy.Emplace();

    EXPECT_CALL(*creator, SendMock());
    creatorProxy->Send();
}
