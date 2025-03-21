#include "buddy_allocator.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cassert>

BuddyAllocator::BuddyAllocator(size_t maxOrder) : maxOrder(maxOrder), totalAllocated(0) {
    poolSize = 1ULL << maxOrder;
    memoryPool = new char[poolSize];
    
    // Initialize available blocks for each order
    availableBlocks.resize(maxOrder + 1);
    for (size_t order = 0; order <= maxOrder; ++order) {
        size_t numBlocks = 1ULL << (maxOrder - order);
        availableBlocks[order].resize(numBlocks, false);
    }
    
    // Initially, only the largest block is available
    availableBlocks[maxOrder][0] = true;
}

BuddyAllocator::~BuddyAllocator() {
    delete[] memoryPool;
}

size_t BuddyAllocator::getSizeForOrder(size_t order) const {
    return 1ULL << order;
}

size_t BuddyAllocator::getOrderForSize(size_t size) const {
    // Find smallest power of 2 that fits the requested size
    size_t order = 0;
    size_t blockSize = 1;
    
    while (blockSize < size && order < maxOrder) {
        blockSize <<= 1;
        order++;
    }
    
    return order;
}

void* BuddyAllocator::allocate(size_t size) {
    // Minimum allocation size is 16 bytes
    size = std::max(size, size_t(16));
    
    // Find order needed for this allocation
    size_t orderNeeded = getOrderForSize(size);
    
    // Check if order is within our capacity
    if (orderNeeded > maxOrder) {
        return nullptr; // Request too large
    }
    
    // Find a block of appropriate size
    for (size_t currentOrder = orderNeeded; currentOrder <= maxOrder; ++currentOrder) {
        // Look for available block at this order
        for (size_t blockIndex = 0; blockIndex < availableBlocks[currentOrder].size(); ++blockIndex) {
            if (isBlockAvailable(currentOrder, blockIndex)) {
                // Found an available block
                
                // If it's larger than what we need, split it down
                size_t order = currentOrder;
                size_t index = blockIndex;
                
                // Mark this block as unavailable
                markBlockUnavailable(order, index);
                
                // Split the block until we reach the desired size
                while (order > orderNeeded) {
                    splitBlock(order, index);
                    order--;
                    index *= 2; // Left child after splitting
                    
                    // Mark the left child as unavailable (we'll use it)
                    markBlockUnavailable(order, index);
                    // Right child is already marked as available after splitting
                }
                
                // Get address of the allocated block
                void* ptr = getBlockAddress(orderNeeded, index);
                
                // Record allocation details for later deallocation
                allocatedBlocks[ptr] = orderNeeded;
                
                // Update total allocated memory
                totalAllocated += getSizeForOrder(orderNeeded);
                
                return ptr;
            }
        }
    }
    
    // No available block found
    return nullptr;
}

void BuddyAllocator::deallocate(void* ptr) {
    if (!ptr) return;
    
    // Find the order and index of the block
    auto it = allocatedBlocks.find(ptr);
    if (it == allocatedBlocks.end()) {
        return; // Not allocated by this allocator
    }
    
    size_t order = it->second;
    size_t size = getSizeForOrder(order);
    
    // Calculate block index based on pointer address
    char* charPtr = static_cast<char*>(ptr);
    size_t offset = charPtr - memoryPool;
    size_t blockIndex = offset / size;
    
    // Mark the block as available
    markBlockAvailable(order, blockIndex);
    
    // Update total allocated memory
    totalAllocated -= size;
    
    // Attempt to merge with buddy blocks
    while (order < maxOrder) {
        size_t buddyIndex = blockIndex ^ 1; // XOR with 1 to get buddy index
        
        // If buddy is also available, merge them
        if (isBlockAvailable(order, buddyIndex)) {
            // Mark both blocks as unavailable at this level
            markBlockUnavailable(order, blockIndex);
            markBlockUnavailable(order, buddyIndex);
            
            // Move up to the parent block
            blockIndex /= 2;
            order++;
            
            // Mark parent as available
            markBlockAvailable(order, blockIndex);
        } else {
            // Cannot merge further
            break;
        }
    }
    
    // Remove from allocated blocks map
    allocatedBlocks.erase(it);
}

size_t BuddyAllocator::getTotalAllocated() const {
    return totalAllocated;
}

void BuddyAllocator::markBlockUnavailable(size_t order, size_t blockIndex) {
    if (order <= maxOrder && blockIndex < availableBlocks[order].size()) {
        availableBlocks[order][blockIndex] = false;
    }
}

void BuddyAllocator::markBlockAvailable(size_t order, size_t blockIndex) {
    if (order <= maxOrder && blockIndex < availableBlocks[order].size()) {
        availableBlocks[order][blockIndex] = true;
    }
}

bool BuddyAllocator::isBlockAvailable(size_t order, size_t blockIndex) const {
    if (order <= maxOrder && blockIndex < availableBlocks[order].size()) {
        return availableBlocks[order][blockIndex];
    }
    return false;
}

void BuddyAllocator::splitBlock(size_t order, size_t blockIndex) {
    if (order <= 0) return;
    
    // Mark the current block as unavailable
    markBlockUnavailable(order, blockIndex);
    
    // Calculate the indices of the two child blocks
    size_t leftChildIndex = blockIndex * 2;
    size_t rightChildIndex = leftChildIndex + 1;
    
    // Mark both children as available
    markBlockAvailable(order - 1, leftChildIndex);
    markBlockAvailable(order - 1, rightChildIndex);
}

void BuddyAllocator::mergeBlocks(size_t order, size_t blockIndex) {
    if (order >= maxOrder) return;
    
    // Calculate parent block index
    size_t parentIndex = blockIndex / 2;
    
    // Mark the parent block as available
    markBlockAvailable(order + 1, parentIndex);
    
    // Mark both child blocks as unavailable
    markBlockUnavailable(order, blockIndex);
    markBlockUnavailable(order, blockIndex ^ 1);
}

size_t BuddyAllocator::getBuddyIndex(size_t blockIndex) const {
    return blockIndex ^ 1; // XOR with 1 to get buddy index
}

void* BuddyAllocator::getBlockAddress(size_t order, size_t blockIndex) const {
    size_t offset = blockIndex * getSizeForOrder(order);
    return memoryPool + offset;
}

std::pair<size_t, size_t> BuddyAllocator::findBlock(void* ptr) const {
    char* charPtr = static_cast<char*>(ptr);
    size_t offset = charPtr - memoryPool;
    
    // Find the allocated block information
    auto it = allocatedBlocks.find(ptr);
    if (it != allocatedBlocks.end()) {
        size_t order = it->second;
        size_t blockSize = getSizeForOrder(order);
        size_t blockIndex = offset / blockSize;
        return {order, blockIndex};
    }
    
    return {0, 0}; // Not found
}