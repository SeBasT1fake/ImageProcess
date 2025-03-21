#ifndef BUDDY_ALLOCATOR_H
#define BUDDY_ALLOCATOR_H

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <cstdint>

class BuddyAllocator {
public:
    // Initialize allocator with 2^maxOrder bytes
    BuddyAllocator(size_t maxOrder);
    ~BuddyAllocator();
    
    // Allocate memory of given size
    void* allocate(size_t size);
    
    // Free allocated memory
    void deallocate(void* ptr);
    
    // Get total memory currently allocated
    size_t getTotalAllocated() const;

private:
    // Maximum order (power of 2) for the allocator
    size_t maxOrder;
    
    // Total memory pool size (2^maxOrder)
    size_t poolSize;
    
    // Base memory address
    char* memoryPool;
    
    // Represents the availability status of blocks at each level
    std::vector<std::vector<bool>> availableBlocks;
    
    // Map of allocated pointers to their sizes (for deallocation)
    std::unordered_map<void*, size_t> allocatedBlocks;
    
    // Total memory currently allocated
    size_t totalAllocated;
    
    // Helper functions
    size_t getSizeForOrder(size_t order) const;
    size_t getOrderForSize(size_t size) const;
    void markBlockUnavailable(size_t order, size_t blockIndex);
    void markBlockAvailable(size_t order, size_t blockIndex);
    bool isBlockAvailable(size_t order, size_t blockIndex) const;
    void splitBlock(size_t order, size_t blockIndex);
    void mergeBlocks(size_t order, size_t blockIndex);
    size_t getBuddyIndex(size_t blockIndex) const;
    void* getBlockAddress(size_t order, size_t blockIndex) const;
    std::pair<size_t, size_t> findBlock(void* ptr) const;
};

#endif // BUDDY_ALLOCATOR_H