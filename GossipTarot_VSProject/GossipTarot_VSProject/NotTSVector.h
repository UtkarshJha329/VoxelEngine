// Chat gpt

#pragma once

#include <vector>
#include <shared_mutex>

template <typename T>
class NotTSVector {
public:
    NotTSVector() = default;
    NotTSVector(const NotTSVector& other) {
        std::shared_lock lock(other.mutex_);
        vec_ = other.vec_;
    }

    // Add an element
    void push_back(const T& value) {
        std::unique_lock lock(mutex_);
        vec_.push_back(value);
    }

    void push_back(T&& value) {
        std::unique_lock lock(mutex_);
        vec_.push_back(std::move(value));
    }

    // Read an element safely
    T get(size_t index) const {
        std::shared_lock lock(mutex_);
        return vec_[index];
    }

    // Get current size
    size_t size() const {
        std::shared_lock lock(mutex_);
        return vec_.size();
    }

    // Clear contents
    void clear() {
        std::unique_lock lock(mutex_);
        vec_.clear();
    }

    std::vector<T> vec_;

private:
    mutable std::shared_mutex mutex_;
};