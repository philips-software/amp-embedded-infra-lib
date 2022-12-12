#ifndef INFRA_MOVE_CONSTRUCTIBLE_HPP
#define INFRA_MOVE_CONSTRUCTIBLE_HPP

namespace infra
{
    struct MoveConstructible
    {
        explicit MoveConstructible(int a)
            : x(a)
        {}

        MoveConstructible(const MoveConstructible& other) = delete;

        MoveConstructible(MoveConstructible&& other) noexcept
            : x(other.x)
        {}

        MoveConstructible& operator=(const MoveConstructible& other) = delete;

        MoveConstructible& operator=(MoveConstructible&& other) noexcept
        {
            x = other.x;
            return *this;
        }

        bool operator==(const MoveConstructible& other) const
        {
            return x == other.x;
        }

        bool operator<(const MoveConstructible& other) const
        {
            return x < other.x;
        }

        int x;
    };
}

#endif
