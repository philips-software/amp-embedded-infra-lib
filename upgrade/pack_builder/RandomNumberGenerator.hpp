#ifndef UPGRADE_PACK_BUILDER_LIBRARY_RANDOM_NUMBER_GENERATOR_HPP
#define UPGRADE_PACK_BUILDER_LIBRARY_RANDOM_NUMBER_GENERATOR_HPP

#include <cstdint>
#include <vector>
#include <windows.h>

namespace application
{
    class RandomNumberGenerator
    {
    public:
        virtual std::vector<uint8_t> Generate(std::size_t n) = 0;

    protected:
        ~RandomNumberGenerator() = default;
    };

    class SecureRandomNumberGenerator
        : public RandomNumberGenerator
    {
    public:
        SecureRandomNumberGenerator();
        ~SecureRandomNumberGenerator();

        virtual std::vector<uint8_t> Generate(std::size_t n) override;

    private:
        HCRYPTPROV hProvider = 0;
    };

    class FixedRandomNumberGenerator
        : public RandomNumberGenerator
    {
    public:
        explicit FixedRandomNumberGenerator(const std::vector<uint8_t>& initialData);

        virtual std::vector<uint8_t> Generate(std::size_t n) override;

    private:
        std::vector<uint8_t> initialData;
    };
}

#endif
