
add_library(infra.syntax_test_doubles EXCLUDE_FROM_ALL STATIC)

target_link_libraries(infra.syntax_test_doubles
    PUBLIC
    infra.syntax
    gmock
)

target_sources(infra.syntax_test_doubles
    PRIVATE
    JsonStreamingParserMock.hpp
    JsonStringMatcher.hpp
)
