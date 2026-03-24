#ifndef INFRA_STREAM_FLUSHABLE_HPP
#define INFRA_STREAM_FLUSHABLE_HPP

namespace infra
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

#endif // INFRA_STREAM_FLUSHABLE_HPP
