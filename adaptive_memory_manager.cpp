#include "adaptive_memory_manager.h"
#include <iostream>
#include <algorithm>

AdaptiveMemoryManager::AdaptiveMemoryManager() {
    // Example: create 3 tiers (fast, normal, slow)
    memoryTiers = {
        { 64*1024*1024, 64*1024*1024, 1.0f, {} }, // Fast (e.g., L1/L2)
        { 256*1024*1024, 256*1024*1024, 0.7f, {} }, // Normal (RAM)
        { 1024*1024*1024, 1024*1024*1024, 0.3f, {} } // Slow (swap/SSD)
    };
}

void AdaptiveMemoryManager::registerProcess(pid_t pid) {
    std::lock_guard<std::mutex> lock(mtx);
    processMemoryNeeds[pid] = MemoryPrediction();
    processMemoryUsage[pid] = 0;
}

void AdaptiveMemoryManager::unregisterProcess(pid_t pid) {
    std::lock_guard<std::mutex> lock(mtx);
    processMemoryNeeds.erase(pid);
    processMemoryUsage.erase(pid);
}

void AdaptiveMemoryManager::analyzeMemoryUsage() {
    std::lock_guard<std::mutex> lock(mtx);
    float systemUtilization = calculateSystemMemoryUtilization();
    auto underutilizedRegions = findUnderutilizedMemoryRegions();
    auto starvedProcesses = identifyMemoryStarvedProcesses();
    if (systemUtilization < 0.7f && !starvedProcesses.empty()) {
        redistributeMemory(underutilizedRegions, starvedProcesses);
        std::cout << "Redistributed memory due to low utilization." << std::endl;
    }
}

void AdaptiveMemoryManager::predictMemoryNeeds(pid_t pid, size_t currentUsage) {
    std::lock_guard<std::mutex> lock(mtx);
    auto& prediction = processMemoryNeeds[pid];
    prediction.update(currentUsage);
    processMemoryUsage[pid] = currentUsage;
    auto projectedNeeds = prediction.project(5 * 60);
    if (projectedNeeds.expectedGrowth > MEMORY_GROWTH_THRESHOLD) {
        preAllocateMemory(pid, projectedNeeds.expectedGrowth);
        std::cout << "Pre-allocated memory for PID: " << pid << std::endl;
    }
}

void AdaptiveMemoryManager::allocateMemoryByTier(pid_t pid, size_t size, SecurityLevel secLevel) {
    std::lock_guard<std::mutex> lock(mtx);
    int tierIndex = selectAppropriateMemoryTier(pid, secLevel);
    if (tierIndex >= 0 && tierIndex < static_cast<int>(memoryTiers.size())) {
        memoryTiers[tierIndex].allocations.push_back({pid, size, getCurrentTime(), secLevel, nullptr});
        memoryTiers[tierIndex].availableSize -= size;
        processMemoryUsage[pid] += size;
        std::cout << "Allocated " << size << " bytes to PID: " << pid << " in tier " << tierIndex << std::endl;
    }
}

size_t AdaptiveMemoryManager::getTotalMemoryUsage() {
    std::lock_guard<std::mutex> lock(mtx);
    size_t total = 0;
    for (const auto& [pid, usage] : processMemoryUsage) total += usage;
    return total;
}

std::vector<pid_t> AdaptiveMemoryManager::getAllPIDs() {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<pid_t> pids;
    for (const auto& [pid, _] : processMemoryNeeds) pids.push_back(pid);
    return pids;
}

float AdaptiveMemoryManager::calculateSystemMemoryUtilization() {
    size_t total = 0, available = 0;
    for (const auto& tier : memoryTiers) {
        total += tier.totalSize;
        available += tier.availableSize;
    }
    return 1.0f - (float)available / (float)total;
}

std::vector<MemoryRegion> AdaptiveMemoryManager::findUnderutilizedMemoryRegions() {
    std::vector<MemoryRegion> result;
    // For demo: return empty
    return result;
}

std::vector<pid_t> AdaptiveMemoryManager::identifyMemoryStarvedProcesses() {
    std::vector<pid_t> result;
    for (const auto& [pid, usage] : processMemoryUsage) {
        if (usage < 2048) result.push_back(pid);
    }
    return result;
}

void AdaptiveMemoryManager::redistributeMemory(const std::vector<MemoryRegion>&, const std::vector<pid_t>&) {
    // Demo: no-op
}

size_t AdaptiveMemoryManager::getCurrentMemoryUsage(pid_t pid) {
    auto it = processMemoryUsage.find(pid);
    return it != processMemoryUsage.end() ? it->second : 0;
}

int AdaptiveMemoryManager::selectAppropriateMemoryTier(pid_t, SecurityLevel secLevel) {
    // Demo: map security level to tier
    switch (secLevel) {
        case SecurityLevel::HIGH: return 0;
        case SecurityLevel::MEDIUM: return 1;
        case SecurityLevel::LOW: default: return 2;
    }
}

std::time_t AdaptiveMemoryManager::getCurrentTime() { return std::time(nullptr); }
void AdaptiveMemoryManager::preAllocateMemory(pid_t, size_t) {/* Stub */}
