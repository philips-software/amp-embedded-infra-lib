#include "args.hxx"
#include "generated/echo/SesameSecurity.pb.hpp"
#include "hal/generic/FileSystemGeneric.hpp"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "infra/stream/IoOutputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "services/util/EchoPolicyDiffieHellman.hpp"
#include "services/util/SesameCryptoMbedTls.hpp"
#include <fstream>
#include <iostream>

namespace
{
    hal::FileSystemGeneric fileSystem;

    template<class T>
    void WriteProtoFile(const T& proto, const std::string& filename)
    {
        infra::StdVectorOutputStream::WithStorage stream;
        infra::ProtoFormatter formatter(stream);
        proto.Serialize(formatter);

        fileSystem.WriteBinaryFile(filename, stream.Storage());
    }

    void GenerateSymmetricKeys(args::Subparser& parser, hal::SynchronousRandomDataGenerator& randomDataGenerator)
    {
        args::ValueFlag<std::string> file(parser, "file", "Generates two files named <file>PeerA.key and <file>PeerB.key, containing the keys for each peer.", { "output" }, args::Options::Required);
        parser.Parse();

        auto keys = services::GenerateSymmetricKeys(randomDataGenerator);

        WriteProtoFile(keys, get(file) + "PeerA.key");
        WriteProtoFile(services::ReverseDirection(keys), get(file) + "PeerB.key");
    }

    void GenerateRootCertificate(args::Subparser& parser, hal::SynchronousRandomDataGenerator& randomDataGenerator)
    {
        args::ValueFlag<std::string> file(parser, "file", "Root certificate", { "output" }, args::Options::Required);
        parser.Parse();

        auto [rootCertificateDer, rootPrivateKeyDer] = services::GenerateRootCertificate(randomDataGenerator);

        fileSystem.WriteBinaryFile(get(file) + ".der", infra::MakeRange(rootCertificateDer));
        fileSystem.WriteBinaryFile(get(file) + ".prv", infra::MakeRange(rootPrivateKeyDer));
    }

    void GenerateDeviceCertificate(args::Subparser& parser, hal::SynchronousRandomDataGenerator& randomDataGenerator)
    {
        args::ValueFlag<std::string> rootKey(parser, "file", "Root certificate private key", { "rootkey" }, args::Options::Required);
        args::ValueFlag<std::string> output(parser, "file", "Device certificate", { "output" }, args::Options::Required);
        parser.Parse();

        auto rootCertificateKeyFile = fileSystem.ReadBinaryFile(get(rootKey));
        services::EcSecP256r1PrivateKey rootCertificateKey(infra::MakeRange(rootCertificateKeyFile), randomDataGenerator);
        auto [deviceCertificateDer, devicePrivateKeyDer] = services::GenerateDeviceCertificate(rootCertificateKey, randomDataGenerator);

        fileSystem.WriteBinaryFile(get(output) + ".der", infra::MakeRange(deviceCertificateDer));
        fileSystem.WriteBinaryFile(get(output) + ".prv", infra::MakeRange(devicePrivateKeyDer));
    }
}

int main(int argc, char* argv[], const char* env[])
{
    hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;

    std::string toolname = argv[0];
    args::ArgumentParser parser(toolname + " is used to generate the various keys needed for secure SESAME operation.");
    args::Group commands(parser, "commands");
    args::Command generateSymmetric(commands, "generate_symmetric_keys", "generate two keyfiles for SESAME with symmetric keys", [&](args::Subparser& parser)
        {
            GenerateSymmetricKeys(parser, randomDataGenerator);
        });
    args::Command generateRootCertificate(commands, "generate_root_certificate", "generate root certificate and private key for SESAME with Diffie-Hellman authentication", [&](args::Subparser& parser)
        {
            GenerateRootCertificate(parser, randomDataGenerator);
        });
    args::Command generateDeviceCertificate(commands, "generate_device_certificate", "generate device certificate and private key for SESAME with Diffie-Hellman authentication", [&](args::Subparser& parser)
        {
            GenerateDeviceCertificate(parser, randomDataGenerator);
        });
    args::HelpFlag h(parser, "help", "help", { 'h', "help" }, args::Options::Global);

    try
    {
        parser.Prog(toolname);
        parser.ParseCLI(argc, argv);
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
