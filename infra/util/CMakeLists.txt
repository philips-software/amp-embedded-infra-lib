add_library(infra.util EXCLUDE_FROM_ALL STATIC)
add_library(emil::infra.util ALIAS infra.util)
install(TARGETS infra.util EXPORT emilInfraTargets)

target_include_directories(infra.util PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(infra.util PUBLIC
    infra.stream
)

target_sources(infra.util PRIVATE
    Aligned.hpp
    Allocator.hpp
    AllocatorFixedSpace.hpp
    AllocatorHeap.hpp
    AutoResetFunction.hpp
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
    ConstructBin.cpp
    ConstructBin.hpp
    CrcCcittCalculator.cpp
    CrcCcittCalculator.hpp
    CyclicBuffer.hpp
    Endian.hpp
    EnumCast.hpp
    FixedPoint.hpp
    Function.cpp
    Function.hpp
    IntegerNormalization.hpp
    IntegerSequence.hpp
    InterfaceConnector.hpp
    IntrusiveBinarySearchTree.hpp
    IntrusiveForwardList.hpp
    IntrusiveList.hpp
    IntrusivePriorityQueue.hpp
    IntrusiveSet.hpp
    IntrusiveUnorderedSet.hpp
    MemoryRange.hpp
    Observer.hpp
    Optional.cpp
    Optional.hpp
    PolymorphicVariant.hpp
    PostAssign.hpp
    ProxyCreator.hpp
    ReallyAssert.hpp
    ReferenceCountedSingleton.hpp
    ReverseRange.hpp
    Sequencer.cpp
    Sequencer.hpp
    SharedObjectAllocator.hpp
    SharedObjectAllocatorFixedSize.hpp
    SharedObjectAllocatorHeap.hpp
    SharedOptional.hpp
    SharedOwnedObserver.hpp
    SharedPtr.cpp
    SharedPtr.hpp
    StaticStorage.hpp
    Tokenizer.cpp
    Tokenizer.hpp
    Unit.hpp
    VariadicTemplates.hpp
    Variant.hpp
    VariantDetail.hpp
    WithStorage.hpp
)

if (TARGET_BUILD_WIN)
    target_sources(infra.util PRIVATE
        infra.util.natvis
    )
endif()

if (BUILD_TESTING)
    add_subdirectory(test)
    add_subdirectory(test_helper)
endif()
