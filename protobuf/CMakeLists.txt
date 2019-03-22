ccola_package(protobuf)

	ccola_subdirectories(
		echo_attributes
		protoc_echo_plugin
		protoc_echo_plugin_csharp
		protoc_echo_plugin_java
		echo
		echo_console
	)

	ccola_pack_directory(protoc_echo)

	ccola_subdirectories(
		services
		protoc_echo_plugin/test
	)

ccola_end_package()
