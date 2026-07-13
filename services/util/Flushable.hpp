#ifndef SERVICES_UTIL_FLUSHABLE_HPP
#define SERVICES_UTIL_FLUSHABLE_HPP

namespace services
{
    class Flushable
    {
    protected:
        Flushable() = default;
        ~Flushable() = default;

    public:
        virtual void Flush() = 0;
    };
}

#endif // SERVICES_UTIL_FLUSHABLE_HPP
