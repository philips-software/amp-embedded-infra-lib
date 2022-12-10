#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "hal/generic/TimerServiceGeneric.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/network/ConnectionMbedTls.hpp"
#include "services/network/HttpClientBasic.hpp"
#include "services/network/HttpClientImpl.hpp"
#include "services/network_instantiations/NetworkAdapter.hpp"
#include "services/tracer/TracerOnIoOutputInfrastructure.hpp"

namespace
{
    const char starfieldClass2Ca[] = "-----BEGIN CERTIFICATE-----\r\n"
                                     "MIIEDzCCAvegAwIBAgIBADANBgkqhkiG9w0BAQUFADBoMQswCQYDVQQGEwJVUzEl\r\n"
                                     "MCMGA1UEChMcU3RhcmZpZWxkIFRlY2hub2xvZ2llcywgSW5jLjEyMDAGA1UECxMp\r\n"
                                     "U3RhcmZpZWxkIENsYXNzIDIgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMDQw\r\n"
                                     "NjI5MTczOTE2WhcNMzQwNjI5MTczOTE2WjBoMQswCQYDVQQGEwJVUzElMCMGA1UE\r\n"
                                     "ChMcU3RhcmZpZWxkIFRlY2hub2xvZ2llcywgSW5jLjEyMDAGA1UECxMpU3RhcmZp\r\n"
                                     "ZWxkIENsYXNzIDIgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwggEgMA0GCSqGSIb3\r\n"
                                     "DQEBAQUAA4IBDQAwggEIAoIBAQC3Msj+6XGmBIWtDBFk385N78gDGIc/oav7PKaf\r\n"
                                     "8MOh2tTYbitTkPskpD6E8J7oX+zlJ0T1KKY/e97gKvDIr1MvnsoFAZMej2YcOadN\r\n"
                                     "+lq2cwQlZut3f+dZxkqZJRRU6ybH838Z1TBwj6+wRir/resp7defqgSHo9T5iaU0\r\n"
                                     "X9tDkYI22WY8sbi5gv2cOj4QyDvvBmVmepsZGD3/cVE8MC5fvj13c7JdBmzDI1aa\r\n"
                                     "K4UmkhynArPkPw2vCHmCuDY96pzTNbO8acr1zJ3o/WSNF4Azbl5KXZnJHoe0nRrA\r\n"
                                     "1W4TNSNe35tfPe/W93bC6j67eA0cQmdrBNj41tpvi/JEoAGrAgEDo4HFMIHCMB0G\r\n"
                                     "A1UdDgQWBBS/X7fRzt0fhvRbVazc1xDCDqmI5zCBkgYDVR0jBIGKMIGHgBS/X7fR\r\n"
                                     "zt0fhvRbVazc1xDCDqmI56FspGowaDELMAkGA1UEBhMCVVMxJTAjBgNVBAoTHFN0\r\n"
                                     "YXJmaWVsZCBUZWNobm9sb2dpZXMsIEluYy4xMjAwBgNVBAsTKVN0YXJmaWVsZCBD\r\n"
                                     "bGFzcyAyIENlcnRpZmljYXRpb24gQXV0aG9yaXR5ggEAMAwGA1UdEwQFMAMBAf8w\r\n"
                                     "DQYJKoZIhvcNAQEFBQADggEBAAWdP4id0ckaVaGsafPzWdqbAYcaT1epoXkJKtv3\r\n"
                                     "L7IezMdeatiDh6GX70k1PncGQVhiv45YuApnP+yz3SFmH8lU+nLMPUxA2IGvd56D\r\n"
                                     "eruix/U0F47ZEUD0/CwqTRV/p2JdLiXTAAsgGh1o+Re49L2L7ShZ3U0WixeDyLJl\r\n"
                                     "xy16paq8U4Zt3VekyvggQQto8PT7dL5WXXp59fkdheMtlb71cZBDzI0fmgAKhynp\r\n"
                                     "VSJYACPq4xJDKVtHCN2MQWplBqjlIapBtJUhlbl90TSrE9atvNziPTnNvT51cKEY\r\n"
                                     "WQPJIrSPnNVeKtelttQKbfi3QBFGmh95DmK/D5fs4C8fF5Q=\r\n"
                                     "-----END CERTIFICATE-----\r\n";
}

namespace application
{
    class TracingHttpClient
        : private services::HttpClientBasic
    {
    public:
        TracingHttpClient(infra::BoundedString url, uint16_t port, services::HttpClientConnector& connector,
                          services::Tracer& tracer)
            : HttpClientBasic(url, port, connector)
            , tracer(tracer)
        {
        }

    private:
        // Implementation of HttpClientBasic
        virtual void Attached() override
        {
            HttpClientObserver::Subject().Get(Path(), Headers());
        }

        virtual void StatusAvailable(services::HttpStatusCode statusCode) override
        {
            tracer.Trace() << "Status: " << statusCode;
        }

        virtual void HeaderAvailable(services::HttpHeader header) override
        {
            tracer.Trace() << "Header: " << header;
        }

        virtual void BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader) override
        {
            infra::DataInputStream::WithErrorPolicy stream(*reader, infra::noFail);

            while (!stream.Empty())
                tracer.Trace() << infra::ByteRangeAsString(stream.ContiguousRange());
        }

        virtual void Done() override
        {
            tracer.Trace() << "Done";
        }

        virtual void Error(bool intermittentFailure) override
        {
            tracer.Trace() << "Error";
        }

    private:
        services::Tracer& tracer;
    };
}

int main(int argc, const char* argv[], const char* env[])
{
    static hal::TimerServiceGeneric timerService;
    static hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
    static main_::TracerOnIoOutputInfrastructure tracer;
    static main_::NetworkAdapter network;
    static services::CertificatesMbedTls certificates;

    certificates.AddCertificateAuthority(infra::BoundedConstString(starfieldClass2Ca, sizeof(starfieldClass2Ca)));
    static services::ConnectionFactoryMbedTls::WithMaxConnectionsListenersAndConnectors<10, 2, 2> networkTls(
        network.ConnectionFactory(), certificates, randomDataGenerator);

    static services::ConnectionFactoryWithNameResolverImpl::WithStorage<1> connectionFactory(networkTls, network.NameResolver());
    static services::ConnectionFactoryWithNameResolverForTls connectionFactoryTls(connectionFactory);
    static services::HttpClientConnectorWithNameResolverImpl<> connector(connectionFactoryTls);

    static infra::BoundedString::WithStorage<32> url{ "httpbin.org/get" };
    static application::TracingHttpClient httpClient(url, 443, connector, tracer.tracer);

    network.Run();

    return 0;
}
