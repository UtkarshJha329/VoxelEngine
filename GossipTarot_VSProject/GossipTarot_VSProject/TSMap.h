#pragma once

#include <map>
#include <shared_mutex>
#include <utility>

//Made using gemini
template <typename Key, typename Value>
class TSMap {
private:
    std::map<Key, Value> data_;
    // Use shared_mutex for readers-writer lock
    mutable std::shared_mutex mutex_;

public:
    // --- 1. Thread-Safe Write/Insert Operation ---
    void insert(const Key& key, const Value& value) {
        // Unique lock for writing (exclusive access)
        std::unique_lock<std::shared_mutex> lock(mutex_);
        data_.insert(std::make_pair(key, value));
    }

    // --- 2. Thread-Safe Read Operation ---
    // Note: The function is const, so we use std::shared_lock
    bool get(const Key& key, Value& out_value) const {
        // Shared lock for reading (multiple readers allowed)
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = data_.find(key);
        if (it != data_.end()) {
            out_value = it->second;
            return true;
        }
        return false;
    }

    // --- 3. Thread-Safe Delete Operation ---
    bool erase(const Key& key) {
        // Unique lock for writing (exclusive access)
        std::unique_lock<std::shared_mutex> lock(mutex_);
        return data_.erase(key) > 0;
    }

    // --- 4. Thread-Safe Size Operation ---
    size_t size() const {
        // Shared lock for reading
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return data_.size();
    }

    /**
     * Finds the first element whose key is not less than the given key.
     * @return true if an element is found, false if the key is out of bounds (end()).
     */
    bool get_lower_bound_pair(const Key& key, Key& out_key, Value& out_value) const {
        // Acquire a shared lock, as this is a read operation
        std::shared_lock<std::shared_mutex> lock(mutex_);

        // Call lower_bound on the protected internal map
        auto it = data_.lower_bound(key);

        // Check if the iterator is valid (i.e., not at the end of the map)
        if (it != data_.end()) {
            // Found it! Copy the key and value to the output parameters
            out_key = it->first;
            out_value = it->second;
            return true;
        }

        // Key was greater than all elements in the map
        return false;
    }
};