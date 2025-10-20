#include "args.hxx"
#include "hal/generic/FileSystemGeneric.hpp"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "hal/generic/UartGeneric.hpp"
#include "infra/stream/IoOutputStream.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/syntax/Json.hpp"
#include "infra/util/Tokenizer.hpp"
#include "services/echo_console/Console.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/network/HttpClientImpl.hpp"
#include "services/network/WebSocketClientConnectionObserver.hpp"
#include "services/tracer/GlobalTracer.hpp"
#include "services/util/EchoPolicyDiffieHellman.hpp"
#include "services/util/EchoPolicySymmetricKey.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/SesameWindowed.hpp"
#include <deque>
#include <fstream>
#include <iostream>

class ConsoleClientUart
    : public application::ConsoleObserver
    , private services::SesameObserver
    , private services::EchoInitialization
{
public:
    ConsoleClientUart(application::Console& console, hal::BufferedSerialCommunication& serial);
    ConsoleClientUart(application::Console& console, hal::BufferedSerialCommunication& serial, const sesame_security::SymmetricKeyFile& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator);
    ConsoleClientUart(application::Console& console, hal::BufferedSerialCommunication& serial, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator);

    // Implementation of ConsoleObserver
    void Send(const std::string& message) override;

private:
    // Implementation of SesameObserver
    void Initialized() override;
    void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
    void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;

private:
    void CheckDataToBeSent();

private:
    std::deque<std::string> messagesToBeSent;
    services::SesameCobs::WithMaxMessageSize<2048> cobs;
    services::SesameWindowed windowed{ cobs };
    std::optional<services::SesameSecured::WithCryptoMbedTls::WithBuffers<2048>> secured;
    services::Sesame& sesame;
    bool sending = false;
    services::SesameObserver::DelayedAttachDetach delayed{ *this, sesame };
    std::optional<services::EchoPolicySymmetricKey> policySymmetricKey;
    std::optional<services::EchoPolicyDiffieHellman::WithCryptoMbedTls> policyDiffieHellman;
};

ConsoleClientUart::ConsoleClientUart(application::Console& console, hal::BufferedSerialCommunication& serial)
    : application::ConsoleObserver(console)
    , cobs(serial)
    , sesame(windowed)
{}

ConsoleClientUart::ConsoleClientUart(application::Console& console, hal::BufferedSerialCommunication& serial, const sesame_security::SymmetricKeyFile& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
    : application::ConsoleObserver(console)
    , cobs(serial)
    , secured{ std::in_place, windowed, keyMaterial }
    , sesame{ *secured }
    , policySymmetricKey{ std::in_place, application::ConsoleObserver::Subject(), static_cast<services::EchoInitialization&>(*this), *secured, randomDataGenerator }
{}

ConsoleClientUart::ConsoleClientUart(application::Console& console, hal::BufferedSerialCommunication& serial, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
    : application::ConsoleObserver(console)
    , cobs(serial)
    , secured{ std::in_place, windowed, services::SesameSecured::KeyMaterial{} }
    , sesame{ *secured }
    , policyDiffieHellman{ std::in_place, application::ConsoleObserver::Subject(), static_cast<services::EchoInitialization&>(*this), *secured, keyMaterial, randomDataGenerator }
{}

void ConsoleClientUart::Send(const std::string& message)
{
    auto size = sesame.MaxSendMessageSize();
    for (std::size_t index = 0; index < message.size(); index += size)
        messagesToBeSent.push_back(message.substr(index, size));
    CheckDataToBeSent();
}

void ConsoleClientUart::Initialized()
{
    services::EchoInitialization::NotifyObservers([](auto& observer)
        {
            observer.Initialized();
        });
}

void ConsoleClientUart::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
{
    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << infra::StringAsByteRange(infra::BoundedConstString(messagesToBeSent.front()));
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
        sesame.RequestSendMessage(static_cast<uint16_t>(messagesToBeSent.front().size()));
    }
}

class ConsoleClientConnection
    : public services::ConnectionObserver
    , public application::ConsoleObserver
{
public:
    explicit ConsoleClientConnection(application::Console& console);

    // Implementation of ConnectionObserver
    void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
    void DataReceived() override;
    void Attached() override;

    // Implementation of ConsoleObserver
    void Send(const std::string& message) override;

private:
    void CheckDataToBeSent();

private:
    std::string dataToBeSent;
    infra::SharedPtr<infra::StreamWriter> writer;
};

ConsoleClientConnection::ConsoleClientConnection(application::Console& console)
    : application::ConsoleObserver(console)
{}

void ConsoleClientConnection::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
{
    this->writer = writer;
    writer = nullptr;

    CheckDataToBeSent();
}

void ConsoleClientConnection::DataReceived()
{
    auto stream = services::ConnectionObserver::Subject().ReceiveStream();
    ConsoleObserver::Subject().DataReceived(*stream);
    services::ConnectionObserver::Subject().AckReceived();
}

void ConsoleClientConnection::Attached()
{
    services::ConnectionObserver::Subject().RequestSendStream(services::ConnectionObserver::Subject().MaxSendStreamSize());
}

void ConsoleClientConnection::Send(const std::string& message)
{
    dataToBeSent += message;
    CheckDataToBeSent();
}

void ConsoleClientConnection::CheckDataToBeSent()
{
    if (writer != nullptr && !dataToBeSent.empty())
    {
        infra::TextOutputStream::WithErrorPolicy stream(*writer);
        std::size_t amount = std::min(stream.Available(), dataToBeSent.size());
        stream << dataToBeSent.substr(0, amount);
        dataToBeSent.erase(0, amount);

        writer = nullptr;
        services::ConnectionObserver::Subject().RequestSendStream(services::ConnectionObserver::Subject().MaxSendStreamSize());
    }
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
    if (!!consoleClientConnection)
        consoleClientConnection->services::ConnectionObserver::Subject().AbortAndDestroy();
}

infra::BoundedConstString ConsoleClientTcp::Hostname() const
{
    return services::HostFromUrl(infra::BoundedConstString(hostname));
}

uint16_t ConsoleClientTcp::Port() const
{
    return services::PortFromUrl(hostname).value_or(1234);
}

void ConsoleClientTcp::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
{
    tracer.Trace() << "Connection established";
    tracer.Trace();
    createdObserver(consoleClientConnection.Emplace(console));
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
          [this](std::optional<services::HttpClientWebSocketInitiation>& value, services::WebSocketClientObserverFactory& clientObserverFactory,
              services::HttpClientWebSocketInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator)
          {
              value.emplace(clientObserverFactory, clientConnector, result, randomDataGenerator);
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
    createdClientObserver(consoleClientConnection.Emplace(console));
}

void ConsoleClientWebSocket::ConnectionFailed(ConnectFailReason reason)
{
    tracer.Trace() << "Connection failed";
    exit(1);
}

namespace
{
    hal::FileSystemGeneric fileSystem;

    template<class T>
    T ReadProtoFile(const std::string& filename)
    {
        auto contents = fileSystem.ReadBinaryFile(filename);
        infra::StdVectorInputStream stream{ contents };
        infra::ProtoParser parser{ stream };
        return T(parser);
    }
}

int main(int argc, char* argv[], const char* env[])
{
    infra::IoOutputStream ioOutputStream;
    services::TracerToStream tracer(ioOutputStream);
    services::SetGlobalTracerInstance(tracer);
    hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
    const uint32_t baudrateDefault = 115200;

    std::string toolname = argv[0];
    args::ArgumentParser parser(toolname + " is a tool to communicate with an ECHO server.");
    args::Group target(parser, "Target:");
    args::ValueFlag<std::string> targetPort(parser, "", "COM port", { "port" });
    args::ValueFlag<uint32_t> baudrateOptional(parser, "baudrate", "COM Port communication baudrate. (default used: " + std::to_string(baudrateDefault) + ")", { 'b', "baudrate" });
    args::ValueFlag<std::string> targetHost(parser, "", "Hostname", { "host" });
    args::Group arguments(parser, "Arguments:");
    args::ValueFlag<std::string> symmetricKeyFile(arguments, "file", "File containing the symmetric keys for SESAME encryption.", { "symmetric-keys" });
    args::ValueFlag<std::string> rootCertificateFile(arguments, "file", "File containing the root certificate for SESAME encryption with Diffie-Hellman key exchange.", { "root-certificate" });
    args::ValueFlag<std::string> clientCertificateFile(arguments, "file", "File containing the client certificate for SESAME encryption with Diffie-Hellman key exchange.", { "client-certificate" });
    args::ValueFlag<std::string> clientCertificatePrivateFile(arguments, "file", "File containing the client certificate private key for SESAME encryption with Diffie-Hellman key exchange.", { "client-certificate-private-key" });
    args::Group flags(parser, "Optional flags:");
    args::HelpFlag help(flags, "help", "Display this help menu.", { 'h', "help" });
    args::PositionalList<std::string> paths(arguments, "paths", "compiled proto files");

    try
    {
        parser.Prog(toolname);
        parser.ParseCLI(argc, argv);

        application::EchoRoot root;

        for (const auto& path : paths)
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

        bool serialConnectionRequested = !get(targetPort).empty();
        application::Console console(root, !serialConnectionRequested);
        services::ConnectionFactoryWithNameResolverImpl::WithStorage<4> connectionFactory(console.ConnectionFactory(), console.NameResolver());
        std::optional<ConsoleClientTcp> consoleClientTcp;
        std::optional<ConsoleClientWebSocket> consoleClientWebSocket;
        std::optional<hal::UartGeneric> uart;
        std::optional<hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<2048>> bufferedUart;
        std::optional<ConsoleClientUart> consoleClientUart;

        if (serialConnectionRequested)
        {
            auto actualBaudrate = baudrateOptional ? get(baudrateOptional) : baudrateDefault;
            hal::UartGeneric::Config config = { actualBaudrate };
            uart.emplace(get(targetPort), config);
            bufferedUart.emplace(*uart);

            if (!get(symmetricKeyFile).empty())
                consoleClientUart.emplace(console, *bufferedUart, ReadProtoFile<sesame_security::SymmetricKeyFile>(get(symmetricKeyFile)), randomDataGenerator);
            else if (!get(rootCertificateFile).empty() && !get(clientCertificateFile).empty() && !get(clientCertificatePrivateFile).empty())
            {
                static auto clientCertificate = fileSystem.ReadBinaryFile(get(clientCertificateFile));
                static auto clientCertificatePrivateKey = fileSystem.ReadBinaryFile(get(clientCertificatePrivateFile));
                static auto rootCertificate = fileSystem.ReadBinaryFile(get(rootCertificateFile));
                consoleClientUart.emplace(console, *bufferedUart, services::EchoPolicyDiffieHellman::KeyMaterial{ clientCertificate, clientCertificatePrivateKey, rootCertificate }, randomDataGenerator);
            }
            else
                consoleClientUart.emplace(console, *bufferedUart);
        }
        else if (services::SchemeFromUrl(infra::BoundedConstString(get(targetHost))) == "ws")
            consoleClientWebSocket.emplace(connectionFactory, console, get(targetHost), randomDataGenerator, tracer);
        else
            consoleClientTcp.emplace(connectionFactory, console, get(targetHost), tracer);

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
