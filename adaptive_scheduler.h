#ifndef ADAPTIVE_SCHEDULER_H
#define ADAPTIVE_SCHEDULER_H

#include <unordered_map>
#include <vector>
#include <string>
#include <ctime>
#include <iostream>
#include <mutex>
#include <atomic>
#include <random>

using pid_t = int;

struct UsageMetrics {
    std::time_t lastInteractionTime;
    int interactionCount;
    int burstCount; // Number of bursts in recent window
    int cpuUsage;   // Simulated CPU usage
    int ioUsage;    // Simulated IO usage
    UsageMetrics() : lastInteractionTime(0), interactionCount(0), burstCount(0), cpuUsage(0), ioUsage(0) {}
};

struct ApplicationEvent {
    enum Type { FOCUS_CHANGE, OTHER } type;
    pid_t previous_pid;
};

struct ApplicationProfile {
    pid_t pid;
    std::string name;
};

struct TimeSlice {
    int ms;
};

namespace ML {
    struct PredictionModel {
        float predict(const std::vector<float>& features) {
            // Stub: return a dummy importance
            return features.empty() ? 1.0f : features[0] * 0.1f + 1.0f;
        }
    };
}

class AdaptiveScheduler {
public:
    struct SchedulingDecision {
        pid_t process_id;
        int base_priority;
        float importance_factor;
        TimeSlice allocation;
    };

    AdaptiveScheduler();
    // Update process usage metrics (thread-safe)
    void updateUsageMetrics(pid_t pid, ApplicationEvent event, int cpuUsage = 0, int ioUsage = 0);
    // Calculate dynamic priorities for all processes
    std::vector<SchedulingDecision> calculateProcessPriorities();
    // Register a new process
    void registerProcess(pid_t pid, const std::string& name);
    // Remove a process
    void unregisterProcess(pid_t pid);
    // For simulation: get all known PIDs
    std::vector<pid_t> getAllPIDs();

private:
    std::unordered_map<pid_t, UsageMetrics> processMetrics;
    std::unordered_map<pid_t, ApplicationProfile> userProfiles;
    ML::PredictionModel priorityModel;
    std::mutex mtx;
    std::unordered_map<pid_t, std::vector<pid_t>> dependencies;

    void recordApplicationDependency(pid_t prev, pid_t curr);
    float timeSinceLastInteraction(const UsageMetrics& metrics);
    float getDependencyScore(pid_t pid);
    float getTimeOfDayRelevance(pid_t pid);
    float getSystemPerformanceFactor(pid_t pid);
    int calculateBasePriority(float importance);
    TimeSlice calculateTimeSlice(float importance, float perf);
};

#endif // ADAPTIVE_SCHEDULER_H
