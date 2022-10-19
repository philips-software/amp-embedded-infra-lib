# EmbeddedInfraLib (EmIL)

[![Continuous Integration](https://github.com/philips-software/embeddedinfralib/workflows/Continuous%20Integration/badge.svg)](https://github.com/philips-software/embeddedinfralib/actions) [![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=philips-software_embeddedinfralib&metric=alert_status)](https://sonarcloud.io/dashboard?id=philips-software_embeddedinfralib) [![Coverage](https://sonarcloud.io/api/project_badges/measure?project=philips-software_embeddedinfralib&metric=coverage)](https://sonarcloud.io/dashboard?id=philips-software_embeddedinfralib) [![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://choosealicense.com/licenses/mit/) [![Documentation](https://img.shields.io/website?down_message=offline&label=Documentation&up_message=online&url=https%3A%2F%2Fimg.shields.io%2Fwebsite-up-down-green-red%2Fhttps%2Fphilips-software.github.io%2amp-embedded-infra-lib.svg)](https://philips-software.github.io/amp-embedded-infra-lib/)

**Description**: EmbeddedInfraLib is a set of C++ libraries and headers that provide heap-less, STL like, infrastructure for embedded software development.

## Dependencies

EmIL requires:
- A recent C++ compiler that supports C++17 at minimum (for a host build it should support std::filesystem).
- CMake 3.24 or higher.

EmIL is know to build under the following configurations:
- Windows from Visual Studio 2017 onwards.
- Linux from GCC 5.3 onwards.
- OSX from XCode 11 and target platform 10.15 onwards.

## How to build the software

EmIL can be built by-itself, for example to execute the included micro-tests, or it can be built as part of a larger project. This paragraph describes how to build EmIL by-itself.

```
cmake -B Build
cmake --build Build
```

## How to test the software

After EmIL has been built. The included automated tests can be run with CTest like so:

```
ctest -D Experimental -C Debug
```

## Code examples

Code examples can be found under the [examples](examples) folder.

## Documentation

Documentation is available on [philips-software.github.io/amp-embedded-infra-lib](https://philips-software.github.io/amp-embedded-infra-lib/).

## Contributing

Please refer to our [Contributing](CONTRIBUTING.md) guide when you want to contribute to this project.

## License

EmbeddedInfraLib is licenced under the [MIT](https://choosealicense.com/licenses/mit/) license. See [LICENSE file](LICENSE.md).
