# EmbeddedInfraLib (EmIL)

[![Continuous Integration](https://github.com/philips-software/embeddedinfralib/workflows/Continuous%20Integration/badge.svg)](https://github.com/philips-software/embeddedinfralib/actions) [![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=philips-software_embeddedinfralib&metric=alert_status)](https://sonarcloud.io/dashboard?id=philips-software_embeddedinfralib) [![Coverage](https://sonarcloud.io/api/project_badges/measure?project=philips-software_embeddedinfralib&metric=coverage)](https://sonarcloud.io/dashboard?id=philips-software_embeddedinfralib) [![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT)

**Description**: EmbeddedInfraLib is a set of C++ libraries and headers that provide heap-less, STL like, infrastructure for embedded software development.

## Dependencies

EmIL requires:
- A recent C++ compiler that supports C++17 at minimum (most notable it should support std::filesystem).
- CMake 3.6 or higher.

EmIL is know to build under the following configurations:
- Windows from Visual Studio 2015 onwards.
- Linux from GCC 5.3 onwards.
- OSX from XCode 11 and target platform 10.15 onwards.

## How to build the software

EmIL can be built by-itself, for example to execute the included micro-tests, or it can be built as part of a larger project. This paragraph describes how to build EmIL by-itself.

```
cmake -E make_directory Build
cd Build
cmake .. -DCCOLA_DIR=ccola -DCCOLA_INSTALL_DIR=Install
cmake --build .
```

## How to test the software

After EmIL has been built. The included automated tests can be run with CTest like so:

```
ctest -D Experimental -C Debug
```

## Code examples

Code examples can be found under the [examples](examples) folder.

## Documentation

Documentation is available under the [documents](documents) folder.

* [CMakeCcolaUserManual](documents/CMakeCcolaUserManual.docx) describes the CMake Component Layer (Ccola) from a user perspective.
* [Coding Standard C++ Embedded Projects](documents/Coding%20Standard%20C++%20Embedded%20Projects.docx) describes the coding standards adhered to for this project.
* [Containers](documents/Containers.md) describes the basic heap-less containers provided by EmIL.
* [Echo (Embedded CHOmmunication)](documents/Echo.md) describes the RPC (Remote Procedure Call) framework built on top of Google Protobuf.
* [MemoryRange](documents/MemoryRange.md) describes the basic building block for most of EmIL's heap-less memory management.

## Contributing

Please refer to our [Contributing](CONTRIBUTING.md) guide when you want to contribute to this project.

## License

EmbeddedInfraLib is licenced under the [MIT](https://opensource.org/licenses/MIT) license. See [LICENSE file](LICENSE.md).
