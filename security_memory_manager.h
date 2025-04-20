#ifndef SECURITY_MEMORY_MANAGER_H
#define SECURITY_MEMORY_MANAGER_H

#include <unordered_map>
#include <vector>
#include <ctime>
#include <iostream>

using pid_t = int;

enum class SecurityLevel { LOW, MEDIUM, HIGH };
enum class AccessType { READ, WRITE, EXECUTE };

struct SecurityProfile {
    int trustScore = 100;
};

struct MemoryRegion {
    void* address;
    size_t size;
    SecurityLevel secLevel;
};

struct Anomaly {
    pid_t pid;
    int severity;
};

struct AnomalyDetector {
    void registerRegionForMonitoring(pid_t, const MemoryRegion&) {/* Stub */}
    std::vector<Anomaly> detectAnomalies() { return {}; }
};

class SecurityMemoryManager {
public:
    SecurityMemoryManager();
    // Register a process for security tracking
    void registerProcess(pid_t pid);
    // Remove a process
    void unregisterProcess(pid_t pid);
    // Allocate secure memory for a process
    MemoryRegion allocateSecureMemory(pid_t pid, size_t size, SecurityLevel reqLevel);
    // Monitor all memory access for anomalies
    void monitorMemoryAccess();
    // Validate a memory access
    bool validateMemoryAccess(pid_t pid, void* address, size_t size, AccessType access);
    // For simulation: get all known PIDs
    std::vector<pid_t> getAllPIDs();

private:
    std::unordered_map<pid_t, SecurityProfile> processSecurityProfiles;
    AnomalyDetector anomalyDetector;
    std::mutex mtx;
    const int CRITICAL_THRESHOLD = 80;

    enum class MemoryProtectionLevel {
        STANDARD,
        ENCRYPTED,
        HARDWARE_ISOLATED,
        FULLY_SECURED
    };

    MemoryProtectionLevel determineProtectionLevel(SecurityLevel req, int trust);
    void* memoryAllocator_allocate(size_t sz);
    void applyMemoryEncryption(MemoryRegion&);
    bool hardwareSupportsIsolation();
    MemoryRegion createHardwareSecureEnclave(size_t sz);
    void applyAccessPatternObfuscation(MemoryRegion&);
    void handleSecurityBreach(const Anomaly& a);
    void logSuspiciousActivity(const Anomaly& a);
    MemoryRegion* findMemoryRegion(void* address);
    bool checkAccessRights(pid_t, MemoryRegion*, AccessType);
};

#endif // SECURITY_MEMORY_MANAGER_H
