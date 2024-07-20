#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>

#include "array_ptr.h"

class ReserveProxyObj {
   public:
    size_t capacity_to_reserve_;
    ReserveProxyObj(size_t capacity) : capacity_to_reserve_(capacity) {}
};

inline ReserveProxyObj Reserve(size_t capacity) {
    return ReserveProxyObj(capacity);
}

template <typename Type>
class SimpleVector {
   public:
    using Iterator = Type *;
    using ConstIterator = const Type *;

    SimpleVector() noexcept = default;

    SimpleVector(const SimpleVector &other) {
        ArrayPtr<Type> new_items(other.capacity_);
        std::copy(other.items_.Get(), other.items_.Get() + other.size_,
                  new_items.Get());
        items_.swap(new_items);
        size_ = other.size_;
        capacity_ = other.capacity_;
    }

    explicit SimpleVector(size_t size) : size_(size), capacity_(size) {
        ArrayPtr<Type> new_items(capacity_);
        std::fill(new_items.Get(), new_items.Get() + size, Type());
        items_.swap(new_items);
    }

    SimpleVector(size_t size, const Type &value)
        : size_(size), capacity_(size) {
        ArrayPtr<Type> new_items(capacity_);
        std::fill(new_items.Get(), new_items.Get() + size, value);
        items_.swap(new_items);
    }

    SimpleVector(std::initializer_list<Type> init) {
        size_t size = init.size();
        ArrayPtr<Type> new_items(size);
        std::copy(init.begin(), init.end(), new_items.Get());
        items_.swap(new_items);
        size_ = size;
        capacity_ = size;
    }

    SimpleVector(ReserveProxyObj obj) {
        ArrayPtr<Type> new_items(obj.capacity_to_reserve_);
        std::fill(new_items.Get(), new_items.Get() + obj.capacity_to_reserve_,
                  Type());
        items_.swap(new_items);
        capacity_ = obj.capacity_to_reserve_;
    }

    SimpleVector(SimpleVector &&other) noexcept
        : items_(std::move(other.items_)),
          size_(other.size_),
          capacity_(other.capacity_) {
        other.size_ = 0;
        other.capacity_ = 0;
    }

    size_t GetSize() const noexcept { return size_; }

    size_t GetCapacity() const noexcept { return capacity_; }

    bool IsEmpty() const noexcept { return !size_; }

    SimpleVector &operator=(const SimpleVector &rhs) {
        if (this != &rhs) {
            ArrayPtr<Type> new_items(rhs.capacity_);
            std::copy(rhs.items_.Get(), rhs.items_.Get() + rhs.size_,
                      new_items.Get());
            items_.swap(new_items);
            capacity_ = rhs.capacity_;
            size_ = rhs.size_;
        }

        return *this;
    }

    SimpleVector &operator=(SimpleVector &&other) noexcept {
        if (this != &other) {
            items_ = std::move(other.items_);
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.size_ = 0;
            other.capacity_ = 0;
        }

        return *this;
    }

    Type &operator[](size_t index) noexcept { return items_[index]; }

    const Type &operator[](size_t index) const noexcept {
        return items_[index];
    }

    Type &At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("invalid index");
        }
        return items_[index];
    }

    const Type &At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("invalid index");
        }
        return items_[index];
    }

    void Clear() noexcept { size_ = 0; }

    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        } else {
            if (new_size > capacity_) {
                Reserve(new_size);
            }
            std::uninitialized_value_construct(end(), begin() + new_size);
            size_ = new_size;
        }
    }

    void PushBack(const Type &item) {
        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        items_[size_] = item;
        ++size_;
    }

    void PushBack(Type &&item) {
        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        items_[size_] = std::move(item);
        ++size_;
    }

    void PopBack() noexcept { size_ -= (size_ ? 1 : 0); }

    Iterator Insert(ConstIterator pos, const Type &value) {
        size_t index = std::distance(cbegin(), pos);

        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }

        std::move_backward(begin() + index, end(), end() + 1);
        items_[index] = value;
        ++size_;

        return begin() + index;
    }

    Iterator Insert(ConstIterator pos, Type &&value) {
        size_t index = std::distance(cbegin(), pos);

        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }

        std::move_backward(begin() + index, end(), end() + 1);
        items_[index] = std::move(value);
        ++size_;

        return begin() + index;
    }

    Iterator Erase(ConstIterator pos) {
        size_t index = std::distance(cbegin(), pos);
        std::move(begin() + index + 1, end(), begin() + index);
        --size_;

        return items_.Get() + index;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_items(new_capacity);
            std::move(begin(), end(), new_items.Get());
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
    }

    void swap(SimpleVector &other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    Iterator begin() noexcept { return items_.Get(); }

    Iterator end() noexcept { return items_.Get() + size_; }

    ConstIterator begin() const noexcept { return items_.Get(); }

    ConstIterator end() const noexcept { return items_.Get() + size_; }

    ConstIterator cbegin() const noexcept { return items_.Get(); }

    ConstIterator cend() const noexcept { return items_.Get() + size_; }

   private:
    ArrayPtr<Type> items_;

    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type> &lhs,
                       const SimpleVector<Type> &rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type> &lhs,
                       const SimpleVector<Type> &rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type> &lhs,
                      const SimpleVector<Type> &rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type> &lhs,
                       const SimpleVector<Type> &rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type> &lhs,
                      const SimpleVector<Type> &rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type> &lhs,
                       const SimpleVector<Type> &rhs) {
    return !(lhs < rhs);
}