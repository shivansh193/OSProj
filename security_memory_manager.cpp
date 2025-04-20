#include "security_memory_manager.h"
#include <iostream>
#include <algorithm>

SecurityMemoryManager::SecurityMemoryManager() {}

void SecurityMemoryManager::registerProcess(pid_t pid) {
    std::lock_guard<std::mutex> lock(mtx);
    processSecurityProfiles[pid] = SecurityProfile();
}

void SecurityMemoryManager::unregisterProcess(pid_t pid) {
    std::lock_guard<std::mutex> lock(mtx);
    processSecurityProfiles.erase(pid);
}

MemoryRegion SecurityMemoryManager::allocateSecureMemory(pid_t pid, size_t size, SecurityLevel reqLevel) {
    std::lock_guard<std::mutex> lock(mtx);
    auto& secProfile = processSecurityProfiles[pid];
    MemoryProtectionLevel protLevel = determineProtectionLevel(reqLevel, secProfile.trustScore);
    MemoryRegion region;
    region.address = memoryAllocator_allocate(size);
    region.size = size;
    region.secLevel = reqLevel;
    switch (protLevel) {
        case MemoryProtectionLevel::ENCRYPTED:
            applyMemoryEncryption(region);
            break;
        case MemoryProtectionLevel::HARDWARE_ISOLATED:
            if (hardwareSupportsIsolation()) {
                region = createHardwareSecureEnclave(size);
            } else {
                applyMemoryEncryption(region);
            }
            break;
        case MemoryProtectionLevel::FULLY_SECURED:
            region = createHardwareSecureEnclave(size);
            applyMemoryEncryption(region);
            applyAccessPatternObfuscation(region);
            break;
        default:
            break;
    }
    anomalyDetector.registerRegionForMonitoring(pid, region);
    std::cout << "Allocated secure memory for PID: " << pid << std::endl;
    return region;
}

void SecurityMemoryManager::monitorMemoryAccess() {
    std::lock_guard<std::mutex> lock(mtx);
    auto anomalies = anomalyDetector.detectAnomalies();
    for (const auto& anomaly : anomalies) {
        if (anomaly.severity > CRITICAL_THRESHOLD) {
            handleSecurityBreach(anomaly);
        } else {
            logSuspiciousActivity(anomaly);
            auto& secProfile = processSecurityProfiles[anomaly.pid];
            secProfile.trustScore -= anomaly.severity;
        }
    }
}

bool SecurityMemoryManager::validateMemoryAccess(pid_t pid, void* address, size_t size, AccessType access) {
    std::lock_guard<std::mutex> lock(mtx);
    auto region = findMemoryRegion(address);
    if (!region) return true;
    return checkAccessRights(pid, region, access);
}

std::vector<pid_t> SecurityMemoryManager::getAllPIDs() {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<pid_t> pids;
    for (const auto& [pid, _] : processSecurityProfiles) pids.push_back(pid);
    return pids;
}

SecurityMemoryManager::MemoryProtectionLevel SecurityMemoryManager::determineProtectionLevel(SecurityLevel req, int trust) {
    if (req == SecurityLevel::HIGH && trust < 50) return MemoryProtectionLevel::FULLY_SECURED;
    if (req == SecurityLevel::HIGH) return MemoryProtectionLevel::HARDWARE_ISOLATED;
    if (req == SecurityLevel::MEDIUM) return MemoryProtectionLevel::ENCRYPTED;
    return MemoryProtectionLevel::STANDARD;
}
void* SecurityMemoryManager::memoryAllocator_allocate(size_t sz) { return malloc(sz); }
void SecurityMemoryManager::applyMemoryEncryption(MemoryRegion&) {/* Stub */}
bool SecurityMemoryManager::hardwareSupportsIsolation() { return false; }
MemoryRegion SecurityMemoryManager::createHardwareSecureEnclave(size_t sz) {
    MemoryRegion r;
    r.address = malloc(sz);
    r.size = sz;
    r.secLevel = SecurityLevel::HIGH;
    return r;
}
void SecurityMemoryManager::applyAccessPatternObfuscation(MemoryRegion&) {/* Stub */}
void SecurityMemoryManager::handleSecurityBreach(const Anomaly& a) {
    std::cout << "SECURITY BREACH! PID: " << a.pid << ", Severity: " << a.severity << std::endl;
}
void SecurityMemoryManager::logSuspiciousActivity(const Anomaly& a) {
    std::cout << "Suspicious activity. PID: " << a.pid << ", Severity: " << a.severity << std::endl;
}
MemoryRegion* SecurityMemoryManager::findMemoryRegion(void* address) { return nullptr; /* Stub */ }
bool SecurityMemoryManager::checkAccessRights(pid_t, MemoryRegion*, AccessType) { return true; }
