# EmbeddedInfraLib Changelog

All notable changes to this project will be documented in this file.


The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)

## [Unreleased]

## [1.3.0]

### Changed

- hal/interfaces/AnalogToDigitalPin; increased ADC results to int32_t
- services/network/Multicast.hpp; added DatagramExchange to interface, on BSD/WinSock implementations we need the actual socket to join a multicast group

### Fixed

- services/network/ConnectionMbedTls; On CloseAndDestroy, empty buffers before forwarding close request
- services/util/Terminal; fixed crash by using services::Tracer to write data to the console, synchronously

### Added

- .travis-ci.yml; Travis-CI build pipeline
- .gitattributes; Ignore external vendor packages in language analysis on GitHub
- https_client example
- services::HttpClientJson; base class to easily interact with REST services exposing large payloads. Uses infra::JsonStreamingObjectParser
- services/network/ExclusiveConnection; only allow one active connection at a time
- protobuf/protoc_echo_plugin: Support enums
- hal::UartWindows; hal::SerialCommunication implementation for Windows serial port
- services::SerialServer; TCP/IP <--> UART bridge, see [serial_net](examples/serial_net) example
- services::WebSocketClientConnectionObserver; WebSocket client support
- services::WebSocketServerConnectionObserver; WebSocket server support
- Support for Linux host builds with GCC

## [1.2.0]

- Initial open source version of EmbeddedInfraLib

[Unreleased]: https://github.com/philips-software/embeddedinfralib/compare/v1.3.0...HEAD
[1.3.0]: https://github.com/philips-software/embeddedinfralib/compare/v1.2.0...v1.3.0
[1.2.0]: https://github.com/philips-software/embeddedinfralib/releases/tag/v1.2.0
