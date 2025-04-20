#ifndef ADAPTIVE_MEMORY_MANAGER_H
#define ADAPTIVE_MEMORY_MANAGER_H

#include <vector>
#include <unordered_map>
#include <ctime>

using pid_t = int;

enum class SecurityLevel { LOW, MEDIUM, HIGH };

struct MemoryRegion {
    pid_t pid;
    size_t size;
    std::time_t allocTime;
    SecurityLevel secLevel;
    void* address;
};

struct MemoryPrediction {
    size_t expectedGrowth;
    void update(size_t currentUsage) {
        expectedGrowth = currentUsage / 10 + 1024; // Stub
    }
    MemoryPrediction project(int seconds) const {
        MemoryPrediction p = *this;
        p.expectedGrowth += seconds / 60 * 256;
        return p;
    }
};

class AdaptiveMemoryManager {
public:
    AdaptiveMemoryManager();
    // Register a process for memory tracking
    void registerProcess(pid_t pid);
    // Remove a process
    void unregisterProcess(pid_t pid);
    // Analyze system-wide memory usage and rebalance
    void analyzeMemoryUsage();
    // Predict memory needs for a process
    void predictMemoryNeeds(pid_t pid, size_t currentUsage = 0);
    // Allocate memory by tier and security level
    void allocateMemoryByTier(pid_t pid, size_t size, SecurityLevel secLevel);
    // Get total memory usage
    size_t getTotalMemoryUsage();
    // For simulation: get all known PIDs
    std::vector<pid_t> getAllPIDs();

private:
    struct MemoryTier {
        size_t totalSize;
        size_t availableSize;
        float accessSpeed;
        std::vector<MemoryRegion> allocations;
    };
    std::vector<MemoryTier> memoryTiers;
    std::unordered_map<pid_t, MemoryPrediction> processMemoryNeeds;
    std::unordered_map<pid_t, size_t> processMemoryUsage;
    std::mutex mtx;
    const size_t MEMORY_GROWTH_THRESHOLD = 4096;

    float calculateSystemMemoryUtilization();
    std::vector<MemoryRegion> findUnderutilizedMemoryRegions();
    std::vector<pid_t> identifyMemoryStarvedProcesses();
    void redistributeMemory(const std::vector<MemoryRegion>&, const std::vector<pid_t>&);
    size_t getCurrentMemoryUsage(pid_t);
    int selectAppropriateMemoryTier(pid_t, SecurityLevel);
    std::time_t getCurrentTime();
    void preAllocateMemory(pid_t, size_t);
};

#endif // ADAPTIVE_MEMORY_MANAGER_H
