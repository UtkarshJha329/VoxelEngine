#pragma once

// made using gemini

#include <unordered_map>
#include <mutex>
#include <shared_mutex> // For fine-grained control (read/write lock)
#include <optional>

template <typename Key, typename Value>
class TSUnorderedMap {
public:
    // --- Writer (Exclusive) Operations ---
    void insert(const Key& key, const Value& value) {
        // Only one thread can write at a time
        std::lock_guard<std::mutex> lock(mutex_);
        map_[key] = value;
    }

    void erase(const Key& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        map_.erase(key);
    }

    // --- Reader (Shared) Operations ---
    bool find(const Key& key, Value& value) const {
        // Mutex is held, so no writes can happen during the read
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = map_.find(key);
        if (it != map_.end()) {
            value = it->second;
            return true;
        }
        return false;
    }

private:
    std::unordered_map<Key, Value> map_;
    mutable std::mutex mutex_; // Must be mutable for use in const methods
};