ccola_package(upgrade)

	ccola_pack_directory(pack_builder_pack)

	ccola_subdirectories(
		pack
		pack_builder
		security_key_generator
		boot_loader
		deploy_pack_to_external
	)

ccola_end_package()
