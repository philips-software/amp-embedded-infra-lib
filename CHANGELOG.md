<!-- markdownlint-disable MD024 -->

# amp-embedded-infra-lib Changelog

All notable changes to this project will be documented in this file.


The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)

## [5.0.0](https://github.com/philips-software/amp-embedded-infra-lib/compare/v4.0.0...v5.0.0) (2023-11-09)


### ⚠ BREAKING CHANGES

* generic pairing/bonding classes updated ([#330](https://github.com/philips-software/amp-embedded-infra-lib/issues/330))
* ble interface improvements ([#300](https://github.com/philips-software/amp-embedded-infra-lib/issues/300))

### Features

* Add protobuf/echo/ProtoMessageBuilder ([#416](https://github.com/philips-software/amp-embedded-infra-lib/issues/416)) ([f5260dd](https://github.com/philips-software/amp-embedded-infra-lib/commit/f5260dd65ae4adfea14755dfef04a18074441951))
* Add ServiceProxyResponseQueue ([#338](https://github.com/philips-software/amp-embedded-infra-lib/issues/338)) ([88cb197](https://github.com/philips-software/amp-embedded-infra-lib/commit/88cb197acb2f38d3b6fad00f88983cf15d49c864))
* Add streaming of little endian mac addresses ([#336](https://github.com/philips-software/amp-embedded-infra-lib/issues/336)) ([e3d7fd8](https://github.com/philips-software/amp-embedded-infra-lib/commit/e3d7fd8f12ad934b98a86a6b27d8f574049f12b3))
* Added SerialCommunicationLoopback ([#440](https://github.com/philips-software/amp-embedded-infra-lib/issues/440)) ([2c87dfb](https://github.com/philips-software/amp-embedded-infra-lib/commit/2c87dfb5ecd1e76669af69a97f80c29bfab773b9))
* Ble interface improvements ([#300](https://github.com/philips-software/amp-embedded-infra-lib/issues/300)) ([88a4689](https://github.com/philips-software/amp-embedded-infra-lib/commit/88a4689e8f08e2afe856170aa5b5036f69d1610c))
* Generic pairing/bonding classes updated ([#330](https://github.com/philips-software/amp-embedded-infra-lib/issues/330)) ([dbda405](https://github.com/philips-software/amp-embedded-infra-lib/commit/dbda405cc2ef2d356fbe707df5389042a49c7959))
* Removed Dac.hpp ([#439](https://github.com/philips-software/amp-embedded-infra-lib/issues/439)) ([4b52156](https://github.com/philips-software/amp-embedded-infra-lib/commit/4b5215609b9ee2a6ca72d8d660b8c797aa3e31c9))
* Support payloads bigger than the MaxSendStreamSize ([#331](https://github.com/philips-software/amp-embedded-infra-lib/issues/331)) ([c411f66](https://github.com/philips-software/amp-embedded-infra-lib/commit/c411f66f7ae972786a7fa1134989ab8e4f0631aa))
* Support streaming echo messages ([#435](https://github.com/philips-software/amp-embedded-infra-lib/issues/435)) ([07756bb](https://github.com/philips-software/amp-embedded-infra-lib/commit/07756bb41f068adbf3a2eb76adb2778fff5491f0))


### Bug Fixes

* Add acd sources to lwip ([#400](https://github.com/philips-software/amp-embedded-infra-lib/issues/400)) ([a4f68ef](https://github.com/philips-software/amp-embedded-infra-lib/commit/a4f68efdf19e7b556cb6fe2cba0165908f56be8c))
* Added workaround mbedtls and clang-cl incomatibility ([#441](https://github.com/philips-software/amp-embedded-infra-lib/issues/441)) ([063b595](https://github.com/philips-software/amp-embedded-infra-lib/commit/063b59508f1d09c2675b338c7b54d64934f7792c))
* Cpp:S6232 well-defined type-punning method should be used instead of a union-based one ([d809c17](https://github.com/philips-software/amp-embedded-infra-lib/commit/d809c173cacefd3bf2d661718111d5d301a95fc1))
* Destruction of HttpClientJson while resetting the reader resulted in memory corruption ([#404](https://github.com/philips-software/amp-embedded-infra-lib/issues/404)) ([ed8a8a0](https://github.com/philips-software/amp-embedded-infra-lib/commit/ed8a8a0e5893074fc56fde94cfdb003a870e930c))
* Detaching HttpClient while in HeaderAvailable resulted in a crash ([#406](https://github.com/philips-software/amp-embedded-infra-lib/issues/406)) ([a905a30](https://github.com/philips-software/amp-embedded-infra-lib/commit/a905a306be194c7cf6380e70cf83b668c8d27425))
* Enable service scoping of method invocations in services.echo_console ([#325](https://github.com/philips-software/amp-embedded-infra-lib/issues/325)) ([7090a40](https://github.com/philips-software/amp-embedded-infra-lib/commit/7090a407f50d89073c468381e5207f1c0f56b6c1))
* UpgradePackBuilderFacade setting mbedtls global memory pool on stack ([#324](https://github.com/philips-software/amp-embedded-infra-lib/issues/324)) ([ec567d5](https://github.com/philips-software/amp-embedded-infra-lib/commit/ec567d511f40eb3c0962ac4b6ac3479f64c0a871))
* UpgradePackBuilderFacade setting mbedtls global memory pool on stack, causing a crash when combined with ConnectionMbedTls ([ec567d5](https://github.com/philips-software/amp-embedded-infra-lib/commit/ec567d511f40eb3c0962ac4b6ac3479f64c0a871))

## [4.0.0](https://github.com/philips-software/amp-embedded-infra-lib/compare/v3.2.0...v4.0.0) (2023-06-02)


### ⚠ BREAKING CHANGES

* security for MessageCommunication ([#294](https://github.com/philips-software/amp-embedded-infra-lib/issues/294))

### Features

* Add documentation ([#223](https://github.com/philips-software/amp-embedded-infra-lib/issues/223)) ([d710807](https://github.com/philips-software/amp-embedded-infra-lib/commit/d7108073d4bc139899bbffd0f46de487d5f591ad))
* Merge upstream changes ([#279](https://github.com/philips-software/amp-embedded-infra-lib/issues/279)) ([9856f23](https://github.com/philips-software/amp-embedded-infra-lib/commit/9856f2304b818f69926db810aabadbd2c1a82659))
* Security for MessageCommunication ([#294](https://github.com/philips-software/amp-embedded-infra-lib/issues/294)) ([f6c3f56](https://github.com/philips-software/amp-embedded-infra-lib/commit/f6c3f56e73a8324f87476908a1ef07f243350093))


### Bug Fixes

* Correct order of destruction in EchoForwarderToSerial. ([#291](https://github.com/philips-software/amp-embedded-infra-lib/issues/291)) ([3267831](https://github.com/philips-software/amp-embedded-infra-lib/commit/32678313eb79ee84165c358cb392e5915323ab64))
* CucumberWireProtocolFormatter; max string value size does not fit into stepMatchArgumentsBuffer. ([#311](https://github.com/philips-software/amp-embedded-infra-lib/issues/311)) ([5504202](https://github.com/philips-software/amp-embedded-infra-lib/commit/55042025bfe5c631a2c247a6e90a23d0f437a2fb))
* Ensure forwarding constructors have constraints ([#290](https://github.com/philips-software/amp-embedded-infra-lib/issues/290)) ([5015938](https://github.com/philips-software/amp-embedded-infra-lib/commit/50159383787e330ea784b684da5f6dfc47b49113))
* GattClient notification ([#284](https://github.com/philips-software/amp-embedded-infra-lib/issues/284)) ([15ff387](https://github.com/philips-software/amp-embedded-infra-lib/commit/15ff3873f9f57169217c6a4337f9fac7d1b1f032))
* Line termination mismatch in hex files ([#307](https://github.com/philips-software/amp-embedded-infra-lib/issues/307)) ([47482c3](https://github.com/philips-software/amp-embedded-infra-lib/commit/47482c3b02e76d13437413cfb802e317439a4300))
* Remove superfluous &gt; from cmake/emil_xsltproc.cmake ([#276](https://github.com/philips-software/amp-embedded-infra-lib/issues/276)) ([be139b2](https://github.com/philips-software/amp-embedded-infra-lib/commit/be139b22aff7dd5998eba2d7c409302598f988fb))
* Usable memory loss in HttpClientBasic::Path ([#304](https://github.com/philips-software/amp-embedded-infra-lib/issues/304)) ([d5d4abf](https://github.com/philips-software/amp-embedded-infra-lib/commit/d5d4abf32050cf92bf7c55c5eb227007d04d88e3))

## [3.2.0](https://github.com/philips-software/amp-embedded-infra-lib/compare/v3.1.0...v3.2.0) (2023-04-21)


### Features

* Add websocket client example ([#235](https://github.com/philips-software/amp-embedded-infra-lib/issues/235)) ([d4aa174](https://github.com/philips-software/amp-embedded-infra-lib/commit/d4aa1740d7ccf2d4468d6afc0c54ed4d256636ce))
* Downgrade mbedtls to 3.2.1 from 3.3.0 ([#237](https://github.com/philips-software/amp-embedded-infra-lib/issues/237)) ([bdec832](https://github.com/philips-software/amp-embedded-infra-lib/commit/bdec83287eac3f7a27a9a559821584980633fcfc))
* Workaround OutputStream optimization issue with Visual Studio 2019 ([#258](https://github.com/philips-software/amp-embedded-infra-lib/issues/258)) ([ea3de1e](https://github.com/philips-software/amp-embedded-infra-lib/commit/ea3de1eeaf99649cdbf558cf11c650795662c2e9))


### Bug Fixes

* Decouple host and embedded builds ([#225](https://github.com/philips-software/amp-embedded-infra-lib/issues/225)) ([d443b16](https://github.com/philips-software/amp-embedded-infra-lib/commit/d443b165b0b2bc821735aa10e6c2fa392299f1fe))
* Protobuf/echo/EchoInstantiation: fix compilation errors ([#267](https://github.com/philips-software/amp-embedded-infra-lib/issues/267)) ([9b53dc1](https://github.com/philips-software/amp-embedded-infra-lib/commit/9b53dc1f77f944ce17d060a7d14fb5087f035677))

## [3.1.0](https://github.com/philips-software/amp-embedded-infra-lib/compare/v3.0.0...v3.1.0) (2023-03-23)


### Features

* **echo_console:** Add support for ECHO over websockets over http ([#230](https://github.com/philips-software/amp-embedded-infra-lib/issues/230)) ([b814f0e](https://github.com/philips-software/amp-embedded-infra-lib/commit/b814f0e6a85f2b5a3a70b01c114d2d9f5ef5d78d))
* **osal:** Add osal ([#82](https://github.com/philips-software/amp-embedded-infra-lib/issues/82)) ([3f5c568](https://github.com/philips-software/amp-embedded-infra-lib/commit/3f5c56868f63ea5d53da39caa63248b6536196b3))


### Bug Fixes

* Osx socket implementation ([#232](https://github.com/philips-software/amp-embedded-infra-lib/issues/232)) ([e127311](https://github.com/philips-software/amp-embedded-infra-lib/commit/e127311db5a33ab7110285794e2e728e3a3d4ed3))

## [3.0.0](https://github.com/philips-software/amp-embedded-infra-lib/compare/v2.2.1...v3.0.0) (2023-03-08)


### ⚠ BREAKING CHANGES

* add upgrade.pack_builder_instantations ([#190](https://github.com/philips-software/amp-embedded-infra-lib/issues/190))

### Features

* Add upgrade.pack_builder_instantations ([#190](https://github.com/philips-software/amp-embedded-infra-lib/issues/190)) ([5b6bf6f](https://github.com/philips-software/amp-embedded-infra-lib/commit/5b6bf6f0ebb6788c9a798a142aee3c15ddb25ee3))
* Compile on gcc windows ([#194](https://github.com/philips-software/amp-embedded-infra-lib/issues/194)) ([ab191f4](https://github.com/philips-software/amp-embedded-infra-lib/commit/ab191f4c830e7f534aeb7fd9991e112fd26d48c5))


### Bug Fixes

* Add check for availability of __has_cpp_attribute ([#218](https://github.com/philips-software/amp-embedded-infra-lib/issues/218)) ([98b5c8d](https://github.com/philips-software/amp-embedded-infra-lib/commit/98b5c8d89e7aab8819a4c2668be8aebe886cf303)), closes [#134](https://github.com/philips-software/amp-embedded-infra-lib/issues/134)
* Add missing overloads to JsonObjectFormatter ([#212](https://github.com/philips-software/amp-embedded-infra-lib/issues/212)) ([c4047c3](https://github.com/philips-software/amp-embedded-infra-lib/commit/c4047c3f974ba6e35bc5368dfee88ba6baf6ea9e))
* **external/protoc:** Download protoc for correct host architecture ([#214](https://github.com/philips-software/amp-embedded-infra-lib/issues/214)) ([6ed6bd1](https://github.com/philips-software/amp-embedded-infra-lib/commit/6ed6bd129cb33ce2627b8f5167eced74bc8656b3))
* **hal/windows:** Correct header sequence for UartPortFinder ([#209](https://github.com/philips-software/amp-embedded-infra-lib/issues/209)) ([d102e2b](https://github.com/philips-software/amp-embedded-infra-lib/commit/d102e2b5a732a9aa53ca17a0244739fc768c45a5))

## [2.2.1](https://github.com/philips-software/amp-embedded-infra-lib/compare/v2.2.0...v2.2.1) (2023-01-25)


### Bug Fixes

* Use gh cli to publish release assets ([#184](https://github.com/philips-software/amp-embedded-infra-lib/issues/184)) ([9de4c45](https://github.com/philips-software/amp-embedded-infra-lib/commit/9de4c453be231feea5f973968ad908355d8c6e43))

## [2.2.0](https://github.com/philips-software/amp-embedded-infra-lib/compare/v2.1.0...v2.2.0) (2023-01-24)


### Features

* Add cmake/emil_folderize.cmake ([#165](https://github.com/philips-software/amp-embedded-infra-lib/issues/165)) ([b3705c9](https://github.com/philips-software/amp-embedded-infra-lib/commit/b3705c947648fa3f492af58cbbd136083f5f3cf0))
* Allow external lwip ([#164](https://github.com/philips-software/amp-embedded-infra-lib/issues/164)) ([05ab97a](https://github.com/philips-software/amp-embedded-infra-lib/commit/05ab97afd339828da10bb6bea225cf5a09327617))
* Apply clang-format to all files ([#171](https://github.com/philips-software/amp-embedded-infra-lib/issues/171)) ([61455f9](https://github.com/philips-software/amp-embedded-infra-lib/commit/61455f92e156ad557d8461aa8fdd86ede55a0726))
* Update mbedtls from 3.2.1 to 3.3.0 ([#183](https://github.com/philips-software/amp-embedded-infra-lib/issues/183)) ([e51d7db](https://github.com/philips-software/amp-embedded-infra-lib/commit/e51d7dbf63226b06959a5927722aa64074a1981b))


### Bug Fixes

* **.vscode/tasks.json:** Call correct clangformat target ([#178](https://github.com/philips-software/amp-embedded-infra-lib/issues/178)) ([a3427b3](https://github.com/philips-software/amp-embedded-infra-lib/commit/a3427b302c24354a267d0b1bec74f79b17ba3d2d))
* **protobuf:** More granular control over excluded warnings ([354e15e](https://github.com/philips-software/amp-embedded-infra-lib/commit/354e15e872674a64f69b8bf6556701cba3b33cfc))
* Release asset upload ([#150](https://github.com/philips-software/amp-embedded-infra-lib/issues/150)) ([76b5fd1](https://github.com/philips-software/amp-embedded-infra-lib/commit/76b5fd1cfd9f552abd14666f3c153c1116661c1d))
* Use EMIL_MAYBE_UNUSED ([3d9834d](https://github.com/philips-software/amp-embedded-infra-lib/commit/3d9834d0e334c9472e83b2468071db144488a744))
* Use EMIL_MAYBE_UNUSED in version file generation ([#177](https://github.com/philips-software/amp-embedded-infra-lib/issues/177)) ([3d9834d](https://github.com/philips-software/amp-embedded-infra-lib/commit/3d9834d0e334c9472e83b2468071db144488a744))

## [2.1.0](https://github.com/philips-software/amp-embedded-infra-lib/compare/v1.3.0...v2.1.0) (2022-12-21)


### Features

* Add clusterfuzzlite action ([#113](https://github.com/philips-software/amp-embedded-infra-lib/issues/113)) ([7be93ab](https://github.com/philips-software/amp-embedded-infra-lib/commit/7be93ab7efb3823d673c068f50cc3780a1aa4db5))
* Add debug support to devcontainer ([ab92693](https://github.com/philips-software/amp-embedded-infra-lib/commit/ab926932f7879c63cc91042e45965bc2e344b00e))
* Add fuzz testing ([#109](https://github.com/philips-software/amp-embedded-infra-lib/issues/109)) ([23061b9](https://github.com/philips-software/amp-embedded-infra-lib/commit/23061b9f0082f5a7851dad1a3c25cbfb32c68f9c))
* Add openssf best practices badge ([#112](https://github.com/philips-software/amp-embedded-infra-lib/issues/112)) ([41528bd](https://github.com/philips-software/amp-embedded-infra-lib/commit/41528bd06b01612ec33793ae49d4f17ed3d92808))
* Add release-please action ([#147](https://github.com/philips-software/amp-embedded-infra-lib/issues/147)) ([45fa1e9](https://github.com/philips-software/amp-embedded-infra-lib/commit/45fa1e9148a7aa2fe758410d48cd195e30e20335))
* Add rpc example ([#141](https://github.com/philips-software/amp-embedded-infra-lib/issues/141)) ([0e0d709](https://github.com/philips-software/amp-embedded-infra-lib/commit/0e0d709b7354c4050ea9bbfba95bf206f8f30450))
* Enable installer package generation ([#138](https://github.com/philips-software/amp-embedded-infra-lib/issues/138)) ([169dd81](https://github.com/philips-software/amp-embedded-infra-lib/commit/169dd81ac8623390b917606bcdae35990789c9d5))
* Export mutation testing result to Sonar ([#105](https://github.com/philips-software/amp-embedded-infra-lib/issues/105)) ([e7f22cd](https://github.com/philips-software/amp-embedded-infra-lib/commit/e7f22cd4e11c44bc981c5af26414d491d5e23f3f))
* Fix fuzzing builds ([#125](https://github.com/philips-software/amp-embedded-infra-lib/issues/125)) ([4e34bc3](https://github.com/philips-software/amp-embedded-infra-lib/commit/4e34bc3e4fcc3a6aedfa10914c912ec9f6555383))
* Fix sonar findings ([#107](https://github.com/philips-software/amp-embedded-infra-lib/issues/107)) ([b2d3935](https://github.com/philips-software/amp-embedded-infra-lib/commit/b2d3935a176c5bc48f09a60ac7237d6926be33b1))
* Fix Sonar findings ([#129](https://github.com/philips-software/amp-embedded-infra-lib/issues/129)) ([455c787](https://github.com/philips-software/amp-embedded-infra-lib/commit/455c787e2ba856d6264967b1af5231a0c6dd18be))
* Generalize examples ([#131](https://github.com/philips-software/amp-embedded-infra-lib/issues/131)) ([dfeb251](https://github.com/philips-software/amp-embedded-infra-lib/commit/dfeb251aba7350a8d6964af9e4b60c2ae175dbfa))
* Hal unix ([#136](https://github.com/philips-software/amp-embedded-infra-lib/issues/136)) ([57817da](https://github.com/philips-software/amp-embedded-infra-lib/commit/57817da690591a9ba31ace66f7e88d6ae2ab73a7))
* Increase Gatt test coverage ([77d297e](https://github.com/philips-software/amp-embedded-infra-lib/commit/77d297e11004b995eb0648a47b7b803da8c36f8c))
* **infra/util/Endian.hpp:** Add constexpr, and endianness detection ([1120b48](https://github.com/philips-software/amp-embedded-infra-lib/commit/1120b4815410bf855163cff7ccc2dc33f3fb2b79))
* Port echo console ([#135](https://github.com/philips-software/amp-embedded-infra-lib/issues/135)) ([c175bae](https://github.com/philips-software/amp-embedded-infra-lib/commit/c175bae6fec44c61039013ff494c6ddfcc740a37))
* Update devcontainer ([7ed3b33](https://github.com/philips-software/amp-embedded-infra-lib/commit/7ed3b33d1d5113004f8172f180b6828d44f8c24f))
* Update mbedtls to v3.2.1 ([4739af3](https://github.com/philips-software/amp-embedded-infra-lib/commit/4739af351fb2a71493608f895aca529bd40c2e98))
* Update protobuf to 3.21.9.0 (v21.9) ([#106](https://github.com/philips-software/amp-embedded-infra-lib/issues/106)) ([76f8e77](https://github.com/philips-software/amp-embedded-infra-lib/commit/76f8e775344923ce984588acd359e1dc33903d3b))


### Bug Fixes

* Convert services/ble/test CMakeLists from ccola to plain CMake ([8c22f65](https://github.com/philips-software/amp-embedded-infra-lib/commit/8c22f65d6d95de109bfa3a9ac12b0a0fffbdc0ec))
* **devcontainer:** Select correct base image version ([#114](https://github.com/philips-software/amp-embedded-infra-lib/issues/114)) ([041d64b](https://github.com/philips-software/amp-embedded-infra-lib/commit/041d64b00f5f35d8ff2c3550fe7b69f31adfe32a))
* Flag false positive of cpp:S836 ([01b2599](https://github.com/philips-software/amp-embedded-infra-lib/commit/01b2599075e8b5a1ee1c943260b62f4c7856deb5))
* Include(CTest) should be called from top-level ([5163094](https://github.com/philips-software/amp-embedded-infra-lib/commit/5163094409620dd6488c89e759e82f2533575035))
* Increase coverage ([587b940](https://github.com/philips-software/amp-embedded-infra-lib/commit/587b940b2722158b780623a17db7c9fc6c4feeb5))
* **protoc:** Propagate link libraries in proto deps ([d609e66](https://github.com/philips-software/amp-embedded-infra-lib/commit/d609e667faedb0c20639107b0bc565aca855c94a))
* Sonar cpp:S128 ([d7b4776](https://github.com/philips-software/amp-embedded-infra-lib/commit/d7b4776bf65140f29ff9d426fc92b1331df0d6c9))
* Sonar cpp:S3519 ([58bcfbd](https://github.com/philips-software/amp-embedded-infra-lib/commit/58bcfbd3380669647a1a62a31332fb28971fb1dd))
* Sonar cpp:S5018 ([291443f](https://github.com/philips-software/amp-embedded-infra-lib/commit/291443f743ce58607956cdf2fbab232792bc81ac))
* Sonar cpp:S912 ([3063057](https://github.com/philips-software/amp-embedded-infra-lib/commit/30630579ab98341fc4f3523fed77c398b89a696e))
* Sonar cpp:S3491 ([bfc763e](https://github.com/philips-software/amp-embedded-infra-lib/commit/bfc763e8ccc09b7556d4482901e94dff357fb2e1))
* **TestJson:** Warning on signed overflow ([45d7ef2](https://github.com/philips-software/amp-embedded-infra-lib/commit/45d7ef2a5d699e9ccb8ff0881fe67562ebafdcda))

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

- Initial open source version of amp-embedded-infra-lib
