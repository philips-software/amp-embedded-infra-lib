# EmbeddedInfraLib (EmIL)

**Description**: EmbeddedInfraLib is a set of C++ libraries and headers that provide heap-less, STL like, infrastructure for embedded software development.

## Dependencies

EmIL requires:
- A recent c++ compiler that supports C++11 at minimum.
- CMake 3.6 or higher.

## How to build the software

EmIL can be built by-itself, for example to execute the included micro-tests, or it can be built as part of a larger project. This paragraph describes how to build EmIL by-itself.

```
cmake -E make_directory Build
cd Build
cmake .. -DCCOLA_DIR=ccola -DCCOLA_INSTALL_DIR=Install
cd ..
cmake --build Build
```

## How to test the software

After EmIL has been built. The included automated tests can be run with one the following commands, depending on the generator used.

When generating for Visual Studio:
```
cmake --build Build --target RUN_TESTS
```

When generating for Ninja:
```
cmake --build Build --target test
```

## Code examples

Code examples can be found under the [examples] folder.

## Documentation

Documentation is available under the documents folder.

* [CMakeCcolaUserManual] describes the CMake Component Layer (Ccola) from a user perspective.
* [Coding Standard C++ Embedded Projects] describes to coding standards adhered to for this project.
* [Containers] describes the basic heap-less containers provided by EmIL.
* [Echo (Embedded CHOmmunication)] describes the RPC (Remote Procedure Call) framework built on top of Google Protobuf.
* [MemoryRange] describes the basic building block for most of EmIL's heap-less memory management.

## Known issues

At the moment only Windows is supported as the host platform.

## Contributing

Please refer to our [Contributing] guide when you want to contribute to this project.

## License

License is MIT. See [LICENSE file](LICENSE.md)

## Links

[examples]: examples
[CMakeCcolaUserManual]: documents/CMakeCcolaUserManual.docx
[Coding Standard C++ Embedded Projects]: documents/Coding%20Standard%20C++%20Embedded%20Projects.docx
[Containers]: documents/Containers.md
[Echo (Embedded CHOmmunication)]: documents/Echo.md
[MemoryRange]: documents/MemoryRange.md
[Contributing]: CONTRIBUTING.md
[License File]: LICENSE.md
