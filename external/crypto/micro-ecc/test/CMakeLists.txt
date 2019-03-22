ccola_component(micro_ecc_test_compress executable)

	ccola_sources(
		test_compress.c
	)

	ccola_dependencies(
		micro_ecc
	)

ccola_end_component()

ccola_component(micro_ecc_test_compute executable)

	ccola_sources(
		test_compute.c
	)

	ccola_dependencies(
		micro_ecc
	)

ccola_end_component()

ccola_component(micro_ecc_test_ecdh executable)

	ccola_sources(
		test_ecdh.c
	)

	ccola_dependencies(
		micro_ecc
	)

ccola_end_component()

ccola_component(micro_ecc_test_ecdsa executable)

	ccola_sources(
		test_ecdsa.c
	)

	ccola_dependencies(
		micro_ecc
	)

ccola_end_component()
