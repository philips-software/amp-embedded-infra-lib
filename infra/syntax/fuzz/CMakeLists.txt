emil_add_fuzzing_executable(infra.syntax_json_fuzzer)

target_link_libraries(infra.syntax_json_fuzzer PUBLIC
    infra.syntax
)

target_sources(infra.syntax_json_fuzzer PRIVATE
    FuzzJson.cpp
)
