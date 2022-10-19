#ifndef INFRA_ALLOCATOR_FIXED_SPACE_HPP
#define INFRA_ALLOCATOR_FIXED_SPACE_HPP

#include "infra/util/Allocator.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/StaticStorage.hpp"

namespace infra
{
    template<class T, std::size_t NumberOfElements, class ConstructionArgs>
    class AllocatorFixedSpace;

    template<class T, std::size_t NumberOfElements, class... ConstructionArgs>
    class AllocatorFixedSpace<T, NumberOfElements, void(ConstructionArgs...)>
        : public Allocator<T, void(ConstructionArgs...)>
    {
        static_assert(sizeof(T) == sizeof(StaticStorage<T>), "sizeof(StaticStorage) must be equal to sizeof(T) else reinterpret_cast will fail");

    public:
        ~AllocatorFixedSpace();

        virtual UniquePtr<T> Allocate(ConstructionArgs... args) override;
        virtual void Deallocate(void* object) override;

    private:
        struct Node
            : public StaticStorage<T>
        {
            Node* next;
        };

        std::size_t FreeListSize() const;
        Node* AllocateNode();

    private:
        typename infra::BoundedVector<Node>::template WithMaxSize<NumberOfElements> elements;
        Node* freeList = nullptr;
    };

    ////    Implementation    ////

    template<class T, std::size_t NumberOfElements, class... ConstructionArgs>
    AllocatorFixedSpace<T, NumberOfElements, void(ConstructionArgs...)>::~AllocatorFixedSpace()
    {
        assert(FreeListSize() == elements.size());
    }

    template<class T, std::size_t NumberOfElements, class... ConstructionArgs>
    UniquePtr<T> AllocatorFixedSpace<T, NumberOfElements, void(ConstructionArgs...)>::Allocate(ConstructionArgs... args)
    {
        Node* node = AllocateNode();
        node->Construct(std::forward<ConstructionArgs>(args)...);

        return MakeUnique<T>(&**node, *this);
    }

    template<class T, std::size_t NumberOfElements, class... ConstructionArgs>
    void AllocatorFixedSpace<T, NumberOfElements, void(ConstructionArgs...)>::Deallocate(void* object)
    {
        Node* node = static_cast<Node*>(reinterpret_cast<StaticStorage<T>*>(static_cast<T*>(object)));
        node->Destruct();

        node->next = freeList;
        freeList = node;
    }

    template<class T, std::size_t NumberOfElements, class... ConstructionArgs>
    std::size_t AllocatorFixedSpace<T, NumberOfElements, void(ConstructionArgs...)>::FreeListSize() const
    {
        Node* node = freeList;
        std::size_t result = 0;

        while (node)
        {
            node = node->next;
            ++result;
        }

        return result;
    }

    template<class T, std::size_t NumberOfElements, class... ConstructionArgs>
    typename AllocatorFixedSpace<T, NumberOfElements, void(ConstructionArgs...)>::Node* AllocatorFixedSpace<T, NumberOfElements, void(ConstructionArgs...)>::AllocateNode()
    {
        if (freeList)
        {
            Node* result = freeList;
            freeList = freeList->next;
            return result;
        }
        else
        {
            if (elements.full())
                return nullptr;

            elements.emplace_back();
            return &elements.back();
        }
    }
}

#endif
