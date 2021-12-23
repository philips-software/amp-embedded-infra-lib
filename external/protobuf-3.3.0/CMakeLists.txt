set(PROTOBUF_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "protobuf-3.3.0 directory")

if (TARGET_ARCH_X86)
    add_library(protobuf.libprotobuf_lite EXCLUDE_FROM_ALL STATIC)
    add_library(emil::protobuf.libprotobuf_lite ALIAS protobuf.libprotobuf_lite)

    target_compile_definitions(protobuf.libprotobuf_lite
        PUBLIC
        PROTOBUF_USE_DLLS
        $<$<BOOL:${TARGET_BUILD_WIN}>:_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING>

        PRIVATE
        LIBPROTOBUF_EXPORTS
        $<$<OR:$<BOOL:${TARGET_BUILD_UNIX}>,$<BOOL:${TARGET_BUILD_DARWIN}>>:HAVE_PTHREAD>
    )

    target_compile_options(protobuf.libprotobuf_lite PRIVATE
        $<$<BOOL:${TARGET_BUILD_WIN}>:
            $<$<CXX_COMPILER_ID:MSVC>:
                /wd4018
                /wd4065
                /wd4244
                /wd4251
                /wd4267
                /wd4305
                /wd4309
                /wd4506
                /wd4661
                /wd4800
                /wd4838
                /wd4996
            >
        >

        $<$<OR:$<BOOL:${TARGET_BUILD_UNIX}>,$<BOOL:${TARGET_BUILD_DARWIN}>>:-fPIC>
    )

    target_link_libraries(protobuf.libprotobuf_lite PUBLIC
        $<$<OR:$<BOOL:${TARGET_BUILD_UNIX}>,$<BOOL:${TARGET_BUILD_DARWIN}>>:pthread>
    )

    target_include_directories(protobuf.libprotobuf_lite PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

    target_sources(protobuf.libprotobuf_lite PRIVATE
        src/google/protobuf/arena.cc
        src/google/protobuf/arena.h
        # src/google/protobuf/arenastring.cc
        src/google/protobuf/arenastring.h
        src/google/protobuf/extension_set.cc
        src/google/protobuf/extension_set.h
        src/google/protobuf/generated_message_util.cc
        src/google/protobuf/generated_message_util.h
        src/google/protobuf/io/coded_stream.cc
        src/google/protobuf/io/coded_stream.h
        src/google/protobuf/io/zero_copy_stream.cc
        src/google/protobuf/io/zero_copy_stream.h
        src/google/protobuf/io/zero_copy_stream_impl_lite.cc
        src/google/protobuf/io/zero_copy_stream_impl_lite.h
        src/google/protobuf/message_lite.cc
        src/google/protobuf/message_lite.h
        src/google/protobuf/repeated_field.cc
        src/google/protobuf/repeated_field.h
        src/google/protobuf/stubs/bytestream.cc
        src/google/protobuf/stubs/bytestream.h
        src/google/protobuf/stubs/common.cc
        src/google/protobuf/stubs/common.h
        src/google/protobuf/stubs/int128.cc
        src/google/protobuf/stubs/int128.h
        src/google/protobuf/stubs/once.cc
        src/google/protobuf/stubs/once.h
        src/google/protobuf/stubs/status.cc
        src/google/protobuf/stubs/status.h
        src/google/protobuf/stubs/statusor.cc
        src/google/protobuf/stubs/statusor.h
        src/google/protobuf/stubs/stringpiece.cc
        src/google/protobuf/stubs/stringpiece.h
        src/google/protobuf/stubs/stringprintf.cc
        src/google/protobuf/stubs/stringprintf.h
        src/google/protobuf/stubs/structurally_valid.cc
        src/google/protobuf/stubs/strutil.cc
        src/google/protobuf/stubs/strutil.h
        src/google/protobuf/stubs/time.cc
        src/google/protobuf/stubs/time.h
        src/google/protobuf/wire_format_lite.cc
        src/google/protobuf/wire_format_lite.h
    )

    if (TARGET_BUILD_UNIX OR TARGET_BUILD_DARWIN)
        target_sources(protobuf.libprotobuf_lite PRIVATE
            src/google/protobuf/stubs/atomicops_internals_x86_gcc.cc
            src/google/protobuf/stubs/atomicops_internals_x86_gcc.h
        )
    endif()

    if (TARGET_BUILD_WIN)
        target_sources(protobuf.libprotobuf_lite PRIVATE
            src/google/protobuf/stubs/atomicops_internals_x86_msvc.cc
            src/google/protobuf/stubs/atomicops_internals_x86_msvc.h
        )
    endif()

    add_library(protobuf.libprotobuf EXCLUDE_FROM_ALL SHARED)
    add_library(emil::protobuf.libprotobuf ALIAS protobuf.libprotobuf)

    target_compile_options(protobuf.libprotobuf PRIVATE
        $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-Wno-enum-compare-switch>
        $<$<BOOL:${TARGET_BUILD_WIN}>:
            $<$<CXX_COMPILER_ID:MSVC>:
                /wd4018
                /wd4065
                /wd4244
                /wd4251
                /wd4267
                /wd4305
                /wd4309
                /wd4506
                /wd4661
                /wd4800
                /wd4838
                /wd4996
            >
        >
    )

    target_compile_definitions(protobuf.libprotobuf PRIVATE
        LIBPROTOBUF_EXPORTS
    )

    target_link_libraries(protobuf.libprotobuf PUBLIC
        protobuf.libprotobuf_lite
    )

    target_sources(protobuf.libprotobuf PRIVATE
        src/google/protobuf/any.cc
        src/google/protobuf/any.h
        src/google/protobuf/any.pb.cc
        src/google/protobuf/any.pb.h
        src/google/protobuf/api.pb.cc
        src/google/protobuf/api.pb.h
        src/google/protobuf/compiler/importer.cc
        src/google/protobuf/compiler/importer.h
        src/google/protobuf/compiler/parser.cc
        src/google/protobuf/compiler/parser.h
        src/google/protobuf/descriptor.cc
        src/google/protobuf/descriptor.h
        src/google/protobuf/descriptor.pb.cc
        src/google/protobuf/descriptor.pb.h
        src/google/protobuf/descriptor_database.cc
        src/google/protobuf/descriptor_database.h
        src/google/protobuf/duration.pb.cc
        src/google/protobuf/duration.pb.h
        src/google/protobuf/dynamic_message.cc
        src/google/protobuf/dynamic_message.h
        src/google/protobuf/empty.pb.cc
        src/google/protobuf/empty.pb.h
        src/google/protobuf/extension_set_heavy.cc
        src/google/protobuf/field_mask.pb.cc
        src/google/protobuf/field_mask.pb.h
        src/google/protobuf/generated_message_reflection.cc
        src/google/protobuf/generated_message_reflection.h
        src/google/protobuf/generated_message_table_driven.cc
        src/google/protobuf/generated_message_table_driven.h
        src/google/protobuf/io/gzip_stream.cc
        src/google/protobuf/io/gzip_stream.h
        src/google/protobuf/io/printer.cc
        src/google/protobuf/io/printer.h
        src/google/protobuf/io/strtod.cc
        src/google/protobuf/io/strtod.h
        src/google/protobuf/io/tokenizer.cc
        src/google/protobuf/io/tokenizer.h
        src/google/protobuf/io/zero_copy_stream_impl.cc
        src/google/protobuf/io/zero_copy_stream_impl.h
        src/google/protobuf/map_field.cc
        src/google/protobuf/map_field.h
        src/google/protobuf/message.cc
        src/google/protobuf/message.h
        src/google/protobuf/reflection_ops.cc
        src/google/protobuf/reflection_ops.h
        src/google/protobuf/service.cc
        src/google/protobuf/service.h
        src/google/protobuf/source_context.pb.cc
        src/google/protobuf/source_context.pb.h
        src/google/protobuf/struct.pb.cc
        src/google/protobuf/struct.pb.h
        src/google/protobuf/stubs/mathlimits.cc
        src/google/protobuf/stubs/mathlimits.h
        src/google/protobuf/stubs/substitute.cc
        src/google/protobuf/stubs/substitute.h
        src/google/protobuf/text_format.cc
        src/google/protobuf/text_format.h
        src/google/protobuf/timestamp.pb.cc
        src/google/protobuf/timestamp.pb.h
        src/google/protobuf/type.pb.cc
        src/google/protobuf/type.pb.h
        src/google/protobuf/unknown_field_set.cc
        src/google/protobuf/unknown_field_set.h
        src/google/protobuf/util/delimited_message_util.cc
        src/google/protobuf/util/delimited_message_util.h
        src/google/protobuf/util/field_comparator.cc
        src/google/protobuf/util/field_comparator.h
        src/google/protobuf/util/field_mask_util.cc
        src/google/protobuf/util/field_mask_util.h
        src/google/protobuf/util/internal/datapiece.cc
        src/google/protobuf/util/internal/datapiece.h
        src/google/protobuf/util/internal/default_value_objectwriter.cc
        src/google/protobuf/util/internal/default_value_objectwriter.h
        # src/google/protobuf/util/internal/error_listener.cc
        src/google/protobuf/util/internal/error_listener.h
        src/google/protobuf/util/internal/field_mask_utility.cc
        src/google/protobuf/util/internal/field_mask_utility.h
        src/google/protobuf/util/internal/json_escaping.cc
        src/google/protobuf/util/internal/json_escaping.h
        src/google/protobuf/util/internal/json_objectwriter.cc
        src/google/protobuf/util/internal/json_objectwriter.h
        src/google/protobuf/util/internal/json_stream_parser.cc
        src/google/protobuf/util/internal/json_stream_parser.h
        src/google/protobuf/util/internal/object_writer.cc
        src/google/protobuf/util/internal/object_writer.h
        src/google/protobuf/util/internal/proto_writer.cc
        src/google/protobuf/util/internal/proto_writer.h
        src/google/protobuf/util/internal/protostream_objectsource.cc
        src/google/protobuf/util/internal/protostream_objectsource.h
        src/google/protobuf/util/internal/protostream_objectwriter.cc
        src/google/protobuf/util/internal/protostream_objectwriter.h
        src/google/protobuf/util/internal/type_info.cc
        src/google/protobuf/util/internal/type_info.h
        src/google/protobuf/util/internal/type_info_test_helper.cc
        src/google/protobuf/util/internal/type_info_test_helper.h
        src/google/protobuf/util/internal/utility.cc
        src/google/protobuf/util/internal/utility.h
        src/google/protobuf/util/json_util.cc
        src/google/protobuf/util/json_util.h
        src/google/protobuf/util/message_differencer.cc
        src/google/protobuf/util/message_differencer.h
        src/google/protobuf/util/time_util.cc
        src/google/protobuf/util/time_util.h
        src/google/protobuf/util/type_resolver_util.cc
        src/google/protobuf/util/type_resolver_util.h
        src/google/protobuf/wire_format.cc
        src/google/protobuf/wire_format.h
        src/google/protobuf/wrappers.pb.cc
        src/google/protobuf/wrappers.pb.h
    )

    add_library(protobuf.libprotoc EXCLUDE_FROM_ALL STATIC)
    add_library(emil::protobuf.libprotoc ALIAS protobuf.libprotoc)

    target_compile_options(protobuf.libprotoc PRIVATE
        $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-Wno-enum-compare-switch>
        $<$<BOOL:${TARGET_BUILD_WIN}>:
            $<$<CXX_COMPILER_ID:MSVC>:
                /wd4018
                /wd4065
                /wd4244
                /wd4251
                /wd4267
                /wd4305
                /wd4309
                /wd4506
                /wd4661
                /wd4800
                /wd4838
                /wd4996
            >
        >
    )

    target_link_options(protobuf.libprotoc PRIVATE
        $<$<BOOL:${TARGET_BUILD_WIN}>:
            $<$<CXX_COMPILER_ID:MSVC>:/ignore:4217>
        >
    )

    target_compile_definitions(protobuf.libprotoc PUBLIC
        LIBPROTOC_EXPORTS
    )

    target_link_libraries(protobuf.libprotoc PUBLIC
        protobuf.libprotobuf
    )

    target_sources(protobuf.libprotoc PRIVATE
        src/google/protobuf/any.cc

        src/google/protobuf/compiler/code_generator.cc
        src/google/protobuf/compiler/code_generator.h
        src/google/protobuf/compiler/command_line_interface.cc
        src/google/protobuf/compiler/command_line_interface.h
        src/google/protobuf/compiler/cpp/cpp_enum.cc
        src/google/protobuf/compiler/cpp/cpp_enum.h
        src/google/protobuf/compiler/cpp/cpp_enum_field.cc
        src/google/protobuf/compiler/cpp/cpp_enum_field.h
        src/google/protobuf/compiler/cpp/cpp_extension.cc
        src/google/protobuf/compiler/cpp/cpp_extension.h
        src/google/protobuf/compiler/cpp/cpp_field.cc
        src/google/protobuf/compiler/cpp/cpp_field.h
        src/google/protobuf/compiler/cpp/cpp_file.cc
        src/google/protobuf/compiler/cpp/cpp_file.h
        src/google/protobuf/compiler/cpp/cpp_generator.cc
        src/google/protobuf/compiler/cpp/cpp_generator.h
        src/google/protobuf/compiler/cpp/cpp_helpers.cc
        src/google/protobuf/compiler/cpp/cpp_helpers.h
        src/google/protobuf/compiler/cpp/cpp_map_field.cc
        src/google/protobuf/compiler/cpp/cpp_map_field.h
        src/google/protobuf/compiler/cpp/cpp_message.cc
        src/google/protobuf/compiler/cpp/cpp_message.h
        src/google/protobuf/compiler/cpp/cpp_message_field.cc
        src/google/protobuf/compiler/cpp/cpp_message_field.h
        src/google/protobuf/compiler/cpp/cpp_primitive_field.cc
        src/google/protobuf/compiler/cpp/cpp_primitive_field.h
        src/google/protobuf/compiler/cpp/cpp_service.cc
        src/google/protobuf/compiler/cpp/cpp_service.h
        src/google/protobuf/compiler/cpp/cpp_string_field.cc
        src/google/protobuf/compiler/cpp/cpp_string_field.h
        #src/google/protobuf/compiler/csharp/csharp_doc_comment.cc
        #src/google/protobuf/compiler/csharp/csharp_enum.cc
        #src/google/protobuf/compiler/csharp/csharp_enum_field.cc
        #src/google/protobuf/compiler/csharp/csharp_field_base.cc
        #src/google/protobuf/compiler/csharp/csharp_generator.cc
        #src/google/protobuf/compiler/csharp/csharp_helpers.cc
        #src/google/protobuf/compiler/csharp/csharp_map_field.cc
        #src/google/protobuf/compiler/csharp/csharp_message.cc
        #src/google/protobuf/compiler/csharp/csharp_message_field.cc
        #src/google/protobuf/compiler/csharp/csharp_primitive_field.cc
        #src/google/protobuf/compiler/csharp/csharp_reflection_class.cc
        #src/google/protobuf/compiler/csharp/csharp_repeated_enum_field.cc
        #src/google/protobuf/compiler/csharp/csharp_repeated_message_field.cc
        #src/google/protobuf/compiler/csharp/csharp_repeated_primitive_field.cc
        #src/google/protobuf/compiler/csharp/csharp_source_generator_base.cc
        #src/google/protobuf/compiler/csharp/csharp_wrapper_field.cc
        #src/google/protobuf/compiler/java/java_context.cc
        #src/google/protobuf/compiler/java/java_doc_comment.cc
        #src/google/protobuf/compiler/java/java_enum.cc
        #src/google/protobuf/compiler/java/java_enum_field.cc
        #src/google/protobuf/compiler/java/java_enum_field_lite.cc
        #src/google/protobuf/compiler/java/java_enum_lite.cc
        #src/google/protobuf/compiler/java/java_extension.cc
        #src/google/protobuf/compiler/java/java_extension_lite.cc
        #src/google/protobuf/compiler/java/java_field.cc
        #src/google/protobuf/compiler/java/java_file.cc
        #src/google/protobuf/compiler/java/java_generator.cc
        #src/google/protobuf/compiler/java/java_generator_factory.cc
        #src/google/protobuf/compiler/java/java_helpers.cc
        #src/google/protobuf/compiler/java/java_lazy_message_field.cc
        #src/google/protobuf/compiler/java/java_lazy_message_field_lite.cc
        #src/google/protobuf/compiler/java/java_map_field.cc
        #src/google/protobuf/compiler/java/java_map_field_lite.cc
        #src/google/protobuf/compiler/java/java_message.cc
        #src/google/protobuf/compiler/java/java_message_builder.cc
        #src/google/protobuf/compiler/java/java_message_builder_lite.cc
        #src/google/protobuf/compiler/java/java_message_field.cc
        #src/google/protobuf/compiler/java/java_message_field_lite.cc
        #src/google/protobuf/compiler/java/java_message_lite.cc
        #src/google/protobuf/compiler/java/java_name_resolver.cc
        #src/google/protobuf/compiler/java/java_primitive_field.cc
        #src/google/protobuf/compiler/java/java_primitive_field_lite.cc
        #src/google/protobuf/compiler/java/java_service.cc
        #src/google/protobuf/compiler/java/java_shared_code_generator.cc
        #src/google/protobuf/compiler/java/java_string_field.cc
        #src/google/protobuf/compiler/java/java_string_field_lite.cc
        #src/google/protobuf/compiler/javanano/javanano_enum.cc
        #src/google/protobuf/compiler/javanano/javanano_enum_field.cc
        #src/google/protobuf/compiler/javanano/javanano_extension.cc
        #src/google/protobuf/compiler/javanano/javanano_field.cc
        #src/google/protobuf/compiler/javanano/javanano_file.cc
        #src/google/protobuf/compiler/javanano/javanano_generator.cc
        #src/google/protobuf/compiler/javanano/javanano_helpers.cc
        #src/google/protobuf/compiler/javanano/javanano_map_field.cc
        #src/google/protobuf/compiler/javanano/javanano_message.cc
        #src/google/protobuf/compiler/javanano/javanano_message_field.cc
        #src/google/protobuf/compiler/javanano/javanano_primitive_field.cc
        #src/google/protobuf/compiler/js/js_generator.cc
        #src/google/protobuf/compiler/js/well_known_types_embed.cc
        #src/google/protobuf/compiler/objectivec/objectivec_enum.cc
        #src/google/protobuf/compiler/objectivec/objectivec_enum_field.cc
        #src/google/protobuf/compiler/objectivec/objectivec_extension.cc
        #src/google/protobuf/compiler/objectivec/objectivec_field.cc
        #src/google/protobuf/compiler/objectivec/objectivec_file.cc
        #src/google/protobuf/compiler/objectivec/objectivec_generator.cc
        #src/google/protobuf/compiler/objectivec/objectivec_helpers.cc
        #src/google/protobuf/compiler/objectivec/objectivec_map_field.cc
        #src/google/protobuf/compiler/objectivec/objectivec_message.cc
        #src/google/protobuf/compiler/objectivec/objectivec_message_field.cc
        #src/google/protobuf/compiler/objectivec/objectivec_oneof.cc
        #src/google/protobuf/compiler/objectivec/objectivec_primitive_field.cc
        #src/google/protobuf/compiler/php/php_generator.cc
        src/google/protobuf/compiler/plugin.cc
        src/google/protobuf/compiler/plugin.h
        src/google/protobuf/compiler/plugin.pb.cc
        src/google/protobuf/compiler/plugin.pb.h
        src/google/protobuf/compiler/profile.pb.cc
        src/google/protobuf/compiler/profile.pb.h
        #src/google/protobuf/compiler/python/python_generator.cc
        #src/google/protobuf/compiler/ruby/ruby_generator.cc
        src/google/protobuf/compiler/subprocess.cc
        src/google/protobuf/compiler/subprocess.h
        src/google/protobuf/compiler/zip_writer.cc
        src/google/protobuf/compiler/zip_writer.h
    )

    install(TARGETS
        protobuf.libprotobuf_lite
        protobuf.libprotobuf
        protobuf.libprotoc
        EXPORT emilProtobufHostTargets
    )
endif()
