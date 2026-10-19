#include "infra/util/CreatorShared.hpp"
#include "gmock/gmock.h"
#include <optional>
#include <type_traits>

class CreatorSharedTest
    : public testing::Test
{
public:
    struct Resource
    {
        explicit Resource(CreatorSharedTest& test)
            : test(test)
        {
            test.constructed.Call();
        }

        ~Resource()
        {
            test.destructed.Call();
        }

        CreatorSharedTest& test;
    };

    testing::StrictMock<testing::MockFunction<void()>> constructed;
    testing::StrictMock<testing::MockFunction<void()>> destructed;
    infra::CreatorShared<Resource, Resource, void()> creator{ [this](std::optional<Resource>& resource)
        {
            resource.emplace(*this);
        } };
    std::optional<infra::ProxyCreator<Resource, void()>> firstUser;
    std::optional<infra::ProxyCreator<Resource, void()>> secondUser;
};

static_assert(!std::is_constructible_v<infra::DelayedProxyCreator<CreatorSharedTest::Resource, void()>, infra::CreatorShared<CreatorSharedTest::Resource, CreatorSharedTest::Resource, void()>&>);
static_assert(std::is_constructible_v<infra::DelayedProxyCreator<CreatorSharedTest::Resource, void()>, infra::Creator<CreatorSharedTest::Resource, CreatorSharedTest::Resource, void()>&>);

TEST_F(CreatorSharedTest, first_user_constructs_the_resource)
{
    EXPECT_CALL(constructed, Call());

    firstUser.emplace(creator);

    EXPECT_CALL(destructed, Call());
}

TEST_F(CreatorSharedTest, second_user_shares_the_resource_of_the_first_user)
{
    EXPECT_CALL(constructed, Call());
    firstUser.emplace(creator);

    secondUser.emplace(creator);

    EXPECT_THAT(&**secondUser, testing::Eq(&**firstUser));
    EXPECT_CALL(destructed, Call());
}

TEST_F(CreatorSharedTest, resource_survives_while_another_user_remains)
{
    EXPECT_CALL(constructed, Call());
    firstUser.emplace(creator);
    secondUser.emplace(creator);

    firstUser.reset();

    EXPECT_CALL(destructed, Call());
}

TEST_F(CreatorSharedTest, last_user_destroys_the_resource)
{
    EXPECT_CALL(constructed, Call());
    firstUser.emplace(creator);
    secondUser.emplace(creator);
    firstUser.reset();

    EXPECT_CALL(destructed, Call());
    secondUser.reset();
}

TEST_F(CreatorSharedTest, a_later_user_constructs_the_resource_again)
{
    EXPECT_CALL(constructed, Call());
    firstUser.emplace(creator);
    EXPECT_CALL(destructed, Call());
    firstUser.reset();

    EXPECT_CALL(constructed, Call());
    secondUser.emplace(creator);

    EXPECT_CALL(destructed, Call());
}
