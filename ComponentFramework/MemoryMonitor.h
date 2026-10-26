#ifndef MEMORYMONITOR_H
#define MEMORYMONITOR_H
#include <iostream>
//#include <memory>
#include <fstream>
#include <cstddef>
#include <cstdlib>

constexpr std::size_t MAX_TRACKED_ALLOCATIONS = 100000; // Instead of storing allocations in a vector/unordered map, since those take up memory as well;

struct AllocationRecord
{
    void* address; // address in memory;
    std::size_t size; // how many bytes were allocated
    std::size_t allocationID; // #1, #2, etc - helps track which ones are which
};

AllocationRecord allocations[MAX_TRACKED_ALLOCATIONS]; // uses the maxed number from above, this actually starts up the database
std::size_t allocationCount = 0; // how many entries are currently used;
std::size_t usedMemoryTotal = 0;
std::size_t totalAllocations = 0;
std::size_t totalFrees = 0;
std::size_t peakMemory = 0;
std::size_t peakAllocations = 0; // counts how many allocations were active max at the same time;
std::size_t nextAllocationID = 1; // ID counter;

void usedMemoryAdd(void* address, std::size_t numBytes)
{
    usedMemoryTotal += numBytes;
    totalAllocations++;

    if (usedMemoryTotal > peakMemory) // check peak memory
        peakMemory = usedMemoryTotal;

    if (allocationCount + 1 > peakAllocations) // check peak allocation count
        peakAllocations = allocationCount + 1;

    if (allocationCount < MAX_TRACKED_ALLOCATIONS) // record the allocation if we still have space for the counter;
    {
        allocations[allocationCount].address = address;
        allocations[allocationCount].size = numBytes;
        allocations[allocationCount].allocationID = nextAllocationID++;

        allocationCount++;
    }
}

void usedMemorySubtract(void* address, std::size_t numBytes)
{
    usedMemoryTotal -= numBytes;
    totalFrees++;
       
    for (std::size_t i = 0; i < allocationCount; i++) // Find the allocation that corresponds to this address
    {
        if (allocations[i].address == address)
        {
            allocations[i] = allocations[allocationCount - 1]; // Replace the removed entry with the last entry

            allocationCount--;

            return;
        }
    }
}

void* operator new(std::size_t numBytes) {
    void* memory = std::malloc(numBytes);

    if (memory)
    {
        usedMemoryAdd(memory, numBytes);
    }

    return memory;
}

void operator delete(void* memoryLocation, std::size_t numBytes) noexcept { // sized delete
    if (memoryLocation)
    {
        usedMemorySubtract(memoryLocation, numBytes);
        std::free(memoryLocation);
    }
}

void operator delete(void* memoryLocation) noexcept { // unsized delete
    if (memoryLocation) {
        for (std::size_t i = 0; i < allocationCount; i++) { // Find the allocation ourselves because this version doesn't give us the size.
            if (allocations[i].address == memoryLocation)
            {
                std::size_t size = allocations[i].size;

                usedMemorySubtract(memoryLocation, size);

                std::free(memoryLocation);

                return;
            }
        }
                
        std::free(memoryLocation); // If we couldn't find it, still free the memory.
    }
}

void* operator new[](std::size_t numBytes) {
    void* memory = std::malloc(numBytes);

    if (memory) {
        usedMemoryAdd(memory, numBytes);
    }

    return memory;
}

void operator delete[](void* memoryLocation, std::size_t numBytes) noexcept {
    if (memoryLocation) {
        usedMemorySubtract(memoryLocation, numBytes);
        std::free(memoryLocation);
    }
}

void operator delete[](void* memoryLocation) noexcept {
    if (memoryLocation) {
        for (std::size_t i = 0; i < allocationCount; i++) {
            if (allocations[i].address == memoryLocation) {
                std::size_t size = allocations[i].size;

                usedMemorySubtract(memoryLocation, size);

                std::free(memoryLocation);

                return;
            }
        }
        std::free(memoryLocation);
    }
}

void writeMemoryLog()
{
    std::ofstream file("memory_log.txt");

    if (!file.is_open())
        return;

    file << "========================================\n";
    file << "          MEMORY MONITOR REPORT\n";
    file << "========================================\n\n";

    file << "Total allocations:   "
        << totalAllocations << "\n";

    file << "Total frees:         "
        << totalFrees << "\n";

    file << "Current allocations: "
        << allocationCount << "\n";

    file << "Current memory:      "
        << usedMemoryTotal << " bytes\n";

    file << "Peak allocations:    "
        << peakAllocations << "\n";

    file << "Peak memory:         "
        << peakMemory << " bytes\n";

    file << "\n========================================\n";
    file << "       OUTSTANDING ALLOCATIONS\n";
    file << "========================================\n\n";

    if (allocationCount == 0)
    {
        file << "No outstanding allocations.\n";
    }
    else
    {
        for (std::size_t i = 0; i < allocationCount; i++)
        {
            file << "Allocation ID: "
                << allocations[i].allocationID << "\n";

            file << "Address: "
                << allocations[i].address << "\n";

            file << "Size: "
                << allocations[i].size
                << " bytes\n\n";
        }
    }

    file << "========================================\n";

    if (usedMemoryTotal == 0)
    {
        file << "RESULT: No outstanding memory.\n";
    }
    else
    {
        file << "RESULT: Potential memory leak detected.\n";
    }

    file << "========================================\n";
}

#endif

