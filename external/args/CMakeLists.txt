ccola_package(arg_parser)

    ccola_component(args x86: source other: noncompile)

        ccola_sources(
            args.hxx
        )

    ccola_end_component()

ccola_end_package()
