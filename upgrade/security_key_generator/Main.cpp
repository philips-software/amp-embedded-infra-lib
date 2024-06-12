#include "args.hxx"
#include "hal/generic/FileSystemGeneric.hpp"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "upgrade/security_key_generator/MaterialGenerator.hpp"
#include <iostream>
#include <string>

void GenerateUpgradeKeys(args::Subparser& p)
{
    args::Group options(p, "Options:");
    args::ValueFlag<std::string> outputFile(options, "OutputFile", "Output file name.", { 'o', "output" });
    args::ValueFlag<std::string> format(options, "Format", "Output format: {proto, cpp}", { 'f', "format" }, "proto");
    args::ValueFlag<uint32_t> ecDsaKeySize(options, "KeySize", "EcDsa key size: {224, 256}", { 's', "size" }, args::Options::Required);

    p.Parse();

    auto keySize = ecDsaKeySize.Get();
    if (keySize != 224 || keySize != 256)
        throw std::runtime_error("Invalid EcDsa key size.");

    std::string defaultKeyname = std::string("Keys") + std::to_string(keySize);

    hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
    application::MaterialGenerator generator(randomDataGenerator);
    generator.GenerateKeys();

    if (format.Get() == "proto")
        generator.WriteKeysProto(outputFile.Get().empty() ? defaultKeyname + ".bin" : outputFile.Get());
    else if (format.Get() == "cpp")
        generator.WriteKeys(outputFile.Get().empty() ? defaultKeyname + ".cpp" : outputFile.Get());
    else
        throw std::runtime_error("Invalid output format.");
}

void ConvertUpgradeKeys(args::Subparser& p)
{
    args::Group options(p, "Options:");
    args::ValueFlag<std::string> inputFile(options, "InputFile", "Input file name.", { 'i', "input" });
    args::ValueFlag<std::string> outputFile(options, "OutputFile", "Output file name.", { 'o', "output" }, "Keys.bin");

    p.Parse();

    if (inputFile.Get().empty())
        throw std::runtime_error("No input file for importing keys is provided.");

    hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
    application::MaterialGenerator generator(randomDataGenerator);
    generator.ImportKeys(inputFile.Get());
    generator.WriteKeysProto(outputFile.Get());
}

int main(int argc, char* argv[])
{
    std::string toolname = std::string(argv[0]).substr(std::string(argv[0]).find_last_of("\\") + 1);
    args::ArgumentParser parser("\"" + toolname + "\"" + " is a tool used to generate and convert upgrade keys.");

    args::Group commands(parser, "Commands:");
    args::Command generateKeysCommand(commands, "generate_keys", "Generate upgrade keys and save in protobuf format to the specified output file[Keys.bin].", [](args::Subparser& p)
        {
            GenerateUpgradeKeys(p);
        });
    args::Command convertKeysCommand(commands, "convert_keys", "Convert upgrade keys from the provided cpp file and save in protobuf format to the specified output file[Keys.bin].", [](args::Subparser& p)
        {
            ConvertUpgradeKeys(p);
        });

    args::Group arguments(parser, "Optional arguments:");
    args::HelpFlag help(arguments, "help", "Display this help menu.", { 'h', "help" });

    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (const args::Completion& e)
    {
        std::cout << e.what();
        return 0;
    }
    catch (const args::Help&)
    {
        std::cout << parser;
        return 0;
    }
    catch (const args::ParseError& e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }

    return 0;
}
