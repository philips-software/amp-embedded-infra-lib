add_executable(infra.util_test)
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

if (TARGET_BUILD_WIN)
    target_sources(infra.util_test PRIVATE
        TestNatvis.cpp
    )
endif()
