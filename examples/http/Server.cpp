#include "examples/network_example/Network.hpp"
#include "hal/generic/TimerServiceGeneric.hpp"
#include "services/network/HttpServer.hpp"

int Main(main_::Network& network, int argc, const char* argv[], const char* env[])
{
    static hal::TimerServiceGeneric timerService;
    static services::DefaultHttpServer::WithBuffer<2048> httpServer(network.ConnectionFactory(), 80);
    static services::HttpPageWithContent page("", "<h1>Welcome</h1>", "text/html");

    httpServer.AddPage(page);
    network.Run();

    return 0;
}
