add_library(infra.util ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_include_directories(infra.util PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_compile_options(infra.util PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:
        /Zc:__cplusplus
        /wd4068 # Disable unknown pragma warning
    >
)

target_sources(infra.util PRIVATE
    Aligned.hpp
    Allocator.hpp
    AllocatorFixedSpace.hpp
    AllocatorHeap.hpp
    AutoResetFunction.hpp
    AutoResetMultiFunction.hpp
    Base64.cpp
    Base64.hpp
    BitLogic.hpp
    BoundedDeque.hpp
    BoundedForwardList.hpp
    BoundedList.hpp
    BoundedPriorityQueue.hpp
    BoundedString.cpp
    BoundedString.hpp
    BoundedVector.hpp
    ByteRange.hpp
    CompareMembers.hpp
    Compatibility.hpp
    ConstructBin.cpp
    ConstructBin.hpp
    Crc.hpp
    CrcCcittCalculator.hpp
    CyclicBuffer.hpp
    Endian.hpp
    EnumCast.hpp
    FixedPoint.hpp
    Function.cpp
    Function.hpp
    IntegerNormalization.hpp
    InterfaceConnector.hpp
    IntrusiveBinarySearchTree.hpp
    IntrusiveForwardList.hpp
    IntrusiveList.hpp
    IntrusivePriorityQueue.hpp
    IntrusiveSet.hpp
    IntrusiveUnorderedSet.hpp
    LogAndAbort.cpp
    LogAndAbort.hpp
    MemoryRange.hpp
    MultiFunction.hpp
    Observer.hpp
    Overloaded.hpp
    PolymorphicVariant.hpp
    PostAssign.hpp
    ProxyCreator.hpp
    ReallyAssert.cpp
    ReallyAssert.hpp
    ReferenceCountedSingleton.hpp
    ReverseRange.hpp
    Sequencer.cpp
    Sequencer.hpp
    SharedObjectAllocator.hpp
    SharedObjectAllocatorFixedSize.hpp
    SharedObjectAllocatorHeap.hpp
    SharedOptional.cpp
    SharedOptional.hpp
    SharedOwnedObserver.hpp
    SharedPtr.cpp
    SharedPtr.hpp
    StaticStorage.hpp
    Tokenizer.cpp
    Tokenizer.hpp
    Unit.hpp
    VariadicTemplates.hpp
    WithStorage.hpp
)

if (EMIL_BUILD_WIN)
    target_sources(infra.util PRIVATE
        infra.util.natvis
    )
endif()


set(conditional_compile_definitions
    EMIL_HOST_BUILD

    # Enable the logging hooks for LogAndAbort and ReallyAssert
    EMIL_ENABLE_LOG_AND_ABORT_LOGGING
    EMIL_ENABLE_REALLY_ASSERT_LOGGING

    # Optionally enable logging file paths (or only filenames) upon LOG_AND_ABORT or really_assert
    EMIL_ENABLE_LOGGING_FILE_UPON_ABORT
    EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT
)
foreach(var IN LISTS conditional_compile_definitions)
    if (${var})
        target_compile_definitions(infra.util PUBLIC ${var})
        message(STATUS "infra.util: Enabled compile definition ${var}")
    endif()
endforeach()

add_subdirectory(test)
add_subdirectory(test_helper)

function(emil_transform_file_to_string input name namespace output)

    add_custom_command(
        OUTPUT "generated/${namespace}/${output}.cpp" "generated/${namespace}/${output}.hpp"
        COMMAND ${CMAKE_COMMAND} -D script_dir="${CMAKE_CURRENT_FUNCTION_LIST_DIR}" -D list_dir="${CMAKE_CURRENT_LIST_DIR}" -D input="${input}" -D name=${name} -D namespace=${namespace} -D output="${output}" -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/transform_file_to_string.cmake"
        DEPENDS ${input} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/transform_file_to_string.cmake
    )

endfunction()

function(emil_target_string_source target input name namespace output)

    emil_transform_file_to_string("${input}" ${name} ${namespace} "${output}")
    target_sources(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated/${namespace}/${output}.cpp ${CMAKE_CURRENT_BINARY_DIR}/generated/${namespace}/${output}.hpp)
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

endfunction()

function(emil_transform_file_to_range input name namespace output)

    add_custom_command(
        OUTPUT "generated/${namespace}/${output}.cpp" "generated/${namespace}/${output}.hpp"
        COMMAND ${CMAKE_COMMAND} -D script_dir="${CMAKE_CURRENT_FUNCTION_LIST_DIR}" -D list_dir="${CMAKE_CURRENT_LIST_DIR}" -D input="${input}" -D name=${name} -D namespace=${namespace} -D output="${output}" -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/transform_file_to_range.cmake"
        DEPENDS ${input} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/transform_file_to_range.cmake
    )

endfunction()

function(emil_target_range_source target input name namespace output)

    emil_transform_file_to_range("${input}" ${name} ${namespace} "${output}")
    target_sources(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated/${namespace}/${output}.cpp ${CMAKE_CURRENT_BINARY_DIR}/generated/${namespace}/${output}.hpp)
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

endfunction()
