#ifndef SERVICES_TRACER_HPP
#define SERVICES_TRACER_HPP

#include "infra/stream/OutputStream.hpp"

namespace services
{
    class Tracer
    {
    public:
        class TracingTextOutputStream;

        Tracer() = default;
        Tracer(const Tracer& other) = delete;
        Tracer& operator=(const Tracer& other) = delete;
        virtual ~Tracer() = default;

        virtual infra::TextOutputStream Continue() = 0;

#ifdef EMIL_ENABLE_GLOBAL_TRACING
        infra::TextOutputStream Trace();
#else
        class EmptyTracing
        {
        public:
            template<class T>
            EmptyTracing operator<<(T x)
            {
                return *this;
            }
        };

        EmptyTracing Trace();

    private:
        infra::StreamWriterDummy dummy;
        infra::TextOutputStream::WithErrorPolicy dummyStream{ dummy };
#endif

    protected:
        friend class TracerToDelegate;

        virtual void InsertHeader();
        virtual void StartTrace();
    };

    class TracerToStream
        : public Tracer
    {
    public:
        explicit TracerToStream(infra::TextOutputStream& stream);

        infra::TextOutputStream Continue() override;

    private:
        infra::TextOutputStream& stream;
    };

    class TracerToDelegate
        : public Tracer
    {
    public:
        TracerToDelegate(Tracer& delegate);

        infra::TextOutputStream Continue() override;

    protected:
        void InsertHeader() override;
        void StartTrace() override;

    private:
        Tracer& delegate;
    };

    class TracerColoured
        : public TracerToDelegate
    {
    public:
        TracerColoured(uint8_t colour, services::Tracer& delegate)
            : TracerToDelegate(delegate)
            , colour(colour)
        {}

        static inline const uint8_t resetColour = 0;
        static inline const uint8_t black = 30;
        static inline const uint8_t red = 31;
        static inline const uint8_t green = 32;
        static inline const uint8_t yellow = 33;
        static inline const uint8_t blue = 34;
        static inline const uint8_t magenta = 35;
        static inline const uint8_t cyan = 36;
        static inline const uint8_t white = 37;
        static inline const uint8_t brightBlack = 90;
        static inline const uint8_t brightRed = 91;
        static inline const uint8_t brightGreen = 92;
        static inline const uint8_t brightYellow = 93;
        static inline const uint8_t brightBlue = 94;
        static inline const uint8_t brightMagenta = 95;
        static inline const uint8_t brightCyan = 96;
        static inline const uint8_t brightWhite = 97;

    protected:
        void StartTrace() override
        {
            TracerToDelegate::StartTrace();
            Continue() << "\033[" << colour << "m";
        }

    private:
        uint8_t colour;
    };
}

#endif
