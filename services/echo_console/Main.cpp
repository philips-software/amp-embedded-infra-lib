#include "args.hxx"
#include "generated/echo/TracingServiceDiscovery.pb.hpp"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/StreamErrorPolicy.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/SharedOptional.hpp"
#include "protobuf/echo/Serialization.hpp"
#include "protobuf/meta_services/PeerServiceDiscoverer.hpp"
#include "services/echo_console/ConsoleService.hpp"
#include "services/network/TracingEchoOnConnection.hpp"
#include "services/tracer/Tracer.hpp"
#include <cstdint>
#include <ostream>
#include <queue>
#include <vector>
#ifdef EMIL_HAL_WINDOWS
#include "hal/windows/UartWindows.hpp"
#else
#include "hal/unix/UartUnix.hpp"
#endif
#include "hal/generic/UartGeneric.hpp"
#include "infra/stream/IoOutputStream.hpp"
#include "services/echo_console/Console.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/network/HttpClientImpl.hpp"
#include "services/network/WebSocketClientConnectionObserver.hpp"
#include "services/tracer/GlobalTracer.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/SesameWindowed.hpp"
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>

class ConsoleClientUart
    : public application::ConsoleObserver
    , private services::SesameObserver
{
public:
    ConsoleClientUart(application::Console& console, hal::BufferedSerialCommunication& serial);

    // Implementation of ConsoleObserver
    void Send(const std::vector<uint8_t>& message) override;

private:
    // Implementation of SesameObserver
    void Initialized() override;
    void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
    void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;

private:
    void CheckDataToBeSent();

private:
    std::deque<std::vector<uint8_t>> messagesToBeSent;
    services::SesameCobs::WithMaxMessageSize<2048> cobs;
    services::SesameWindowed windowed{ cobs };
    bool sending = false;
    services::SesameObserver::DelayedAttachDetach delayed{ *this, windowed };
};

ConsoleClientUart::ConsoleClientUart(application::Console& console, hal::BufferedSerialCommunication& serial)
    : application::ConsoleObserver(console)
    , cobs(serial)
{}

void ConsoleClientUart::Send(const std::vector<uint8_t>& message)
{
    auto size = windowed.MaxSendMessageSize();
    for (std::size_t index = 0; index < message.size(); index += size)
        messagesToBeSent.push_back(std::vector<uint8_t>(message.begin() + index, message.begin() + size));
    CheckDataToBeSent();
}

void ConsoleClientUart::Initialized()
{}

void ConsoleClientUart::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
{
    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << messagesToBeSent.front();
    writer = nullptr;

    messagesToBeSent.pop_front();
    sending = false;

    CheckDataToBeSent();
}

void ConsoleClientUart::ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
{
    ConsoleObserver::Subject().DataReceived(*reader);
}

void ConsoleClientUart::CheckDataToBeSent()
{
    if (!sending && !messagesToBeSent.empty())
    {
        sending = true;
        windowed.RequestSendMessage(static_cast<uint16_t>(messagesToBeSent.front().size()));
    }
}

class ConsoleClientConnection
    : private services::MethodSerializerFactory::OnHeap
    , public services::TracingEchoOnConnection
    , public application::ConsoleObserver
{
public:
    explicit ConsoleClientConnection(application::Console& console, services::Tracer& tracer);

    ~ConsoleClientConnection();

    // ConnectionObserver
    void Attached() override;

    // Implementation of ConsoleObserver
    void Send(const std::vector<uint8_t>& message) override;

private:
    class PeerServiceDiscoveryObserverTracer
        : public application::PeerServiceDiscoveryObserver
    {
    public:
        PeerServiceDiscoveryObserverTracer(application::PeerServiceDiscovererEcho& subject, services::Tracer& tracer)
            : PeerServiceDiscoveryObserver(subject)
            , tracer(tracer)
        {}

        // Implementation of PeerServiceDiscoveryObserver
        void ServicesDiscovered(infra::MemoryRange<uint32_t> services) override
        {
            tracer.Trace() << "Services discovered: ";
            for (auto service : services)
                tracer.Continue() << service << " ";
        }

    private:
        services::Tracer& tracer;
    };

    class PeerServiceDiscoveryConsoleInteractor
        : public application::PeerServiceDiscoveryObserver
    {
    public:
        PeerServiceDiscoveryConsoleInteractor(application::PeerServiceDiscovererEcho& subject, application::Console& console)
            : PeerServiceDiscoveryObserver(subject)
            , console(console)
        {}

        // Implementation of PeerServiceDiscoveryObserver
        void ServicesDiscovered(infra::MemoryRange<uint32_t> services) override
        {
            console.ServicesDiscovered(services);
        }

    private:
        application::Console& console;
    };

private:
    void CheckDataToBeSent();
    void SendAsEcho();

private:
    services::Tracer& tracer;
    std::queue<std::vector<uint8_t>> dataQueue;
    infra::NotifyingSharedOptional<infra::ByteInputStream> reader;
    infra::SharedPtr<infra::StreamWriter> writer;
    service_discovery::ServiceDiscoveryTracer serviceDiscoveryTracer;
    service_discovery::ServiceDiscoveryResponseTracer serviceDiscoveryResponseTracer;
    infra::Optional<application::PeerServiceDiscovererEcho> peerServiceDiscoverer;
    infra::Optional<PeerServiceDiscoveryObserverTracer> peerServiceDiscoveryObserverTracer;
    infra::Optional<PeerServiceDiscoveryConsoleInteractor> peerServiceDiscoveryConsoleInteractor;
    infra::Optional<services::ConsoleServiceProxy> consoleServiceProxy;
};

ConsoleClientConnection::ConsoleClientConnection(application::Console& console, services::Tracer& tracer)
    : application::ConsoleObserver(console)
    , services::TracingEchoOnConnection(*static_cast<services::MethodSerializerFactory*>(this), services::echoErrorPolicyAbortOnMessageFormatError, tracer)
    , serviceDiscoveryTracer(*this)
    , serviceDiscoveryResponseTracer(*this)
    , tracer(tracer)
{
    tracer.Trace() << "ConsoleClientConnection";
}

ConsoleClientConnection::~ConsoleClientConnection()
{
    tracer.Trace() << "~ConsoleClientConnection";
}

void ConsoleClientConnection::Attached()
{
    peerServiceDiscoverer.Emplace(*this);
    peerServiceDiscoveryObserverTracer.Emplace(*peerServiceDiscoverer, tracer);
    peerServiceDiscoveryConsoleInteractor.Emplace(*peerServiceDiscoverer, application::ConsoleObserver::Subject());
    consoleServiceProxy.Emplace(*this);
}

void ConsoleClientConnection::Send(const std::vector<uint8_t>& message)
{
    // Do we need to know the serviceid, methodid and treat them differently or do we just send strings back to back?
    dataQueue.push(message);
    SendAsEcho();
}

void ConsoleClientConnection::CheckDataToBeSent()
{
    // if (writer != nullptr && !dataToBeSent.empty())
    // {
    //     infra::TextOutputStream::WithErrorPolicy stream(*writer);
    //     std::size_t amount = std::min(stream.Available(), dataToBeSent.size());
    //     stream << dataToBeSent.substr(0, amount);
    //     dataToBeSent.erase(0, amount);

    //     writer = nullptr;
    //     services::ConnectionObserver::Subject().RequestSendStream(services::ConnectionObserver::Subject().MaxSendStreamSize());
    // }
}

void ConsoleClientConnection::SendAsEcho()
{
    if (!consoleServiceProxy)
        throw std::runtime_error("No console service proxy available");

    if (!dataQueue.empty() && reader.Allocatable())
        consoleServiceProxy->RequestSend([this]()
            {
                auto inputReader = reader.Emplace(dataQueue.front(), infra::softFail);
                reader.OnAllocatable([this]
                    {
                        dataQueue.pop();
                        SendAsEcho();
                    });
                consoleServiceProxy->SendMessage(inputReader);
            });
}

class ConsoleClientTcp
    : private services::ClientConnectionObserverFactoryWithNameResolver
{
public:
    ConsoleClientTcp(services::ConnectionFactoryWithNameResolver& connectionFactory, application::Console& console, const std::string& hostname, services::Tracer& tracer);
    ~ConsoleClientTcp();

private:
    infra::BoundedConstString Hostname() const override;
    uint16_t Port() const override;
    void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;
    void ConnectionFailed(services::ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason reason) override;

private:
    infra::SharedOptional<ConsoleClientConnection> consoleClientConnection;
    infra::SharedPtr<void> connector;
    application::Console& console;
    std::string hostname;
    services::Tracer& tracer;
};

ConsoleClientTcp::ConsoleClientTcp(services::ConnectionFactoryWithNameResolver& connectionFactory, application::Console& console, const std::string& hostname, services::Tracer& tracer)
    : console(console)
    , hostname(hostname)
    , tracer(tracer)
{
    tracer.Trace() << "Connecting to " << Hostname() << " port " << Port();
    tracer.Trace();

    connectionFactory.Connect(*this);
}

ConsoleClientTcp::~ConsoleClientTcp()
{
    tracer.Trace() << "~ConsoleClientTcp";

    if (!!consoleClientConnection)
        consoleClientConnection->services::ConnectionObserver::Subject().AbortAndDestroy();
}

infra::BoundedConstString ConsoleClientTcp::Hostname() const
{
    return services::HostFromUrl(infra::BoundedConstString(hostname));
}

uint16_t ConsoleClientTcp::Port() const
{
    return services::PortFromUrl(hostname).ValueOr(1234);
}

void ConsoleClientTcp::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
{
    tracer.Trace() << "Connection established";
    tracer.Trace();
    createdObserver(consoleClientConnection.Emplace(console, tracer));
}

void ConsoleClientTcp::ConnectionFailed(services::ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason reason)
{
    tracer.Trace() << "Connection failed";
    exit(1);
}

class ConsoleClientWebSocket
    : private services::WebSocketClientObserverFactory
{
public:
    ConsoleClientWebSocket(services::ConnectionFactoryWithNameResolver& connectionFactory, application::Console& console,
        infra::BoundedString url, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer);

private:
    infra::BoundedString Url() const override;
    uint16_t Port() const override;
    void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> client)>&& createdClientObserver) override;
    void ConnectionFailed(ConnectFailReason reason) override;

private:
    infra::BoundedString url;
    services::HttpClientConnectorWithNameResolverImpl<> clientConnector;
    infra::Creator<services::Stoppable, services::HttpClientWebSocketInitiation, void(services::WebSocketClientObserverFactory& clientObserverFactory, services::HttpClientWebSocketInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator)> httpClientInitiationCreator;
    services::WebSocketClientFactorySingleConnection webSocketFactory;
    services::Tracer& tracer;

    infra::SharedOptional<ConsoleClientConnection> consoleClientConnection;
    application::Console& console;
};

ConsoleClientWebSocket::ConsoleClientWebSocket(services::ConnectionFactoryWithNameResolver& connectionFactory, application::Console& console,
    infra::BoundedString url, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer)
    : url(url)
    , clientConnector(connectionFactory)
    , httpClientInitiationCreator(
          [this](infra::Optional<services::HttpClientWebSocketInitiation>& value, services::WebSocketClientObserverFactory& clientObserverFactory,
              services::HttpClientWebSocketInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator)
          {
              value.Emplace(clientObserverFactory, clientConnector, result, randomDataGenerator);
          })
    , webSocketFactory(randomDataGenerator, { httpClientInitiationCreator })
    , tracer(tracer)
    , console(console)
{
    webSocketFactory.Connect(*this);
}

infra::BoundedString ConsoleClientWebSocket::Url() const
{
    return url;
}

uint16_t ConsoleClientWebSocket::Port() const
{
    return 80;
}

void ConsoleClientWebSocket::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> client)>&& createdClientObserver)
{
    tracer.Trace() << "Connection established";
    tracer.Trace();
    createdClientObserver(consoleClientConnection.Emplace(console, tracer));
}

void ConsoleClientWebSocket::ConnectionFailed(ConnectFailReason reason)
{
    tracer.Trace() << "Connection failed";
    exit(1);
}

int main(int argc, char* argv[], const char* env[])
{
    infra::IoOutputStream ioOutputStream;
    services::TracerToStream tracer(ioOutputStream);
    services::SetGlobalTracerInstance(tracer);
    hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;

    std::string toolname = argv[0];
    args::ArgumentParser parser(toolname + " is a tool to communicate with an ECHO server.");
    args::Group arguments(parser, "Arguments:");
    args::Positional<std::string> target(arguments, "target", "COM port or hostname", args::Options::Required);
    args::Group flags(parser, "Optional flags:");
    args::HelpFlag help(flags, "help", "Display this help menu.", { 'h', "help" });
    args::PositionalList<std::string> paths(arguments, "paths", "compiled proto files");

    try
    {
        parser.Prog(toolname);
        parser.ParseCLI(argc, argv);

        application::EchoRoot root;

        namespace fs = std::filesystem;

        std::vector<fs::path> pbFiles;

        for (const fs::path path : paths)
        {
            if (fs::is_directory(path))
                for (const fs::directory_entry& entry : fs::recursive_directory_iterator(path))
                {
                    auto path = entry.path();

                    if (path.extension() == ".pb" && path.parent_path().string().rfind("/echo") != std::string::npos)
                        pbFiles.push_back(path);
                }
            else if (path.extension() == ".pb")
                pbFiles.push_back(path);
        }

        for (const auto& path : pbFiles)
        {
            std::ifstream stream(path, std::ios::binary);
            if (!stream)
            {
                std::cerr << argv[0] << ": Could not read " << path << std::endl;
                return 1;
            }

            google::protobuf::FileDescriptorSet descriptorSet;
            if (!descriptorSet.ParseFromIstream(&stream))
            {
                std::cerr << argv[0] << ": Could not parse contents from " << path << std::endl;
                return 1;
            }

            root.AddDescriptorSet(descriptorSet);
            std::cout << "Loaded " << path << std::endl;
        }

        bool serialConnectionRequested = get(target).substr(0, 3) == "COM" || get(target).substr(0, 4) == "/dev";
        application::Console console(root, !serialConnectionRequested);
        services::ConnectionFactoryWithNameResolverImpl::WithStorage<4> connectionFactory(console.ConnectionFactory(), console.NameResolver());
        infra::Optional<ConsoleClientTcp> consoleClientTcp;
        infra::Optional<ConsoleClientWebSocket> consoleClientWebSocket;
        infra::Optional<hal::UartGeneric> uart;
        infra::Optional<hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<2048>> bufferedUart;
        infra::Optional<ConsoleClientUart> consoleClientUart;

        if (serialConnectionRequested)
        {
            uart.Emplace(get(target));
            bufferedUart.Emplace(*uart);
            consoleClientUart.Emplace(console, *bufferedUart);
        }
        else if (services::SchemeFromUrl(infra::BoundedConstString(get(target))) == "ws")
            consoleClientWebSocket.Emplace(connectionFactory, console, get(target), randomDataGenerator, tracer);
        else
            consoleClientTcp.Emplace(connectionFactory, console, get(target), tracer);

        console.Run();
    }
    catch (const args::Help&)
    {
        std::cout << parser;
        return 1;
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
