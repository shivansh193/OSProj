// simulation.cpp
// Simulates 100-200 processes, resource usage, and optimization using all three modules
#include "adaptive_scheduler.h"
#include "adaptive_memory_manager.h"
#include "security_memory_manager.h"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>

constexpr int NUM_PROCESSES = 150;
constexpr int SIMULATION_TICKS = 500;
constexpr int MAX_CPU = 100;
constexpr int MAX_IO = 100;
constexpr int MAX_MEM = 16384; // 16MB

struct SimProcess {
    pid_t pid;
    std::string name;
    int cpuProfile; // 0=light, 1=medium, 2=heavy
    int ioProfile;  // 0=light, 1=medium, 2=heavy
    int memProfile; // 0=light, 1=medium, 2=heavy
    SecurityLevel secLevel;
    size_t memAllocated;
};

int main() {
    AdaptiveScheduler scheduler;
    AdaptiveMemoryManager memManager;
    SecurityMemoryManager secManager;
    std::vector<SimProcess> processes;
    std::mt19937 rng((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> profileDist(0,2);
    std::uniform_int_distribution<int> secDist(0,2);
    std::uniform_int_distribution<int> cpuDist(1,MAX_CPU);
    std::uniform_int_distribution<int> ioDist(1,MAX_IO);
    std::uniform_int_distribution<int> memDist(1024,MAX_MEM);

    // 1. Create processes
    for (int i = 0; i < NUM_PROCESSES; ++i) {
        pid_t pid = 1000 + i;
        std::string name = "proc_" + std::to_string(pid);
        int cpuP = profileDist(rng);
        int ioP = profileDist(rng);
        int memP = profileDist(rng);
        SecurityLevel sec = static_cast<SecurityLevel>(secDist(rng));
        processes.push_back({pid, name, cpuP, ioP, memP, sec, 0});
        scheduler.registerProcess(pid, name);
        memManager.registerProcess(pid);
        secManager.registerProcess(pid);
    }

    // 2. Simulate ticks
    for (int tick = 0; tick < SIMULATION_TICKS; ++tick) {
        // Each process generates resource usage
        for (auto& proc : processes) {
            int cpu = cpuDist(rng) * (proc.cpuProfile+1) / 3;
            int io = ioDist(rng) * (proc.ioProfile+1) / 3;
            size_t mem = memDist(rng) * (proc.memProfile+1) / 3;
            proc.memAllocated = mem;
            scheduler.updateUsageMetrics(proc.pid, {ApplicationEvent::OTHER, 0}, cpu, io);
            memManager.predictMemoryNeeds(proc.pid, mem);
            memManager.allocateMemoryByTier(proc.pid, mem, proc.secLevel);
            secManager.allocateSecureMemory(proc.pid, mem/4, proc.secLevel);
        }
        // System-wide analysis
        memManager.analyzeMemoryUsage();
        secManager.monitorMemoryAccess();
        // Scheduler optimizes
        auto decisions = scheduler.calculateProcessPriorities();
        // Simulate: print top 3 scheduled
        if (tick % 100 == 0) {
            std::cout << "Tick " << tick << " top scheduled: ";
            for (int i = 0; i < 3 && i < (int)decisions.size(); ++i) {
                std::cout << decisions[i].process_id << "(P=" << decisions[i].base_priority << ") ";
            }
            std::cout << std::endl;
        }
        // Optionally: std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // 3. Print summary
    std::cout << "\nSimulation complete.\n";
    std::cout << "Total processes: " << processes.size() << std::endl;
    std::cout << "Total memory used: " << memManager.getTotalMemoryUsage() << " bytes" << std::endl;
    std::cout << "(See logs above for periodic optimization results.)" << std::endl;
    return 0;
}
