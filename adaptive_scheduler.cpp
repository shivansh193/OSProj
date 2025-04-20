#include "adaptive_scheduler.h"
#include <algorithm>

AdaptiveScheduler::AdaptiveScheduler() {}

void AdaptiveScheduler::registerProcess(pid_t pid, const std::string& name) {
    std::lock_guard<std::mutex> lock(mtx);
    userProfiles[pid] = ApplicationProfile{pid, name};
    processMetrics[pid] = UsageMetrics();
}

void AdaptiveScheduler::unregisterProcess(pid_t pid) {
    std::lock_guard<std::mutex> lock(mtx);
    userProfiles.erase(pid);
    processMetrics.erase(pid);
    dependencies.erase(pid);
}

void AdaptiveScheduler::updateUsageMetrics(pid_t pid, ApplicationEvent event, int cpuUsage, int ioUsage) {
    std::lock_guard<std::mutex> lock(mtx);
    auto& metrics = processMetrics[pid];
    metrics.lastInteractionTime = std::time(nullptr);
    metrics.interactionCount++;
    metrics.cpuUsage = cpuUsage;
    metrics.ioUsage = ioUsage;
    if (event.type == ApplicationEvent::FOCUS_CHANGE) {
        recordApplicationDependency(event.previous_pid, pid);
    }
}

std::vector<AdaptiveScheduler::SchedulingDecision> AdaptiveScheduler::calculateProcessPriorities() {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<SchedulingDecision> decisions;
    for (const auto& [pid, metrics] : processMetrics) {
        float importance = priorityModel.predict({
            static_cast<float>(metrics.interactionCount),
            timeSinceLastInteraction(metrics),
            getDependencyScore(pid),
            getTimeOfDayRelevance(pid),
            static_cast<float>(metrics.cpuUsage),
            static_cast<float>(metrics.ioUsage)
        });
        float performanceFactor = getSystemPerformanceFactor(pid);
        decisions.push_back({
            pid,
            calculateBasePriority(importance),
            importance * performanceFactor,
            calculateTimeSlice(importance, performanceFactor)
        });
    }
    // Sort by importance descending for simulation
    std::sort(decisions.begin(), decisions.end(), [](const SchedulingDecision& a, const SchedulingDecision& b) {
        return a.importance_factor > b.importance_factor;
    });
    return decisions;
}

void AdaptiveScheduler::recordApplicationDependency(pid_t prev, pid_t curr) {
    dependencies[curr].push_back(prev);
    std::cout << "Dependency: " << prev << " -> " << curr << std::endl;
}

float AdaptiveScheduler::timeSinceLastInteraction(const UsageMetrics& metrics) {
    return static_cast<float>(std::time(nullptr) - metrics.lastInteractionTime);
}

float AdaptiveScheduler::getDependencyScore(pid_t pid) {
    // Example: if many dependencies, increase importance
    auto it = dependencies.find(pid);
    return it != dependencies.end() ? 1.0f + 0.1f * it->second.size() : 1.0f;
}

float AdaptiveScheduler::getTimeOfDayRelevance(pid_t pid) {
    // Stub: could use actual time for real system
    return 1.0f;
}

float AdaptiveScheduler::getSystemPerformanceFactor(pid_t pid) {
    // Stub: could use system load, latency, etc.
    return 1.0f;
}

int AdaptiveScheduler::calculateBasePriority(float importance) {
    return static_cast<int>(importance * 10);
}

TimeSlice AdaptiveScheduler::calculateTimeSlice(float importance, float perf) {
    return {static_cast<int>(100 * importance * perf)};
}

std::vector<pid_t> AdaptiveScheduler::getAllPIDs() {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<pid_t> pids;
    for (const auto& [pid, _] : processMetrics) pids.push_back(pid);
    return pids;
}
