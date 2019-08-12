#include "upgrade/security_key_generator/MaterialGenerator.hpp"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
        application::MaterialGenerator generator(randomDataGenerator);
        generator.WriteKeys("Keys.cpp");
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return 2;
    }

    return 0;
}
