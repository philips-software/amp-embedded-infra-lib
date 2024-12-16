add_executable(infra.util_test)
emil_build_for(infra.util_test BOOL EMIL_BUILD_TESTS)
emil_add_test(infra.util_test)

target_link_libraries(infra.util_test PUBLIC
    gmock_main
    infra.util
    infra.util_test_helper
)

target_sources(infra.util_test PRIVATE
    TestAllocatorFixedSpace.cpp
    TestAllocatorHeap.cpp
    TestAutoResetFunction.cpp
    TestAutoResetMultiFunction.cpp
    TestBitLogic.cpp
    TestBoundedDeque.cpp
    TestBoundedForwardList.cpp
    TestBoundedList.cpp
    TestBoundedPriorityQueue.cpp
    TestBoundedString.cpp
    TestBoundedVector.cpp
    TestCompareMembers.cpp
    TestCrc.cpp
    TestCrcCcittCalculator.cpp
    TestCyclicBuffer.cpp
    TestEndian.cpp
    TestFixedPoint.cpp
    TestFunction.cpp
    TestInterfaceConnector.cpp
    TestIntrusiveBinarySearchTree.cpp
    TestIntrusiveForwardList.cpp
    TestIntrusiveList.cpp
    TestIntrusivePriorityQueue.cpp
    TestIntrusiveSet.cpp
    TestIntrusiveUnorderedSet.cpp
    TestMemoryRange.cpp
    TestObserver.cpp
    TestOptional.cpp
    TestPolymorphicVariant.cpp
    TestProxyCreator.cpp
    TestReferenceCountedSingleton.cpp
    TestSequencer.cpp
    TestSharedObjectAllocatorFixedSize.cpp
    TestSharedObjectAllocatorHeap.cpp
    TestSharedOptional.cpp
    TestSharedPtr.cpp
    TestStaticStorage.cpp
    TestTokenizer.cpp
    TestUnit.cpp
    TestVariant.cpp
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Workaround for false positive on stringop overflow
    set_source_files_properties(TestBoundedVector.cpp PROPERTIES COMPILE_FLAGS -Wno-stringop-overflow)
endif()

if (EMIL_BUILD_WIN)
    target_sources(infra.util_test PRIVATE
        TestNatvis.cpp
    )
endif()
