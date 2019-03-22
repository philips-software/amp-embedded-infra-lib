#include "gmock/gmock.h"
#include "infra/util/ProxyCreator.hpp"
#include "infra/util/test_helper/MockCallback.hpp"

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
    infra::Creator<PeripheralInterface, Peripheral> creator;

    infra::ProxyCreator<PeripheralInterface> creatorProxy(creator);
}

TEST(ProxyCreatorTest, CreatePeripheralWithParameterGivenByProxy)
{
    infra::Creator<PeripheralInterface, PeripheralWithTwoParameters, int, int> creator;

    infra::ProxyCreator<PeripheralInterface, int, int> creatorProxy(creator, 5, 6);
    EXPECT_EQ(5, creator->x);
}

TEST(ProxyCreatorTest, CreatePeripheralWithParameterGivenByCreator)
{
    infra::Creator<PeripheralInterface, PeripheralWithTwoParameters> creator([](infra::Optional<PeripheralWithTwoParameters>& object) { object.Emplace(5, 6); });

    infra::ProxyCreator<PeripheralInterface> creatorProxy(creator);
    EXPECT_EQ(5, creator->x);
}

TEST(ProxyCreatorTest, CreatePeripheralWithParameterGivenByProxyAndCreator)
{
    infra::Creator<PeripheralInterface, PeripheralWithTwoParameters, int> creator([](infra::Optional<PeripheralWithTwoParameters>& object, int x) { object.Emplace(x, 6); });

    infra::ProxyCreator<PeripheralInterface, int> creatorProxy(creator, 5);
    EXPECT_EQ(5, creator->x);
    EXPECT_EQ(6, creator->y);
}

TEST(ProxyCreatorTest, CreatePeripheralWithParameterGivenByCreatorAndProxy)
{
    infra::Creator<PeripheralInterface, PeripheralWithTwoParameters, int> creator([](infra::Optional<PeripheralWithTwoParameters>& object, int y) { object.Emplace(5, y); });

    infra::ProxyCreator<PeripheralInterface, int> creatorProxy(creator, 6);
    EXPECT_EQ(5, creator->x);
    EXPECT_EQ(6, creator->y);
}

TEST(ProxyCreatorTest, AccessPeripheral)
{
    infra::Creator<PeripheralInterface, Peripheral> creator;

    infra::ProxyCreator<PeripheralInterface> creatorProxy(creator);

    EXPECT_CALL(*creator, SendMock());
    creatorProxy->Send();
}

TEST(ProxyCreatorTest, CreatePeripheralWithVoidAccess)
{
    infra::Creator<void, PeripheralWithTwoParameters, int, int> creator;

    infra::ProxyCreator<void, int, int> creatorProxy(creator, 5, 6);
    EXPECT_EQ(5, creator->x);
}

TEST(ProxyCreatorTest, CreatePeripheralByCreatorExternal)
{
    infra::MockCallback<void()> construction;
    infra::MockCallback<void()> destruction;

    PeripheralWithTwoParameters peripheral(5, 6);
    infra::CreatorExternal<PeripheralInterface> creator([&peripheral, &construction]() -> PeripheralWithTwoParameters& { construction.callback(); return peripheral; }, [&destruction]() { destruction.callback(); });

    EXPECT_CALL(construction, callback());
    infra::ProxyCreator<PeripheralInterface> creatorProxy(creator);

    EXPECT_CALL(peripheral, SendMock());
    creatorProxy->Send();

    EXPECT_CALL(destruction, callback());
}

TEST(DelayedProxyCreatorTest, CreatePeripheral)
{
    infra::Creator<PeripheralInterface, Peripheral> creator;

    infra::DelayedProxyCreator<PeripheralInterface> creatorProxy(creator);
    creatorProxy.Emplace();
    creatorProxy.Destroy();
}

TEST(DelayedProxyCreatorTest, CreatePeripheralWithParameterGivenByProxy)
{
    infra::Creator<PeripheralInterface, PeripheralWithTwoParameters, int, int> creator;

    infra::DelayedProxyCreator<PeripheralInterface, int, int> creatorProxy(creator);
    creatorProxy.Emplace(5, 6);
    EXPECT_EQ(5, creator->x);
}

TEST(DelayedProxyCreatorTest, CreatePeripheralWithParameterGivenByCreator)
{
    infra::Creator<PeripheralInterface, PeripheralWithTwoParameters> creator([](infra::Optional<PeripheralWithTwoParameters>& object) { object.Emplace(5, 6); });

    infra::DelayedProxyCreator<PeripheralInterface> creatorProxy(creator);
    creatorProxy.Emplace();
    EXPECT_EQ(5, creator->x);
}

TEST(DelayedProxyCreatorTest, CreatePeripheralWithParameterGivenByProxyAndCreator)
{
    infra::Creator<PeripheralInterface, PeripheralWithTwoParameters, int> creator([](infra::Optional<PeripheralWithTwoParameters>& object, int x) { object.Emplace(x, 6); });

    infra::DelayedProxyCreator<PeripheralInterface, int> creatorProxy(creator);
    creatorProxy.Emplace(5);
    EXPECT_EQ(5, creator->x);
    EXPECT_EQ(6, creator->y);
}

TEST(DelayedProxyCreatorTest, CreatePeripheralWithParameterGivenByCreatorAndProxy)
{
    infra::Creator<PeripheralInterface, PeripheralWithTwoParameters, int> creator([](infra::Optional<PeripheralWithTwoParameters>& object, int y) { object.Emplace(5, y); });

    infra::DelayedProxyCreator<PeripheralInterface, int> creatorProxy(creator);
    creatorProxy.Emplace(6);
    EXPECT_EQ(5, creator->x);
    EXPECT_EQ(6, creator->y);
}

TEST(DelayedProxyCreatorTest, AccessPeripheral)
{
    infra::Creator<PeripheralInterface, Peripheral> creator;

    infra::DelayedProxyCreator<PeripheralInterface> creatorProxy(creator);
    creatorProxy.Emplace();

    EXPECT_CALL(*creator, SendMock());
    creatorProxy->Send();
}
