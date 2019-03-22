#include "infra/event/ClaimableResource.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include <gtest/gtest.h>

class TestClaimer
    : public infra::ClaimableResource::Claimer
{
public:
    TestClaimer(infra::ClaimableResource& resource)
        : infra::ClaimableResource::Claimer(resource)
        , claimsGranted(0)
    {}

    uint32_t claimsGranted;

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
