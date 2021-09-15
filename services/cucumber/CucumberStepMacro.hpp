#ifndef SERVICES_CUCUMBER_STEP_MACRO_HPP
#define SERVICES_CUCUMBER_STEP_MACRO_HPP

#include "services/cucumber/CucumberStep.hpp"
#include "services/cucumber/CucumberStepStorage.hpp"
#include <initializer_list>

#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)

#define STRINGIZE(x) STRINGIZE_(x)
#define STRINGIZE_(x) #x

#define CLASSNAME CONCAT(Step_, __LINE__)
#define VARNAME CONCAT(step_, __LINE__)

#define SOURCE_FILE CONCAT(CLASSNAME, _file)
#define SOURCE_LINE CONCAT(CLASSNAME, _line)
#define SOURCE_LOCATION CONCAT(CLASSNAME, _location)

#define GIVEN(NAME) DEFINESTEP(NAME)
#define WHEN(NAME) DEFINESTEP(NAME)
#define THEN(NAME) DEFINESTEP(NAME)

namespace detail
{
    template<size_t N>
    struct String
    {
        char c[N];
    };

    template<size_t... Len>
    constexpr auto Concatenate(const char (&... strings)[Len])
    {
        constexpr size_t N = (... + Len) - sizeof...(Len);
        String<N + 1> result = {};
        result.c[N] = '\0';

        char* dst = result.c;
        for (const char* src : { strings... })
            for (; *src != '\0'; src++, dst++)
                *dst = *src;

        return result;
    }

    template<size_t N>
    constexpr auto Filename(const char (&file)[N])
    {
        const char* lastSlash = &file[0];
        for (const auto& c : file)
            if (c == '/' || c == '\\')
                lastSlash = &c;

        String<N + 1> result = {};
        result.c[N] = '\0';

        char* dst = result.c;
        for (const char* src = lastSlash + 1; *src != '\0'; src++, dst++)
            *dst = *src;

        return result;
    }
}

#define DEFINESTEP(NAME)                                                                                     \
    namespace                                                                                                \
    {                                                                                                        \
        constexpr auto SOURCE_FILE = detail::Filename(__FILE__);                                             \
        constexpr char SOURCE_LINE[] = STRINGIZE(__LINE__);                                                  \
        constexpr auto SOURCE_LOCATION = detail::Concatenate(SOURCE_FILE.c, ":", SOURCE_LINE);               \
                                                                                                             \
        class CLASSNAME                                                                                      \
            : public services::CucumberStep                                                                  \
        {                                                                                                    \
        public:                                                                                              \
            CLASSNAME()                                                                                      \
                : services::CucumberStep(NAME, SOURCE_LOCATION.c)                                            \
            {                                                                                                \
                services::CucumberStepStorage::Instance().AddStep(*this);                                    \
            }                                                                                                \
                                                                                                             \
        public:                                                                                              \
            virtual void Invoke(infra::JsonArray& arguments) override                                        \
            {                                                                                                \
                invokeArguments = &arguments;                                                                \
                Execute();                                                                                   \
            }                                                                                                \
                                                                                                             \
        private:                                                                                             \
            void Execute();                                                                                  \
                                                                                                             \
            void Success()                                                                                   \
            {                                                                                                \
                assert(Context().Contains("InvokeSuccess"));                                                 \
                Context().Get<infra::Function<void()>>("InvokeSuccess")();                                   \
            }                                                                                                \
                                                                                                             \
            void Error(infra::BoundedConstString failReason)                                                 \
            {                                                                                                \
                assert(Context().Contains("InvokeError"));                                                   \
                Context().Get<infra::Function<void(infra::BoundedConstString&)>>("InvokeError")(failReason); \
            }                                                                                                \
        };                                                                                                   \
                                                                                                             \
        static CLASSNAME VARNAME;                                                                            \
    }                                                                                                        \
                                                                                                             \
    void CLASSNAME::Execute()

#endif
