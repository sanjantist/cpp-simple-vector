#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iterator>
#include <utility>

template <typename Type>
class ArrayPtr {
   public:
    ArrayPtr() = default;

    explicit ArrayPtr(size_t size) {
        raw_ptr_ = (size == 0 ? nullptr : new Type[size]);
    }

    explicit ArrayPtr(Type* raw_ptr) noexcept : raw_ptr_(raw_ptr) {}

    ArrayPtr(const ArrayPtr&) = delete;

    ArrayPtr(ArrayPtr&& other) noexcept
        : raw_ptr_(std::exchange(other.raw_ptr_, nullptr)) {}

    ~ArrayPtr() { delete[] raw_ptr_; }

    ArrayPtr& operator=(const ArrayPtr&) = delete;

    ArrayPtr& operator=(ArrayPtr&& other) noexcept {
        if (this != &other) {
            std::swap(raw_ptr_, other.raw_ptr_);
        }

        return *this;
    }

    [[nodiscard]] Type* Release() noexcept {
        return std::exchange(this, nullptr);
    }

    Type& operator[](size_t index) noexcept { return raw_ptr_[index]; }

    const Type& operator[](size_t index) const noexcept {
        return raw_ptr_[index];
    }

    explicit operator bool() const { return raw_ptr_; }

    Type* Get() const noexcept { return raw_ptr_; }

    void swap(ArrayPtr& other) noexcept { std::swap(raw_ptr_, other.raw_ptr_); }

   private:
    Type* raw_ptr_ = nullptr;
};