#include "infra/event/ClaimableResource.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include <gtest/gtest.h>

class TestClaimer
    : public infra::ClaimableResource::Claimer
{
public:
    using infra::ClaimableResource::Claimer::Claimer;

    uint32_t claimsGranted = 0;

    void GrantedClaim()
    {
        EXPECT_TRUE(IsClaimed());
        ++claimsGranted;
    }
};

class TestClaimableResource
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    TestClaimableResource()
        : claimerA(resource)
        , claimerB(resource)
        , claimerC(resource)
    {}

    infra::ClaimableResource resource;
    TestClaimer claimerA;
    TestClaimer claimerB;
    TestClaimer claimerC;
};

TEST_F(TestClaimableResource, ClaimIsGranted)
{
    claimerA.Claim([this]() { claimerA.GrantedClaim(); });
    ExecuteAllActions();
    EXPECT_EQ(1, claimerA.claimsGranted);
}

TEST_F(TestClaimableResource, ClaimWhileAlreadyClaimedIsGrantedAfterRelease)
{
    claimerA.Claim([this]() { claimerA.GrantedClaim(); });
    ExecuteAllActions();
    EXPECT_EQ(1, claimerA.claimsGranted);
    claimerA.Claim([this]() { claimerA.GrantedClaim(); });
    ExecuteAllActions();
    EXPECT_EQ(1, claimerA.claimsGranted);
    claimerA.Release();
    ExecuteAllActions();
    EXPECT_EQ(2, claimerA.claimsGranted);
}

TEST_F(TestClaimableResource, IfNoReleaseIsDoneSecondClaimIsNotGranted)
{
    claimerA.Claim([this]() { claimerA.GrantedClaim(); });
    ExecuteAllActions();
    ASSERT_EQ(1, claimerA.claimsGranted);
    claimerA.Claim([this]() { claimerA.GrantedClaim(); });
    ExecuteAllActions();
    ASSERT_EQ(1, claimerA.claimsGranted);
}

TEST_F(TestClaimableResource, ReleaseBeforeClaimGranted)
{
    claimerB.Claim([this]() { claimerB.GrantedClaim(); });
    claimerA.Claim([this]() { claimerA.GrantedClaim(); });
    ExecuteAllActions();
    EXPECT_EQ(0, claimerA.claimsGranted);
    EXPECT_EQ(1, claimerB.claimsGranted);
    claimerA.Release();
    claimerB.Release();
    ExecuteAllActions();
    EXPECT_EQ(0, claimerA.claimsGranted);
    EXPECT_EQ(1, claimerB.claimsGranted);
}

TEST_F(TestClaimableResource, AtDestructorTheClaimIsReleased)
{
    {
        TestClaimer localClaimer(resource);
        localClaimer.Claim([&localClaimer]() { localClaimer.GrantedClaim(); });
        claimerA.Claim([this]() { claimerA.GrantedClaim(); });
        ExecuteAllActions();
        EXPECT_EQ(0, claimerA.claimsGranted);
    }
    ExecuteAllActions();
    EXPECT_EQ(1, claimerA.claimsGranted);
}

TEST_F(TestClaimableResource, ClaimIsGrantedWhileProcessingReleaseEvent)
{
    claimerA.Claim([this]() { claimerA.GrantedClaim(); });
    ExecuteAllActions();
    claimerB.Claim([this]() { claimerB.GrantedClaim(); });
    claimerA.Release();
    claimerC.Claim([this]() { claimerC.GrantedClaim(); });
    EXPECT_EQ(0, claimerB.claimsGranted);
    ExecuteAllActions();
    EXPECT_EQ(1, claimerB.claimsGranted);
}

TEST_F(TestClaimableResource, TwoConsecutiveReleasesBeforeReleaseOfClaimIsProcessedResultsInNoAdditionalClaimsGranted)
{
    claimerA.Claim([this]() { claimerA.GrantedClaim(); });
    claimerB.Claim([this]() { claimerB.GrantedClaim(); });
    claimerC.Claim([this]() { claimerC.GrantedClaim(); });
    ExecuteAllActions();
    ASSERT_EQ(1, claimerA.claimsGranted);

    claimerA.Release();
    claimerB.Release();
    claimerC.Release();
    ExecuteAllActions();
    EXPECT_EQ(1, claimerA.claimsGranted);
    EXPECT_EQ(0, claimerB.claimsGranted);
    EXPECT_EQ(0, claimerC.claimsGranted);
}

TEST_F(TestClaimableResource, MoveClaimerBeforeGranted)
{
    infra::Optional<TestClaimer> oldClaimer(infra::inPlace, resource);
    infra::MockCallback<void()> granted;
    oldClaimer->Claim([this, &granted]() { granted.callback(); });
    TestClaimer newClaimer(std::move(*oldClaimer));
    oldClaimer = infra::none;

    EXPECT_CALL(granted, callback());
    ExecuteAllActions();
}

TEST_F(TestClaimableResource, MoveClaimerAfterGranted)
{
    infra::Optional<TestClaimer> oldClaimer(infra::inPlace, resource);
    oldClaimer->Claim([]() {});
    ExecuteAllActions();

    TestClaimer newClaimer(std::move(*oldClaimer));
    oldClaimer = infra::none;

    newClaimer.Release();
}

TEST_F(TestClaimableResource, MoveClaimerAfterGrantedAndClaimedAgain)
{
    infra::Optional<TestClaimer> oldClaimer(infra::inPlace, resource);
    oldClaimer->Claim([]() {});
    ExecuteAllActions();
    oldClaimer->Claim([]() {});

    TestClaimer newClaimer(std::move(*oldClaimer));
    oldClaimer = infra::none;

    newClaimer.Release();
    newClaimer.Release();
}

TEST_F(TestClaimableResource, CopyMoveClaimerBeforeGranted)
{
    infra::Optional<TestClaimer> oldClaimer(infra::inPlace, resource);
    infra::MockCallback<void()> granted;
    oldClaimer->Claim([this, &granted]() { granted.callback(); });
    TestClaimer newClaimer(resource);
    newClaimer = std::move(*oldClaimer);
    oldClaimer = infra::none;

    EXPECT_CALL(granted, callback());
    ExecuteAllActions();
}

TEST_F(TestClaimableResource, CopyMoveClaimerAfterGranted)
{
    infra::Optional<TestClaimer> oldClaimer(infra::inPlace, resource);
    oldClaimer->Claim([]() {});
    ExecuteAllActions();

    TestClaimer newClaimer(resource);
    newClaimer = std::move(*oldClaimer);
    oldClaimer = infra::none;

    newClaimer.Release();
}
