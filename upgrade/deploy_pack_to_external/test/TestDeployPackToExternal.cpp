#include "gmock/gmock.h"
#include "hal/interfaces/test_doubles/GpioStub.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/util/Optional.hpp"
#include "hal/interfaces/test_doubles/FlashStub.hpp"
#include "upgrade/deploy_pack_to_external/DeployPackToExternal.hpp"
#include "upgrade/pack/UpgradePackHeader.hpp"

namespace
{
    class DeployPackToExternalObserverMock
        : public application::DeployPackToExternalObserver
    {
    public:
        DeployPackToExternalObserverMock(application::DeployPackToExternal& deploy)
            : application::DeployPackToExternalObserver(deploy)
        {}

        //using application::DeployPackToExternalObserver::DeployPackToExternalObserver;

        MOCK_METHOD0(NotDeployable, void());
        MOCK_METHOD0(DoesntFit, void());
        MOCK_METHOD0(Done, void());
    };
}

class DeployPackToExternalTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    DeployPackToExternalTest()
        : from(1, 4096)
        , to(1, 4096)
    {}

    hal::FlashStub from;
    hal::FlashStub to;
};

TEST_F(DeployPackToExternalTest, DeployPackCopiesHeader)
{
    infra::ByteOutputStream outputStream(from.sectors[0]);
    application::UpgradePackHeaderPrologue header = {};
    header.status = application::UpgradePackStatus::readyToDeploy;
    outputStream << header;

    application::DeployPackToExternal deploy(from, to);
    DeployPackToExternalObserverMock observer(deploy);
    EXPECT_CALL(observer, Done());
    ExecuteAllActions();

    infra::ByteInputStream inputStream(to.sectors[0]);
    application::UpgradePackHeaderPrologue writtenHeader = {};
    inputStream >> infra::MakeByteRange(writtenHeader);

    EXPECT_TRUE(infra::ContentsEqual(infra::MakeByteRange(header), infra::MakeByteRange(writtenHeader)));
}

TEST_F(DeployPackToExternalTest, AfterCopyDeployStatusIsSet)
{
    infra::ByteOutputStream outputStream(from.sectors[0]);
    application::UpgradePackHeaderPrologue header = {};
    header.status = application::UpgradePackStatus::readyToDeploy;
    outputStream << header;

    application::DeployPackToExternal deploy(from, to);
    DeployPackToExternalObserverMock observer(deploy);
    EXPECT_CALL(observer, Done());
    ExecuteAllActions();

    infra::ByteInputStream inputStream(to.sectors[0]);
    application::UpgradePackHeaderPrologue writtenHeader = {};
    inputStream >> infra::MakeByteRange(writtenHeader);

    header.status = application::UpgradePackStatus::readyToDeploy;
    EXPECT_TRUE(infra::ContentsEqual(infra::MakeByteRange(header), infra::MakeByteRange(writtenHeader)));
}

TEST_F(DeployPackToExternalTest, DeployPackCopiesChunk)
{
    infra::ByteOutputStream outputStream(from.sectors[0]);
    application::UpgradePackHeaderPrologue header = {};
    header.status = application::UpgradePackStatus::readyToDeploy;
    std::array<uint8_t, 5> contents = { 1, 2, 3, 4, 5 };
    header.signedContentsLength = contents.size();
    outputStream << header << contents;

    application::DeployPackToExternal deploy(from, to);
    DeployPackToExternalObserverMock observer(deploy);
    EXPECT_CALL(observer, Done());
    ExecuteAllActions();

    infra::ByteInputStream inputStream(to.sectors[0]);
    application::UpgradePackHeaderPrologue writtenHeader = {};
    std::array<uint8_t, 5> writtenContents;
    inputStream >> infra::MakeByteRange(writtenHeader) >> writtenContents;

    EXPECT_EQ(contents, writtenContents);
}

TEST_F(DeployPackToExternalTest, DeployPackCopiesMultipleChunks)
{
    infra::ByteOutputStream outputStream(from.sectors[0]);
    application::UpgradePackHeaderPrologue header = {};
    header.status = application::UpgradePackStatus::readyToDeploy;
    std::array<uint8_t, 1024> largeContents;
    std::array<uint8_t, 5> contents = { 1, 2, 3, 4, 5 };
    header.signedContentsLength = contents.size() + largeContents.size();
    outputStream << header << largeContents << contents;

    application::DeployPackToExternal deploy(from, to);
    DeployPackToExternalObserverMock observer(deploy);
    EXPECT_CALL(observer, Done());
    ExecuteAllActions();

    infra::ByteInputStream inputStream(to.sectors[0]);
    application::UpgradePackHeaderPrologue writtenHeader = {};
    std::array<uint8_t, 5> writtenContents;
    inputStream >> infra::MakeByteRange(writtenHeader) >> largeContents >> writtenContents;

    EXPECT_EQ(contents, writtenContents);
}

TEST_F(DeployPackToExternalTest, AfterDeployDeployedStatusIsSet)
{
    infra::ByteOutputStream outputStream(from.sectors[0]);
    application::UpgradePackHeaderPrologue header = {};
    header.status = application::UpgradePackStatus::readyToDeploy;
    outputStream << header;

    application::DeployPackToExternal deploy(from, to);
    DeployPackToExternalObserverMock observer(deploy);
    EXPECT_CALL(observer, Done());
    ExecuteAllActions();

    infra::ByteInputStream inputStream(from.sectors[0]);
    inputStream >> header;
    EXPECT_EQ(application::UpgradePackStatus::deployed, header.status);
}

TEST_F(DeployPackToExternalTest, AfterDeployStatusLedSignalsSuccess)
{
    infra::ByteOutputStream outputStream(from.sectors[0]);
    application::UpgradePackHeaderPrologue header = {};
    header.status = application::UpgradePackStatus::readyToDeploy;
    outputStream << header;

    application::DeployPackToExternal deploy(from, to);
    DeployPackToExternalObserverMock observer(deploy);
    EXPECT_CALL(observer, Done());
    ExecuteAllActions();
}

TEST_F(DeployPackToExternalTest, TooBigDoesNotDeploy)
{
    infra::ByteOutputStream outputStream(from.sectors[0]);
    application::UpgradePackHeaderPrologue header = {};
    header.status = application::UpgradePackStatus::readyToDeploy;
    header.signedContentsLength = to.SizeOfSector(0) * 5;
    outputStream << header;

    application::DeployPackToExternal deploy(from, to);
    DeployPackToExternalObserverMock observer(deploy);
    EXPECT_CALL(observer, DoesntFit());
    ExecuteAllActions();

    infra::ByteInputStream inputStream(to.sectors[0]);
    application::UpgradePackHeaderPrologue writtenHeader = {};
    inputStream >> infra::MakeByteRange(writtenHeader);

    EXPECT_FALSE(infra::ContentsEqual(infra::MakeByteRange(header), infra::MakeByteRange(writtenHeader)));
}
