#include "adaptive_scheduler.h"
#include "adaptive_memory_manager.h"
#include "security_memory_manager.h"
#include <iostream>

int main() {
    AdaptiveScheduler scheduler;
    AdaptiveMemoryManager memManager;
    SecurityMemoryManager secManager;
    
    // Simulate process usage
    pid_t pid1 = 1001, pid2 = 1002;
    ApplicationEvent evt{ApplicationEvent::FOCUS_CHANGE, pid1};
    scheduler.updateUsageMetrics(pid2, evt);
    auto decisions = scheduler.calculateProcessPriorities();
    std::cout << "\nProcess Scheduling Decisions:\n";
    for (const auto& d : decisions) {
        std::cout << "PID: " << d.process_id << ", Priority: " << d.base_priority
                  << ", Importance: " << d.importance_factor << ", TimeSlice: " << d.allocation.ms << "ms\n";
    }

    // Simulate memory management
    memManager.analyzeMemoryUsage();
    memManager.predictMemoryNeeds(pid1);
    memManager.allocateMemoryByTier(pid1, 4096, SecurityLevel::MEDIUM);

    // Simulate secure memory allocation
    auto region = secManager.allocateSecureMemory(pid1, 2048, SecurityLevel::HIGH);
    secManager.monitorMemoryAccess();
    bool access = secManager.validateMemoryAccess(pid1, region.address, region.size, AccessType::READ);
    std::cout << "\nMemory access for PID " << pid1 << (access ? ": allowed" : ": denied") << std::endl;

    std::cout << "\nDemo completed. All modules operational.\n";
    return 0;
}
