#include "infra/stream/StdStringOutputStream.hpp"
#include "protobuf/echo/EchoErrorPolicy.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class EchoErrorPolicyTest
    : public testing::Test
{
public:
    infra::StdStringOutputStream::WithStorage stream;
    services::TracerToStream tracer{ stream };
};

TEST_F(EchoErrorPolicyTest, policy_warn_message_format_warns)
{
    services::EchoErrorPolicyWarn policy(tracer);
    policy.MessageFormatError("reason");
    EXPECT_THAT(stream.Storage(), testing::StrEq("\r\nWarning! Echo message format error: reason"));
}

TEST_F(EchoErrorPolicyTest, policy_warn_service_not_found_warns)
{
    services::EchoErrorPolicyWarn policy(tracer);
    policy.ServiceNotFound(42);
    EXPECT_THAT(stream.Storage(), testing::StrEq("\r\nWarning! Echo service not found: serviceID: 42"));
}

TEST_F(EchoErrorPolicyTest, policy_warn_method_not_found_warns)
{
    services::EchoErrorPolicyWarn policy(tracer);
    policy.MethodNotFound(42, 7);
    EXPECT_THAT(stream.Storage(), testing::StrEq("\r\nWarning! Echo method not found: serviceID: 42 methodID: 7"));
}
