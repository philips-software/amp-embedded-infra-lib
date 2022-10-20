#ifndef SERVICES_WRITABLE_STATIC_CONFIGURATION_MOCK_HPP
#define SERVICES_WRITABLE_STATIC_CONFIGURATION_MOCK_HPP

#include "services/util/WritableConfiguration.hpp"
#include "gmock/gmock.h"

namespace services
{
    template<class T>
    class WritableConfigurationWriterMock
        : public WritableConfigurationWriter<T>
    {
    public:
        MOCK_METHOD2_T(Write, void(const T&, const infra::Function<void()>&));
        MOCK_METHOD1(Clear, void(const infra::Function<void()>&));
    };

    template<class TRef>
    class WritableConfigurationReaderMock
        : public WritableConfigurationReader<TRef>
    {
    public:
        MOCK_CONST_METHOD0_T(Valid, bool());
        MOCK_CONST_METHOD0_T(Get, const TRef&());
        MOCK_METHOD1_T(Read, void(const infra::Function<void()>&));
    };
}

#endif
