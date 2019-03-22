#include "services/network_win/ConnectionWin.hpp"
#include "hal/windows/TimerServiceWin.hpp"
#include "infra/syntax/JsonFormatter.hpp"
#include "services/network/HttpServer.hpp"
#include "services/network_win/EventDispatcherWithNetwork.hpp"

namespace application
{
    class TimeHttpPage
        : public services::HttpPage
    {
    public:
        // Implementation of HttpPage
        virtual bool ServesRequest(const infra::Tokenizer& pathTokens) const override
        {
            return pathTokens.TokenAndRest(0) == "";
        }

        virtual void RespondToRequest(services::HttpRequestParser& parser, services::HttpServerConnection& connection) override
        {
            connection.SendResponse(timeResponse);
        }

    private:
        class TimeResponse
            : public services::HttpResponse
        {
        public:
            TimeResponse()
                : services::HttpResponse(128)
            {}

            virtual infra::BoundedConstString Status() const override
            {
                return "200 OK";
            }

            virtual infra::BoundedConstString ContentType() const override
            {
                return "application/json";
            }

            virtual void WriteBody(infra::TextOutputStream& stream) const override
            {
                infra::JsonObjectFormatter json(stream);
                json.Add("time", std::chrono::system_clock::now().time_since_epoch().count());
            }

            virtual void AddHeaders(services::HttpResponseHeaderBuilder& builder) const
            {}
        };

        TimeResponse timeResponse;
    };
}

int main(int argc, const char* argv[], const char* env[])
{
    services::EventDispatcherWithNetwork eventDispatcherWithNetwork;
    hal::TimerServiceWin timerService;

    services::DefaultHttpServer::WithBuffer<2048> httpServer(eventDispatcherWithNetwork, 80);
    application::TimeHttpPage timePage;
    httpServer.AddPage(timePage);

    eventDispatcherWithNetwork.Run();
}
