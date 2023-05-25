#ifndef UPGRADE_SPARSE_VECTOR_HPP
#define UPGRADE_SPARSE_VECTOR_HPP

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <map>
#include <sstream>
#include <vector>

namespace application
{
    class OverwriteException
        : public std::exception
    {
    public:
        explicit OverwriteException(std::size_t position);

        virtual char const* what() const noexcept override;

    private:
        std::string message;
    };

    template<class T>
    class SparseVector
    {
    public:
        class Iterator
        {
        public:
            Iterator(const SparseVector<T>& vector, std::size_t position);

            std::pair<std::size_t, T> operator*() const;
            Iterator& operator++();
            bool operator==(const Iterator& other) const;
            bool operator!=(const Iterator& other) const;

        private:
            const SparseVector<T>& vector;
            std::size_t position;
        };

        bool Empty() const;
        Iterator begin() const;
        Iterator end() const;
        std::size_t Size() const;

        bool InvariantHolds() const;

        void Insert(T element, std::size_t position);
        T& operator[](std::size_t position);
        std::pair<std::size_t, T> ElementAtIndex(std::size_t position) const;

        bool operator==(const SparseVector<T>& other) const;
        bool operator!=(const SparseVector<T>& other) const;

    private:
        std::map<std::size_t, std::vector<T>> buckets;
    };

    ////    Implementation    ////

    inline OverwriteException::OverwriteException(std::size_t position)
    {
        std::stringstream ss;
        ss << "Contents specified twice for memory location at address 0x" << std::hex << std::setw(8) << std::setfill('0') << position;
        message = ss.str();
    }

    inline char const* OverwriteException::what() const noexcept
    {
        return message.c_str();
    }

    template<class T>
    SparseVector<T>::Iterator::Iterator(const SparseVector<T>& vector, std::size_t position)
        : vector(vector)
        , position(position)
    {}

    template<class T>
    std::pair<std::size_t, T> SparseVector<T>::Iterator::operator*() const
    {
        return vector.ElementAtIndex(position);
    }

    template<class T>
    typename SparseVector<T>::Iterator& SparseVector<T>::Iterator::operator++()
    {
        ++position;

        return *this;
    }

    template<class T>
    bool SparseVector<T>::Iterator::operator==(const Iterator& other) const
    {
        return &vector == &other.vector && position == other.position;
    }

    template<class T>
    bool SparseVector<T>::Iterator::operator!=(const Iterator& other) const
    {
        return !(*this == other);
    }

    template<class T>
    bool SparseVector<T>::Empty() const
    {
        return buckets.empty();
    }

    template<class T>
    typename SparseVector<T>::Iterator SparseVector<T>::begin() const
    {
        return Iterator(*this, 0);
    }

    template<class T>
    typename SparseVector<T>::Iterator SparseVector<T>::end() const
    {
        return Iterator(*this, Size());
    }

    template<class T>
    std::size_t SparseVector<T>::Size() const
    {
        std::size_t result = 0;

        for (auto& bucket : buckets)
        {
            result += bucket.second.size();
        }

        return result;
    }

    template<class T>
    bool SparseVector<T>::InvariantHolds() const
    {
        std::ptrdiff_t currentAddress = -1;

        for (auto& bucket : buckets)
        {
            if (static_cast<std::ptrdiff_t>(bucket.first) <= currentAddress)
                return false;

            currentAddress = bucket.first + bucket.second.size();
        }

        return true;
    }

    template<class T>
    void SparseVector<T>::Insert(T element, std::size_t position)
    {
        for (auto& bucket : buckets)
        {
            if (position == bucket.first + bucket.second.size())
            {
                bucket.second.push_back(element);
                return;
            }

            if (position >= bucket.first && position < bucket.first + bucket.second.size())
                throw OverwriteException(position);
        }

        buckets.insert(std::make_pair(position, std::vector<T>(1, element)));
    }

    template<class T>
    T& SparseVector<T>::operator[](std::size_t position)
    {
        for (auto& index : buckets)
        {
            if (index.first <= position && index.first + index.second.size() > position)
                return index.second[position - index.first];
        }

        std::abort();
    }

    template<class T>
    std::pair<std::size_t, T> SparseVector<T>::ElementAtIndex(std::size_t position) const
    {
        for (auto& bucket : buckets)
        {
            if (position < bucket.second.size())
                return std::make_pair(bucket.first + position, bucket.second[position]);

            position -= bucket.second.size();
        }

        std::abort();
    }

    template<class T>
    bool SparseVector<T>::operator==(const SparseVector<T>& other) const
    {
        return buckets == other.buckets;
    }

    template<class T>
    bool SparseVector<T>::operator!=(const SparseVector<T>& other) const
    {
        return !(*this == other);
    }
}

#endif
