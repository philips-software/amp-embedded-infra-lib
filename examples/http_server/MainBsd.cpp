#include "examples/http_server/TimeHttpPage.hpp"
#include "services/network_bsd/ConnectionBsd.hpp"
#include "hal/bsd/TimerServiceBsd.hpp"
#include "services/network/HttpServer.hpp"
#include "services/network_bsd/EventDispatcherWithNetwork.hpp"

int main(int argc, const char* argv[], const char* env[])
{
    services::EventDispatcherWithNetwork eventDispatcherWithNetwork;
    hal::TimerServiceBsd timerService;

    services::DefaultHttpServer::WithBuffer<2048> httpServer(eventDispatcherWithNetwork, 80);
    application::TimeHttpPage timePage;
    httpServer.AddPage(timePage);

    eventDispatcherWithNetwork.Run();
}
