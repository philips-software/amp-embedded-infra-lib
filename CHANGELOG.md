<!-- markdownlint-disable MD024 -->

# amp-embedded-infra-lib Changelog

All notable changes to this project will be documented in this file.


The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)

## [7.0.0](https://github.com/philips-software/amp-embedded-infra-lib/compare/v6.1.0...v7.0.0) (2024-10-24)


### ⚠ BREAKING CHANGES

* sesame stack which does not execute in interrupt context ([#514](https://github.com/philips-software/amp-embedded-infra-lib/issues/514))

### Features

* Add 256bit ecdsa key support for upgrade pack builder ([#657](https://github.com/philips-software/amp-embedded-infra-lib/issues/657)) ([a72b4d4](https://github.com/philips-software/amp-embedded-infra-lib/commit/a72b4d4a85df0a207675c64a09c630d59a26b20c))
* Add custom clang-format file support ([#668](https://github.com/philips-software/amp-embedded-infra-lib/issues/668)) ([3653875](https://github.com/philips-software/amp-embedded-infra-lib/commit/3653875fa54af2f6c5f610e07177a044821c3b86))
* Add NewConnectionStrategy to SingleConnectionListener ([#747](https://github.com/philips-software/amp-embedded-infra-lib/issues/747)) ([66e4f4a](https://github.com/philips-software/amp-embedded-infra-lib/commit/66e4f4ad7b6b5171c710bea647223db5f7b85f08))
* Add NotifyingSharedOptional::OnAllocatable overload ([#667](https://github.com/philips-software/amp-embedded-infra-lib/issues/667)) ([cb37f67](https://github.com/philips-software/amp-embedded-infra-lib/commit/cb37f67e787ef608dfe7f60aa5df67b05aa71508))
* Add osal/threadx/EventDispatcherThreadX ([#637](https://github.com/philips-software/amp-embedded-infra-lib/issues/637)) ([8c66ddd](https://github.com/philips-software/amp-embedded-infra-lib/commit/8c66dddb6d6efa5d25d097f5c7ffbc74b8362dc9))
* Add selection of TCP echo port to echo_console ([#748](https://github.com/philips-software/amp-embedded-infra-lib/issues/748)) ([3d2622f](https://github.com/philips-software/amp-embedded-infra-lib/commit/3d2622f3674a7d390fdda98fe33852e22cb21f7d))
* Added EcDsa256 bit support for sign and verify ([#651](https://github.com/philips-software/amp-embedded-infra-lib/issues/651)) ([bc39360](https://github.com/philips-software/amp-embedded-infra-lib/commit/bc39360c56bf232e5ac8a951323e2bacd9f34189))
* Added infra::MockFunction helper function for testing callbacks via infra::Function ([#658](https://github.com/philips-software/amp-embedded-infra-lib/issues/658)) ([49810ee](https://github.com/philips-software/amp-embedded-infra-lib/commit/49810eef4d70e477dc8d96b947263d621fe47bda))
* Crc with automatic table generation ([#636](https://github.com/philips-software/amp-embedded-infra-lib/issues/636)) ([23ac162](https://github.com/philips-software/amp-embedded-infra-lib/commit/23ac162fbd3be451bd0b6a3d4488e6c8b9d38b1b))
* Enable SHA384 and SHA512 on mbedtls ([#745](https://github.com/philips-software/amp-embedded-infra-lib/issues/745)) ([2c0351e](https://github.com/philips-software/amp-embedded-infra-lib/commit/2c0351e4c5ce4f212ed1179ee53d52f555c1a13d))
* Extend gpio interrupt interface to enable immediate interrupt handlers ([#721](https://github.com/philips-software/amp-embedded-infra-lib/issues/721)) ([95ddf85](https://github.com/philips-software/amp-embedded-infra-lib/commit/95ddf856192b8eb7e7282f3cb6f4e028aef200ad))
* Gap proto extension before sesame ([#688](https://github.com/philips-software/amp-embedded-infra-lib/issues/688)) ([d5efa72](https://github.com/philips-software/amp-embedded-infra-lib/commit/d5efa729fd51f7c734e6ee751eb255f4acfc8246))
* Generate string literals ([#699](https://github.com/philips-software/amp-embedded-infra-lib/issues/699)) ([d272b1f](https://github.com/philips-software/amp-embedded-infra-lib/commit/d272b1f8774435755ecb8e0e51f8a1873612b692))
* HttpClientBasic will now reset the contentError flag before a new connection is attempted ([#598](https://github.com/philips-software/amp-embedded-infra-lib/issues/598)) ([c0e6755](https://github.com/philips-software/amp-embedded-infra-lib/commit/c0e67552bc328b1947f44ed0370cea6820c0512d))
* Implement persisted mbed tls session ([#602](https://github.com/philips-software/amp-embedded-infra-lib/issues/602)) ([53d45ce](https://github.com/philips-software/amp-embedded-infra-lib/commit/53d45ce8006c6a6e504ee90266feaa8b3ad235bc))
* Make I2cAddress constexpr ([#740](https://github.com/philips-software/amp-embedded-infra-lib/issues/740)) ([ca31705](https://github.com/philips-software/amp-embedded-infra-lib/commit/ca31705a21937ceb07a654d8828eb8f982f468e1))
* Protobuf: add support for optional keyword ([#661](https://github.com/philips-software/amp-embedded-infra-lib/issues/661)) ([82032aa](https://github.com/philips-software/amp-embedded-infra-lib/commit/82032aaeceed20d8b5fd588456e9b600da7a8263))
* Protobuf: add support for optional keyword ([#665](https://github.com/philips-software/amp-embedded-infra-lib/issues/665)) ([b33ed06](https://github.com/philips-software/amp-embedded-infra-lib/commit/b33ed0647d149cb6af81bcbe9daeeb6c4c6e92ab))
* Sesame stack which does not execute in interrupt context ([#514](https://github.com/philips-software/amp-embedded-infra-lib/issues/514)) ([4dc5736](https://github.com/philips-software/amp-embedded-infra-lib/commit/4dc5736b2814734a66a1de75d2eabb356d1ed6ff))


### Bug Fixes

* Echo: only ContinueReceiveMessage() when readerPtr != nullptr ([#701](https://github.com/philips-software/amp-embedded-infra-lib/issues/701)) ([81c87c4](https://github.com/philips-software/amp-embedded-infra-lib/commit/81c87c4926cbdb89830c4003277a1919a26a6cec))
* **echo:** Fix generation of tracer class for empty services ([#671](https://github.com/philips-software/amp-embedded-infra-lib/issues/671)) ([751fefd](https://github.com/philips-software/amp-embedded-infra-lib/commit/751fefdd673e364c492efcaa2bf892281f73c396))
* **echo:** Support serializing nested messages ([#670](https://github.com/philips-software/amp-embedded-infra-lib/issues/670)) ([63c55e0](https://github.com/philips-software/amp-embedded-infra-lib/commit/63c55e048888397a3b163762e919256e4fe1f2da))
* Fix move construct and move assign for Variant ([#749](https://github.com/philips-software/amp-embedded-infra-lib/issues/749)) ([ed39658](https://github.com/philips-software/amp-embedded-infra-lib/commit/ed3965834d98c8bb30fefeb594997bf9b5fec4cc))
* Pad 0xff when add hex and elf to upgrade pack ([#646](https://github.com/philips-software/amp-embedded-infra-lib/issues/646)) ([5c5d318](https://github.com/philips-software/amp-embedded-infra-lib/commit/5c5d3186c45e3463d57535a1d07cf011da91f6dc))
* Services.echo_console: parsing tokens of nested messages and arrays, formatting of messages, and properly close upon losing connection ([#712](https://github.com/philips-software/amp-embedded-infra-lib/issues/712)) ([4a69845](https://github.com/philips-software/amp-embedded-infra-lib/commit/4a69845e0b2ffa44a96b80a792db5a9b44e98742))
* Tcp packet fragmentation ([#700](https://github.com/philips-software/amp-embedded-infra-lib/issues/700)) ([c419e54](https://github.com/philips-software/amp-embedded-infra-lib/commit/c419e5473bdb46fa90bd7282614435f5ab511a10))
* Timer limited repeat ([#659](https://github.com/philips-software/amp-embedded-infra-lib/issues/659)) ([4f2a49e](https://github.com/philips-software/amp-embedded-infra-lib/commit/4f2a49ee501f358965bdb54737f666cf1bafb407))
* Trace outgoing echo calls ([#710](https://github.com/philips-software/amp-embedded-infra-lib/issues/710)) ([279bb62](https://github.com/philips-software/amp-embedded-infra-lib/commit/279bb6295d1d1c0d7fa9849e6a7ca04805ab84b0))
* UartUnix receiveData callback being called after destruction ([#677](https://github.com/philips-software/amp-embedded-infra-lib/issues/677)) ([65f164b](https://github.com/philips-software/amp-embedded-infra-lib/commit/65f164b480720379607497f61ad276f35d0474c2))

## [6.1.0](https://github.com/philips-software/amp-embedded-infra-lib/compare/v6.0.0...v6.1.0) (2024-05-01)


### Features

* Add connection factory with name resolver mbed tls ([#536](https://github.com/philips-software/amp-embedded-infra-lib/issues/536)) ([af200d2](https://github.com/philips-software/amp-embedded-infra-lib/commit/af200d2262340ed065ed36dade63ea9639210231))
* Add FlashHeterogenous ([#549](https://github.com/philips-software/amp-embedded-infra-lib/issues/549)) ([a0deadb](https://github.com/philips-software/amp-embedded-infra-lib/commit/a0deadb8bd6800b1cda2a1c28f3f4d2713b37c04))
* Add Segger RTT tracing output option ([#572](https://github.com/philips-software/amp-embedded-infra-lib/issues/572)) ([16529d7](https://github.com/philips-software/amp-embedded-infra-lib/commit/16529d773417547c2c08163eedf6fb8ff7035227))
* Changed compile definitions to be compatible with the gnu assembler ([#554](https://github.com/philips-software/amp-embedded-infra-lib/issues/554)) ([5a5e5dc](https://github.com/philips-software/amp-embedded-infra-lib/commit/5a5e5dca2cc522875b13f8a8ceebc73037dc0c94))
* Disabled documentation warning when compiling with AppleClang ([#618](https://github.com/philips-software/amp-embedded-infra-lib/issues/618)) ([5fd230a](https://github.com/philips-software/amp-embedded-infra-lib/commit/5fd230aa0c71787108fa7320dddc3478eee8b2ac))
* Services/network/HttpClientBasic is now able to process more than 1 request ([#576](https://github.com/philips-software/amp-embedded-infra-lib/issues/576)) ([0df2625](https://github.com/philips-software/amp-embedded-infra-lib/commit/0df26250e0e0894e7a3bb9af2d0306aa4d6cc312))


### Bug Fixes

* Avoid overflow when scheduling TimerSingleShot ([#612](https://github.com/philips-software/amp-embedded-infra-lib/issues/612)) ([ea4683c](https://github.com/philips-software/amp-embedded-infra-lib/commit/ea4683c464a2272936995bc89a9ff9bed506d1a7))
* Ble advertisement parser erroneous data handling ([#600](https://github.com/philips-software/amp-embedded-infra-lib/issues/600)) ([f6d6b5e](https://github.com/philips-software/amp-embedded-infra-lib/commit/f6d6b5efb7ee6bad5d13079310d40280803586c7))
* BufferingStreamReader OOB access and Echo segments calculated correctly ([#569](https://github.com/philips-software/amp-embedded-infra-lib/issues/569)) ([b1c1d12](https://github.com/philips-software/amp-embedded-infra-lib/commit/b1c1d12d8bb3414cfd52b6b295388c472c1f7f12))
* ConnectionMbedTls no longer calls DataReceived() with 0 bytes ([#585](https://github.com/philips-software/amp-embedded-infra-lib/issues/585)) ([cd74d66](https://github.com/philips-software/amp-embedded-infra-lib/commit/cd74d664717066664cbb99cc7619a7919dffbcac))
* Duplicate header guard HAL_UART_HOST_HPP ([#558](https://github.com/philips-software/amp-embedded-infra-lib/issues/558)) ([a8b1e63](https://github.com/philips-software/amp-embedded-infra-lib/commit/a8b1e639d42718fab274590a38f548ffe02270c0))
* Heap memory leak on SerializerFactory::OnHeap ([#561](https://github.com/philips-software/amp-embedded-infra-lib/issues/561)) ([0ed606b](https://github.com/philips-software/amp-embedded-infra-lib/commit/0ed606bb6fdb925e1511314d4f3b25ab19632a18))
* Networking on linux ([#594](https://github.com/philips-software/amp-embedded-infra-lib/issues/594)) ([972ff81](https://github.com/philips-software/amp-embedded-infra-lib/commit/972ff816320440f3ee837d14c50b8c8aa6a516d6))
* Repeated proto message incorrect maxMessageSize calculation ([#617](https://github.com/philips-software/amp-embedded-infra-lib/issues/617)) ([2d28a51](https://github.com/philips-software/amp-embedded-infra-lib/commit/2d28a51a94ab9f41bd776f16db47ed3ad8335e2c))
* Services/network/ConnectionMbedTls: Hostname() may not be invoked after ConnectionEstablished() ([#609](https://github.com/philips-software/amp-embedded-infra-lib/issues/609)) ([fcc469a](https://github.com/philips-software/amp-embedded-infra-lib/commit/fcc469aa899cac1fd712016cf8a0c05a89e27637))
* Services/network/WebSocketClientConnectionObserver: Don't close websocket connection after 1 minute ([#578](https://github.com/philips-software/amp-embedded-infra-lib/issues/578)) ([d2e0c05](https://github.com/philips-software/amp-embedded-infra-lib/commit/d2e0c05d916aeeb88551e42306161ce372bf1aa1))
* Timer jumped shall trigger next trigger time update ([9bbfb53](https://github.com/philips-software/amp-embedded-infra-lib/commit/9bbfb535061a56e76cf9e1aa037269030efff144))
* Timer jumped will trigger next trigger time update ([#590](https://github.com/philips-software/amp-embedded-infra-lib/issues/590)) ([9bbfb53](https://github.com/philips-software/amp-embedded-infra-lib/commit/9bbfb535061a56e76cf9e1aa037269030efff144))

## [6.0.0](https://github.com/philips-software/amp-embedded-infra-lib/compare/v5.0.1...v6.0.0) (2024-01-24)


### ⚠ BREAKING CHANGES

* refactor adc to support multiple samples ([#530](https://github.com/philips-software/amp-embedded-infra-lib/issues/530))

### Features

* Add ExecuteUntil to event dispatchers, add EventDispatcherThreadAware ([#526](https://github.com/philips-software/amp-embedded-infra-lib/issues/526)) ([d05d1f3](https://github.com/philips-software/amp-embedded-infra-lib/commit/d05d1f3fb8272ebe48407cfa42cfe9c938683106))
* Add targets in order upgrade pack ([#488](https://github.com/philips-software/amp-embedded-infra-lib/issues/488)) ([e528794](https://github.com/philips-software/amp-embedded-infra-lib/commit/e528794ef639b5884ade70aa79f6048c3a8ccf58))
* Added generic gatt dis characteristic uuid ([#457](https://github.com/philips-software/amp-embedded-infra-lib/issues/457)) ([be6fdde](https://github.com/philips-software/amp-embedded-infra-lib/commit/be6fddea702f47ef816fe4fc976cdd609c1fbe87))
* Added hal/generic/UartGeneric and hal/generic/SynchronousUartGeneric ([#535](https://github.com/philips-software/amp-embedded-infra-lib/issues/535)) ([799e0ee](https://github.com/philips-software/amp-embedded-infra-lib/commit/799e0eeae498521471a48724fee54564ecb295d5))
* Changed EMIL_ENABLE_GLOBAL_TRACING define to EMIL_ENABLE_TRACING and EMIL_DISABLE_TRACING ([#521](https://github.com/philips-software/amp-embedded-infra-lib/issues/521)) ([2d4eb48](https://github.com/philips-software/amp-embedded-infra-lib/commit/2d4eb489817fc36b35ff758bf4d9acbc3e7ceda5))
* Forward tags to scenario request handler ([#483](https://github.com/philips-software/amp-embedded-infra-lib/issues/483)) ([a66f711](https://github.com/philips-software/amp-embedded-infra-lib/commit/a66f711cd582fa099fe92db0d4995d2f052aba19))
* Made EventDispatcher and EventDispatcherWithWeakptr exception safe when executing an action ([#525](https://github.com/philips-software/amp-embedded-infra-lib/issues/525)) ([46a118c](https://github.com/philips-software/amp-embedded-infra-lib/commit/46a118c9d792981f282abcf8bb980bc754aa152b))
* Optional tracing ([#489](https://github.com/philips-software/amp-embedded-infra-lib/issues/489)) ([0c594d9](https://github.com/philips-software/amp-embedded-infra-lib/commit/0c594d954fa1e5d9458d0dbbbd8f186da735793a))
* Refactor adc to support multiple samples ([#530](https://github.com/philips-software/amp-embedded-infra-lib/issues/530)) ([419f636](https://github.com/philips-software/amp-embedded-infra-lib/commit/419f636bd76f2c0ee2f7524ca548e724bf479e27))


### Bug Fixes

* Handle echo message split over multiple streams ([#481](https://github.com/philips-software/amp-embedded-infra-lib/issues/481)) ([37d7878](https://github.com/philips-software/amp-embedded-infra-lib/commit/37d7878f709090e9d556e9b84c7bf6eeafa1c6e0))
* HttpClientBasic reporting error for reestablished connections ([#473](https://github.com/philips-software/amp-embedded-infra-lib/issues/473)) ([0619299](https://github.com/philips-software/amp-embedded-infra-lib/commit/0619299fb6330cd7cee92ac952fad229317f64f3))
* Incorrect handling of float numbers in json ([#478](https://github.com/philips-software/amp-embedded-infra-lib/issues/478)) ([c8ee456](https://github.com/philips-software/amp-embedded-infra-lib/commit/c8ee456e6936a76a1e7cc3ab18198b7def9d2834))
* Infra/util/Endian: fix comparison against C++ version ([#486](https://github.com/philips-software/amp-embedded-infra-lib/issues/486)) ([4fe74e5](https://github.com/philips-software/amp-embedded-infra-lib/commit/4fe74e5751c90eca2786d263cbf6bca1520a4aba))
* Null json value ([#470](https://github.com/philips-software/amp-embedded-infra-lib/issues/470)) ([e72d48f](https://github.com/philips-software/amp-embedded-infra-lib/commit/e72d48fd105cd8b9fd2cfe5d780a91e4591f89bd))
* Remove \r from end of line of certificates ([#510](https://github.com/philips-software/amp-embedded-infra-lib/issues/510)) ([7f581d8](https://github.com/philips-software/amp-embedded-infra-lib/commit/7f581d87f7eab9d0b4635c25886d9b100cac22a3))
* Remove virtual destructors reducing binary size ([#502](https://github.com/philips-software/amp-embedded-infra-lib/issues/502)) ([fc69a3c](https://github.com/philips-software/amp-embedded-infra-lib/commit/fc69a3c85b2a89018d4c9503378737a967912b31))
* ServiceForwarder with LimitedStreamReader ends up in infinite loop ([#524](https://github.com/philips-software/amp-embedded-infra-lib/issues/524)) ([9e7f949](https://github.com/philips-software/amp-embedded-infra-lib/commit/9e7f949e96493a2dd4240995b535b3c23d455e51))

## [5.0.1](https://github.com/philips-software/amp-embedded-infra-lib/compare/v5.0.0...v5.0.1) (2023-11-15)


### Bug Fixes

* Corrected incorrect destructor syntax ([#460](https://github.com/philips-software/amp-embedded-infra-lib/issues/460)) ([e01dd98](https://github.com/philips-software/amp-embedded-infra-lib/commit/e01dd985d0d4699dc859d3e8fc375c72dccf8a89))
* Various fixes for echo  ([#462](https://github.com/philips-software/amp-embedded-infra-lib/issues/462)) ([0062598](https://github.com/philips-software/amp-embedded-infra-lib/commit/006259893b8bd57b7c0796977cd96fc4bfbcc626))

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
