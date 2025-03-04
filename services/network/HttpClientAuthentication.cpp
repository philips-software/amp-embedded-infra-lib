#include "services/network/HttpClientAuthentication.hpp"

namespace services
{
    HttpClientAuthentication::HttpClientAuthentication(infra::BoundedVector<HttpHeader>& headersWithAuthorization)
        : headersWithAuthorization(headersWithAuthorization)
    {}

    void HttpClientAuthentication::Get(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        Request(headers, HttpVerb::get, requestTarget, [this]()
            {
                Subject().Get(target, infra::MakeRange(headersWithAuthorization));
            });
    }

    void HttpClientAuthentication::Head(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        Request(headers, HttpVerb::head, requestTarget, [this]()
            {
                Subject().Head(target, infra::MakeRange(headersWithAuthorization));
            });
    }

    void HttpClientAuthentication::Connect(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        Request(headers, HttpVerb::connect, requestTarget, [this]()
            {
                Subject().Connect(target, infra::MakeRange(headersWithAuthorization));
            });
    }

    void HttpClientAuthentication::Options(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        Request(headers, HttpVerb::options, requestTarget, [this]()
            {
                Subject().Options(target, infra::MakeRange(headersWithAuthorization));
            });
    }

    void HttpClientAuthentication::Post(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        Request(headers, HttpVerb::post, requestTarget, [this, content]()
            {
                Subject().Post(target, content, infra::MakeRange(headersWithAuthorization));
            });
    }

    void HttpClientAuthentication::Post(infra::BoundedConstString requestTarget, std::size_t contentSize, HttpHeaders headers)
    {
        Request(headers, HttpVerb::post, requestTarget, [this, contentSize]()
            {
                Subject().Post(target, contentSize, infra::MakeRange(headersWithAuthorization));
            });
    }

    void HttpClientAuthentication::Post(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        Request(headers, HttpVerb::post, requestTarget, [this]()
            {
                Subject().Post(target, infra::MakeRange(headersWithAuthorization));
            });
    }

    void HttpClientAuthentication::Put(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        Request(headers, HttpVerb::put, requestTarget, [this, content]()
            {
                Subject().Put(target, content, infra::MakeRange(headersWithAuthorization));
            });
    }

    void HttpClientAuthentication::Put(infra::BoundedConstString requestTarget, std::size_t contentSize, HttpHeaders headers)
    {
        Request(headers, HttpVerb::put, requestTarget, [this, contentSize]()
            {
                Subject().Put(target, contentSize, infra::MakeRange(headersWithAuthorization));
            });
    }

    void HttpClientAuthentication::Put(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        Request(headers, HttpVerb::put, requestTarget, [this]()
            {
                Subject().Put(target, infra::MakeRange(headersWithAuthorization));
            });
    }

    void HttpClientAuthentication::Patch(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        Request(headers, HttpVerb::patch, requestTarget, [this, content]()
            {
                Subject().Patch(target, content, infra::MakeRange(headersWithAuthorization));
            });
    }

    void HttpClientAuthentication::Patch(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        Request(headers, HttpVerb::patch, requestTarget, [this]()
            {
                Subject().Patch(target, infra::MakeRange(headersWithAuthorization));
            });
    }

    void HttpClientAuthentication::Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        Request(headers, HttpVerb::delete_, requestTarget, [this, content]()
            {
                Subject().Delete(target, content, infra::MakeRange(headersWithAuthorization));
            });
    }

    void HttpClientAuthentication::AckReceived()
    {
        Subject().AckReceived();
    }

    void HttpClientAuthentication::CloseConnection()
    {
        Subject().CloseConnection();
    }

    Connection& HttpClientAuthentication::GetConnection()
    {
        return Subject().GetConnection();
    }

    void HttpClientAuthentication::StatusAvailable(HttpStatusCode statusCode)
    {
        if (statusCode == HttpStatusCode::Unauthorized)
            unauthorized = true;
        else
            Observer().StatusAvailable(statusCode);
    }

    void HttpClientAuthentication::HeaderAvailable(HttpHeader header)
    {
        if (unauthorized)
        {
            if (infra::CaseInsensitiveCompare(header.Field(), "WWW-Authenticate"))
                Authenticate(header.Value());
        }
        else
            Observer().HeaderAvailable(header);
    }

    void HttpClientAuthentication::BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader)
    {
        if (!unauthorized)
            Observer().BodyAvailable(std::move(reader));
        else
        {
            infra::DataInputStream::WithErrorPolicy stream(*reader);
            while (!stream.Empty())
                stream.ContiguousRange();
        }
    }

    void HttpClientAuthentication::BodyComplete()
    {
        if (unauthorized)
        {
            if (Retry())
            {
                unauthorized = false;
                Reset();
                request();
                return;
            }

            Observer().StatusAvailable(HttpStatusCode::Unauthorized);
        }

        if (HttpClient::IsAttached())
            Observer().BodyComplete();
    }

    void HttpClientAuthentication::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        Observer().SendStreamAvailable(std::move(writer));
    }

    void HttpClientAuthentication::FillContent(infra::StreamWriter& writer) const
    {
        Observer().FillContent(writer);
    }

    void HttpClientAuthentication::Detaching()
    {
        if (HttpClient::IsAttached())
            HttpClient::Detach();
        HttpClientObserver::Detaching();
    }

    void HttpClientAuthentication::Request(HttpHeaders headers, HttpVerb verb, infra::BoundedConstString target, const infra::Function<void(), sizeof(void*) + 2 * sizeof(infra::BoundedConstString)>& newRequest)
    {
        MakeHeaders(headers);
        unauthorized = false;
        request = newRequest;
        this->verb = verb;
        this->target = target;
        request();
    }

    void HttpClientAuthentication::MakeHeaders(HttpHeaders headers)
    {
        assert(headers.size() < headersWithAuthorization.max_size());
        headersWithAuthorization.assign(headers.begin(), headers.end());
        headersWithAuthorization.emplace_back("Authorization", AuthenticationHeader());
    }

    void HttpClientAuthentication::Authenticate(infra::BoundedConstString headerValue)
    {
        auto spaceIndex = std::min(headerValue.find(' '), headerValue.size());
        auto scheme = headerValue.substr(0, spaceIndex);
        auto challenge = headerValue.substr(std::min(headerValue.find_first_not_of(' ', spaceIndex), headerValue.size()));

        Authenticate(verb, target, scheme, challenge);

        headersWithAuthorization.pop_back();
        headersWithAuthorization.emplace_back("Authorization", AuthenticationHeader());
    }

    HttpClientAuthenticationConnector::HttpClientAuthenticationConnector(HttpClientConnector& connector, HttpClientAuthentication& clientAuthentication)
        : connector(connector)
        , clientAuthentication(clientAuthentication)
    {}

    void HttpClientAuthenticationConnector::Connect(HttpClientObserverFactory& factory)
    {
        assert(!clientAuthentication.HttpClientObserver::IsAttached() && this->factory == nullptr);
        this->factory = &factory;
        connector.Connect(*this);
    }

    void HttpClientAuthenticationConnector::CancelConnect(HttpClientObserverFactory& factory)
    {
        this->factory = nullptr;
        connector.CancelConnect(*this);
    }

    infra::BoundedConstString HttpClientAuthenticationConnector::Hostname() const
    {
        return factory->Hostname();
    }

    uint16_t HttpClientAuthenticationConnector::Port() const
    {
        return factory->Port();
    }

    void HttpClientAuthenticationConnector::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<HttpClientObserver> client)>&& createdClientObserver)
    {
        factory->ConnectionEstablished([this, &createdClientObserver](infra::SharedPtr<HttpClientObserver> client)
            {
                createdClientObserver(infra::MakeContainedSharedObject(clientAuthentication, client));
                clientAuthentication.Attach(client);
            });

        factory = nullptr;
    }

    void HttpClientAuthenticationConnector::ConnectionFailed(ConnectFailReason reason)
    {
        infra::PostAssign(factory, nullptr)->ConnectionFailed(reason);
    }
}
