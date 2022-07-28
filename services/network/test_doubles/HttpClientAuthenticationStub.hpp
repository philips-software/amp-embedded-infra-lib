#ifndef SERVICES_NETWORK_HTTP_CLIENT_AUTHENTICATION_STUB_HPP
#define SERVICES_NETWORK_HTTP_CLIENT_AUTHENTICATION_STUB_HPP

#include "gmock/gmock.h"
#include "services/network/HttpClientAuthentication.hpp"

class HttpClientAuthenticationStub
    : public services::HttpClientAuthentication
{
public:
    template<std::size_t MaxHeaders>
        using WithMaxHeaders = infra::WithStorage<HttpClientAuthenticationStub, infra::BoundedVector<services::HttpHeader>::WithMaxSize<MaxHeaders>>;

    using services::HttpClientAuthentication::HttpClientAuthentication;

protected:
    // Implementation of HttpClientAuthentication
    virtual void Authenticate(infra::BoundedConstString scheme, infra::BoundedConstString value) override
    {}

    virtual infra::BoundedConstString AuthenticationHeader() const override
    {
        return "header";
    }

    virtual bool Retry() const override
    {
        return false;
    }

    virtual void Reset() override
    {}
};

#endif
