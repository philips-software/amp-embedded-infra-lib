#include "examples/http_server/TimeHttpPage.hpp"
#include "hal/generic/TimerServiceGeneric.hpp"
#include "services/network/HttpServer.hpp"
#include "services/network_bsd/EventDispatcherWithNetwork.hpp"

int main(int argc, const char* argv[], const char* env[])
{
    static services::EventDispatcherWithNetwork eventDispatcherWithNetwork;
    static hal::TimerServiceGeneric timerService;

    static services::DefaultHttpServer::WithBuffer<2048> httpServer(eventDispatcherWithNetwork, 80);
    static application::TimeHttpPage timePage;
    httpServer.AddPage(timePage);

    eventDispatcherWithNetwork.Run();
}
