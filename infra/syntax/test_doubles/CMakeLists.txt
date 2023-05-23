add_library(infra.syntax_test_doubles INTERFACE)
emil_build_for(infra.syntax_test_doubles BOOL BUILD_TESTING)

target_link_libraries(infra.syntax_test_doubles INTERFACE
    infra.syntax
    gmock
)

target_sources(infra.syntax_test_doubles PRIVATE
    JsonStreamingParserMock.hpp
    JsonStringMatcher.hpp
)
