# CONTAINERS

## 1. Introduction

Containers are used to store a number of elements. While the C++ standard template library contains several containers like `std::vector` and `std::set`, the storage of these containers is allocated on the heap. In embedded applications, more control over the storage is desired. For this reason, the infra package contains two types of containers: the `Bounded` containers and the `Intrusive` containers. While their storage is not on the heap, the interfaces of these containers closely mimic the interfaces of the standard containers.

## 2. Bounded Containers

### 2.1 Overview

The first strategy to deal with storage in containers is by allocating memory inside the containers themselves. The size of a container therefore includes the storage of its elements. The maximum number of elements to be stored is passed as a template parameter to the instantiation of the container, e.g. `infra::BoundedVector<int>::WithMaxSize<5> myVector` declares a container `myVector` which holds up to 5 integers. Since the size of the storage is known up front, the maximum number of elements that can be placed in the container is bounded, hence their name.

The Bounded counterparts of the containers of the standard library are the following:

| Bounded Container | Standard Container |
| ----------------- | ------------------ |
| `infra::BoundedVector<T>::WithMaxSize<N>` | `std::vector<T>` |
| `infra::BoundedDeque<T>::WithMaxSize<N>` | `std::deque<T>` |
| `infra::BoundedString<T>::WithStorage<N>` | `std::string<T>` |
| `infra::BoundedPriorityQueue<T, N>` | `std::priority_queue<T>` |
| `infra::BoundedList<T>::WithMaxSize<N>` | `std::list<T>` |
| `infra::BoundedForwardList<T>::WithMaxSize<N>` | `std::forward_list<T>` |

These bounded containers conform to the same specifications as placed by the C++ standard on their counterparts where possible, so they generally have the same constructors, accessors, types, and complexity requirements. One notable omission is the allocator passed to standard containers, since bounded containers allocate their elements inside their own space. These containers have two additional accessors: `std::size_t max_size()` and bool `full()`.

While the intention of this document is not to repeat the documentation available on standard containers, for quick reference a summary of available functions is given here:

| Method | Description |
| ------ | ----------- |
| `bool empty()` | Returns true if and only if the container is empty |
| `std::size_t size()` | Returns the number of currently held elements |
| `iterator begin()` | Returns an iterator to the first element |
| `iterator end()` | Returns an iterator which points to one past the last element |
| `void clear()` | Removes all elements |
| `void push_back(element)` | Inserts one element to the back |
| `void push_front(element)` | Inserts one element to the front |
| `void emplace_back(...)` | Constructs one element at the back, parameters are forwarded to the constructor of the element |
| `void emplace_front(...)` | Constructs one element at the front, parameters are forwarded to the constructor of the element |
| `void pop_back()` | Removes one element from the back |
| `void pop_front()` | Removes one element from the front |
| `iterator insert(position, element)` | Inserts an element at the given position |
| `iterator erase(position)` | Removes an element at the given position |

### 2.2 References to Bounded Containers

Often, the class that holds a container of elements is not the class that makes the decision how big that container should be. For example, the `UrlRouter` holds a `BoundedVector` for storing the `PageServers` to which it refers. It doesn’t know, however, whether it should be able to hold 2, 10, or 500 `PageServers`. Allocating too much space would be wasteful; but at the place in the code where the `UrlRouter` is instantiated this limit is known. In order to specify the maximum storage for a container at a later moment, `BoundedVector<T>` (and similarly other containers) may be created as a reference to a `BoundedVector<T>::WithStorage<N>`. By constructing the actual `BoundedVector` at the point of instantiation of `UrlRouter`, the choice for the maximum number of elements is delayed. For example:

```cpp
class UrlRouter
{
public:
    ...
    explicit UrlRouter(infra::BoundedVector<PageServer>& storage);

protected:
    infra::BoundedVector<PageServer>& pageServers;
};
```

The `UrlRouter` can now be instantiated as follows:

```cpp
    infra::BoundedVector<PageServer>::WithMaxSize<10> storage;
    UrlRouter router(storage);
```

The `UrlRouter` now uses a `BoundedVector` with a maximum size of 10 elements.
In order to simplify instantiation of the `UrlRouter`, it uses the `infra::WithStorage` helper class to feed a constructed `BoundedVector` to the `UrlRouter`. Using this `WithMaxSize` parameter, instantiation is simplified to:

```cpp
class UrlRouter
{
public:
    ...
    template<std::size_t Max>
        using WithMaxSize = infra::WithStorage<UrlRouter, infra::BoundedVector<PageServer>::WithMaxSize<Max>>;

    explicit UrlRouter(infra::BoundedVector<PageServer>& storage);

protected:
    infra::BoundedVector<PageServer>& pageServers;
};

UrlRouter::WithMaxSize<10> router;
```

## 3. Intrusive Containers

Another approach for storing elements in a container is to not allocate additional storage for an object, but to make the user of a container responsible for storage allocation. By adding some additional administration to an object, a container can keep track of its objects by jumping from object to object via that additional administration. For instance, objects used in an `infra::IntrusiveForwardList` each have a next pointer, so that when the container keeps track of its first element it can enumerate all its elements by following the next pointer.

In order to inject the administration into elements, the elements derive from `container::NodeType`. The container’s administration therefore intrudes into the element, hence their name `Intrusive` containers.
A consequence of this is that an element can only be assigned to one container at a time, but unlike the `Bounded` containers, `Intrusive` containers cannot become full.

Like the `Bounded` containers, the infra package contains counterparts of containers found in the standard library that closely mimic their behaviour. The provided `Intrusive` containers are:

| Intrusive Container | Standard Container |
| ------------------- | ------------------ |
| `infra::IntrusiveForwardList<T>` | `std::forward_list<T>` |
| `infra::IntrusiveList<T>` | `std::list<T>` |
| `infra::IntrusivePriorityQueue<T, Compare>` | `std::priority_queue<T, Compare>` |
| `infra::IntrusiveSet<T, Compare>` | `std::set<T, Compare>` |

## 4. std::array

One standard container of note is `std::array<T, Size>`. This container does not use the heap and has a fixed size. It is a drop-in replacement of the C array, with a C++ interface. When a container of a fixed-size is needed, `std::array` should be used.
