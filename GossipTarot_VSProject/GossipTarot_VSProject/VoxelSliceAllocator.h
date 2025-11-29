#pragma once

// created using gemini

#include <list>
#include <mutex>
#include <algorithm>
#include <stdexcept>

class VoxelSliceAllocator {
public:
    // A structure to hold the location and size of an allocated slice
    struct SliceHandle {
        size_t index = 0;
        size_t size = 0;
        bool valid = false;
    };

    unsigned int allocationNextFrom = 0;

    // Constructor: Pre-allocates the entire memory pool
    VoxelSliceAllocator(size_t total_elements) :
        total_size_(total_elements) {
        // Initialize the Free List with one block representing the entire pool
        free_list_.push_back({ 0, total_elements });
    }

    // Allocate a slice of 'count' elements
    SliceHandle allocate_slice(size_t count) {
        std::lock_guard<std::mutex> lock(mutex_);

        // Use an iterator to search the Free List (First-Fit strategy)
        auto it = free_list_.begin();
        while (it != free_list_.end()) {
            if (it->size >= count) {
                // Found a block large enough
                SliceHandle handle = { it->start_index, count, true };

                if (it->size == count) {
                    // Exact match: remove the block
                    it = free_list_.erase(it);
                }
                else {
                    // Larger block: shrink the free block
                    it->start_index += count;
                    it->size -= count;
                }

                if (allocationNextFrom < handle.index + handle.size) {
                    allocationNextFrom = handle.index + handle.size;
                }

                return handle;
            }
            ++it;
        }

        // Allocation failed (fragmentation or out of memory)
        throw std::bad_alloc();
    }

    // Reclaim a slice given its handle
    void reclaim_slice(SliceHandle handle) {
        if (!handle.valid) return;

        std::lock_guard<std::mutex> lock(mutex_);

        // 1. Insert the reclaimed block into the free list (and keep it sorted by index)
        FreeBlock new_block = { handle.index, handle.size };
        auto it = free_list_.begin();
        while (it != free_list_.end() && it->start_index < new_block.start_index) {
            ++it;
        }
        it = free_list_.insert(it, new_block); // 'it' now points to the new block

        // 2. Coalescing (Merging adjacent free blocks)

        // Coalesce with the block BEFORE the new block
        if (it != free_list_.begin()) {
            auto prev_it = std::prev(it);
            if (prev_it->start_index + prev_it->size == it->start_index) {
                // Merge 'it' into 'prev_it'
                prev_it->size += it->size;
                it = free_list_.erase(it); // 'it' now points to the successor of the erased element
            }
        }

        // Coalesce with the block AFTER the new block (which 'it' now points to, or to its successor)
        if (it != free_list_.end()) {
            // Check the block *before* the current 'it' (which is the combined/new block)
            auto current_block_it = (it != free_list_.begin() && std::prev(it)->start_index < handle.index) ? std::prev(it) : it;

            if (current_block_it->start_index + current_block_it->size == it->start_index) {
                // Merge 'it' into 'current_block_it'
                current_block_it->size += it->size;
                free_list_.erase(it);
            }
        }
    }


private:
    // Represents a continuous block of free memory within the pool_ vector
    struct FreeBlock {
        size_t start_index;
        size_t size;
    };

    std::list<FreeBlock> free_list_; // A list of free blocks, sorted by start_index
    std::mutex mutex_;
    size_t total_size_;
};
