#include "infra/util/ProxyCreatorShared.hpp"
#include "gmock/gmock.h"
#include <optional>
#include <type_traits>

class ProxyCreatorSharedTest
    : public testing::Test
{
public:
    struct Resource
    {
        explicit Resource(ProxyCreatorSharedTest& test)
            : test(test)
        {
            test.constructed.Call();
        }

        ~Resource()
        {
            test.destructed.Call();
        }

        ProxyCreatorSharedTest& test;
    };

    testing::StrictMock<testing::MockFunction<void()>> constructed;
    testing::StrictMock<testing::MockFunction<void()>> destructed;
    infra::ProxyCreatorShared<Resource, Resource, void()> creator{ [this](std::optional<Resource>& resource)
        {
            resource.emplace(*this);
        } };
    std::optional<infra::ProxyCreator<Resource, void()>> firstUser;
    std::optional<infra::ProxyCreator<Resource, void()>> secondUser;
};

static_assert(!std::is_constructible_v<infra::DelayedProxyCreator<ProxyCreatorSharedTest::Resource, void()>, infra::ProxyCreatorShared<ProxyCreatorSharedTest::Resource, ProxyCreatorSharedTest::Resource, void()>&>);
static_assert(std::is_constructible_v<infra::DelayedProxyCreator<ProxyCreatorSharedTest::Resource, void()>, infra::Creator<ProxyCreatorSharedTest::Resource, ProxyCreatorSharedTest::Resource, void()>&>);

TEST_F(ProxyCreatorSharedTest, first_user_constructs_the_resource)
{
    EXPECT_CALL(constructed, Call());

    firstUser.emplace(creator);

    EXPECT_CALL(destructed, Call());
}

TEST_F(ProxyCreatorSharedTest, second_user_shares_the_resource_of_the_first_user)
{
    EXPECT_CALL(constructed, Call());
    firstUser.emplace(creator);

    secondUser.emplace(creator);

    EXPECT_THAT(&**secondUser, testing::Eq(&**firstUser));
    EXPECT_CALL(destructed, Call());
}

TEST_F(ProxyCreatorSharedTest, resource_survives_while_another_user_remains)
{
    EXPECT_CALL(constructed, Call());
    firstUser.emplace(creator);
    secondUser.emplace(creator);

    firstUser.reset();

    EXPECT_CALL(destructed, Call());
}

TEST_F(ProxyCreatorSharedTest, last_user_destroys_the_resource)
{
    EXPECT_CALL(constructed, Call());
    firstUser.emplace(creator);
    secondUser.emplace(creator);
    firstUser.reset();

    EXPECT_CALL(destructed, Call());
    secondUser.reset();
}

TEST_F(ProxyCreatorSharedTest, a_later_user_constructs_the_resource_again)
{
    EXPECT_CALL(constructed, Call());
    firstUser.emplace(creator);
    EXPECT_CALL(destructed, Call());
    firstUser.reset();

    EXPECT_CALL(constructed, Call());
    secondUser.emplace(creator);

    EXPECT_CALL(destructed, Call());
}
