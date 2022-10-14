#include "gmock/gmock.h"
#include "upgrade/boot_loader/DecryptorAesMbedTls.hpp"
#include "upgrade/boot_loader/DecryptorAesTiny.hpp"

class DecryptorAesTest
    : public testing::Test
{
public:
    DecryptorAesTest()
        : key(std::vector<uint8_t>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 })
        , decryptorTiny(key)
        , decryptorMbedTls(key)
    {
        decryptorTiny.Reset();
        decryptorMbedTls.Reset();
    }

    std::vector<uint8_t> key;
    application::DecryptorAesTiny decryptorTiny;
    application::DecryptorAesMbedTls decryptorMbedTls;
};

TEST_F(DecryptorAesTest, DecryptPartSmall)
{
    std::vector<uint8_t> dataTiny{ 1, 2, 3, 4 };
    std::vector<uint8_t> dataMbedTls{ 1, 2, 3, 4 };

    decryptorTiny.DecryptPart(dataTiny);
    decryptorMbedTls.DecryptPart(dataMbedTls);

    EXPECT_EQ((std::vector<uint8_t>{ 0xc7, 0xa3, 0x38, 0x33 }), dataTiny);
    EXPECT_EQ((std::vector<uint8_t>{ 0xc7, 0xa3, 0x38, 0x33 }), dataMbedTls);
}

TEST_F(DecryptorAesTest, DecryptPartLarge)
{
    std::vector<uint8_t> dataTiny{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    std::vector<uint8_t> dataMbedTls{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

    decryptorTiny.DecryptPart(dataTiny);
    decryptorMbedTls.DecryptPart(dataMbedTls);

    EXPECT_EQ((std::vector<uint8_t>{
                  0xc7, 0xa3, 0x38, 0x33, 0x82, 0x89, 0x5c, 0x8a,
                  0x66, 0x45, 0x8a, 0x6e, 0xac, 0xc6, 0xd7, 0x69,
                  0x72, 0x44, 0x10, 0x91, 0x90, 0xc6, 0xb3, 0x16,
                  0x40, 0x71, 0xb6, 0xef, 0x68, 0xfa, 0x22, 0x1a }),
        dataTiny);
    EXPECT_EQ((std::vector<uint8_t>{
                  0xc7, 0xa3, 0x38, 0x33, 0x82, 0x89, 0x5c, 0x8a,
                  0x66, 0x45, 0x8a, 0x6e, 0xac, 0xc6, 0xd7, 0x69,
                  0x72, 0x44, 0x10, 0x91, 0x90, 0xc6, 0xb3, 0x16,
                  0x40, 0x71, 0xb6, 0xef, 0x68, 0xfa, 0x22, 0x1a }),
        dataMbedTls);
}
