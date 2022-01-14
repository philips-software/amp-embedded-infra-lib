if(NOT IS_ABSOLUTE "${input}")
    set(input "${list_dir}/${input}")
endif()

file(READ "${input}" contents HEX)
string(LENGTH "${contents}" contentsLength)

string(REGEX REPLACE "(..)" "'\\\\x\\1', " contents "${contents}")

configure_file("${script_dir}/transform_file_to_string_template.cpp.conf" generated/${namespace}/${output}.cpp)
configure_file("${script_dir}/transform_file_to_string_template.hpp.conf" generated/${namespace}/${output}.hpp)
