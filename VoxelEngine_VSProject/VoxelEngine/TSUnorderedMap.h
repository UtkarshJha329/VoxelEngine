#pragma once

#include <unordered_map>
#include <vector>
#include <mutex>
#include <optional>
#include <functional>
#include <shared_mutex> // not required, but left for optional upgrades
#include <numeric>      // for accumulate
#include <utility>

template<
    typename Key,
    typename T,
    typename Hash = std::hash<Key>,
    typename KeyEqual = std::equal_to<Key>,
    size_t NumBuckets = 16
>
class ThreadSafeUnorderedMap {
private:
    struct Bucket {
        std::mutex mtx;
        std::unordered_map<Key, T, Hash, KeyEqual> map;
    };

    std::vector<Bucket> buckets;
    Hash hasher;

    Bucket& get_bucket(const Key& key) {
        auto h = hasher(key);
        return buckets[h % buckets.size()];
    }

public:
    ThreadSafeUnorderedMap() : buckets(NumBuckets) {}

    // Disable copying (copy semantics are expensive & tricky)
    ThreadSafeUnorderedMap(const ThreadSafeUnorderedMap&) = delete;
    ThreadSafeUnorderedMap& operator=(const ThreadSafeUnorderedMap&) = delete;

    // Insert or update: returns true if inserted, false if updated existing
    bool insert_or_assign(const Key& key, const T& value) {
        Bucket& b = get_bucket(key);
        std::lock_guard<std::mutex> lock(b.mtx);
        auto it = b.map.find(key);
        if (it == b.map.end()) {
            b.map.emplace(key, value);
            return true;
        }
        else {
            it->second = value;
            return false;
        }
    }

    // Erase key; returns true if erased
    bool erase(const Key& key) {
        Bucket& b = get_bucket(key);
        std::lock_guard<std::mutex> lock(b.mtx);
        return b.map.erase(key) > 0;
    }

    // Check existence without returning value
    bool contains(const Key& key) const {
        auto h = hasher(key);
        const Bucket& b = buckets[h % buckets.size()];
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(b.mtx));
        return b.map.find(key) != b.map.end();
    }

    // Get a copy of the value if present
    std::optional<T> find(const Key& key) const {
        auto h = hasher(key);
        const Bucket& b = buckets[h % buckets.size()];
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(b.mtx));
        auto it = b.map.find(key);
        if (it == b.map.end()) return std::nullopt;
        return it->second; // copy
    }

    // Acquire : returns a copy of existing value
    T get(const Key& key) {
        Bucket& b = get_bucket(key);
        std::lock_guard<std::mutex> lock(b.mtx);
        auto it = b.map.find(key);
        return it->second; // copy
    }

    // Total size (expensive: locks all buckets)
    size_t size() const {
        size_t total = 0;
        for (const auto& b : buckets) {
            std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(b.mtx));
            total += b.map.size();
        }
        return total;
    }

    // Clear everything
    void clear() {
        for (auto& b : buckets) {
            std::lock_guard<std::mutex> lock(b.mtx);
            b.map.clear();
        }
    }
};