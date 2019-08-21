ccola_component(syntax_test_doubles x86: source other: noncompile)

    ccola_sources(
        JsonStreamingParserMock.hpp
        JsonStringMatcher.hpp
    )

    ccola_dependencies(
        syntax
        gmock.gmock
    )

ccola_end_component()
