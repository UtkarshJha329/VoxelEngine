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

    // Try insert only; returns true if inserted, false if key already existed
    bool try_insert(const Key& key, const T& value) {
        Bucket& b = get_bucket(key);
        std::lock_guard<std::mutex> lock(b.mtx);
        auto [it, inserted] = b.map.emplace(key, value);
        return inserted;
    }

    // try_emplace with perfect-forwarding
    template<typename... Args>
    bool try_emplace(const Key& key, Args&&... args) {
        Bucket& b = get_bucket(key);
        std::lock_guard<std::mutex> lock(b.mtx);
        auto [it, inserted] = b.map.try_emplace(key, std::forward<Args>(args)...);
        return inserted;
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

    // Acquire or create: returns a copy of existing or inserted default value
    T get_or_insert(const Key& key, const T& defaultValue) {
        Bucket& b = get_bucket(key);
        std::lock_guard<std::mutex> lock(b.mtx);
        auto it = b.map.find(key);
        if (it == b.map.end()) {
            auto [newIt, inserted] = b.map.emplace(key, defaultValue);
            return newIt->second; // copy
        }
        return it->second; // copy
    }

    // Update-in-place using a callback under lock: callback receives T& (if present) and returns bool whether modified
    // If key doesn't exist, callback won't be called (use insert_or_assign or try_emplace)
    template<typename Func>
    bool update_if_exists(const Key& key, Func&& updater) {
        Bucket& b = get_bucket(key);
        std::lock_guard<std::mutex> lock(b.mtx);
        auto it = b.map.find(key);
        if (it == b.map.end()) return false;
        updater(it->second);
        return true;
    }

    // Apply a function to every element (locks buckets one at a time). The callable receives (const Key&, T&).
    // NOTE: The caller must avoid long blocking inside the function to avoid hurting concurrency.
    template<typename Func>
    void for_each(Func&& f) {
        for (auto& bucket : buckets) {
            std::lock_guard<std::mutex> lock(bucket.mtx);
            for (auto& kv : bucket.map) {
                f(kv.first, kv.second);
            }
        }
    }

    // Return a snapshot (copy) of the whole map. Useful when you need to iterate safely across all elements.
    std::unordered_map<Key, T, Hash, KeyEqual> snapshot() const {
        std::unordered_map<Key, T, Hash, KeyEqual> result;
        // Reserve approximately (optional)
        // We can't know exact size cheaply without locking all buckets, so we accumulate.
        size_t total = 0;
        for (const auto& b : buckets) {
            std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(b.mtx));
            total += b.map.size();
        }
        result.reserve(total);
        for (const auto& b : buckets) {
            std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(b.mtx));
            for (const auto& kv : b.map) result.emplace(kv.first, kv.second);
        }
        return result;
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