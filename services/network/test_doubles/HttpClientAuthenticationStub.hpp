#ifndef SERVICES_NETWORK_HTTP_CLIENT_AUTHENTICATION_STUB_HPP
#define SERVICES_NETWORK_HTTP_CLIENT_AUTHENTICATION_STUB_HPP

#include "services/network/HttpClientAuthentication.hpp"
#include "gmock/gmock.h"

class HttpClientAuthenticationStub
    : public services::HttpClientAuthentication
{
public:
    template<std::size_t MaxHeaders>
    using WithMaxHeaders = infra::WithStorage<HttpClientAuthenticationStub, infra::BoundedVector<services::HttpHeader>::WithMaxSize<MaxHeaders>>;

    using services::HttpClientAuthentication::HttpClientAuthentication;

protected:
    // Implementation of HttpClientAuthentication
    void Authenticate(infra::BoundedConstString scheme, infra::BoundedConstString value) override
    {}

    infra::BoundedConstString AuthenticationHeader() const override
    {
        return "header";
    }

    bool Retry() const override
    {
        return false;
    }

    void Reset() override
    {}
};

#endif
