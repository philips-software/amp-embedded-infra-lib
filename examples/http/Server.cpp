#include "hal/generic/TimerServiceGeneric.hpp"
#include "services/network/HttpServer.hpp"
#include "services/network_instantiations/NetworkAdapter.hpp"

int main(int argc, const char* argv[], const char* env[])
{
    static hal::TimerServiceGeneric timerService;
    static main_::NetworkAdapter network;
    static services::DefaultHttpServer::WithBuffer<2048> httpServer(network.ConnectionFactory(), 80);
    static services::HttpPageWithContent page("", "<h1>Welcome</h1>", "text/html");

    httpServer.AddPage(page);
    network.Run();

    return 0;
}
