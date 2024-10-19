#ifndef CORESTATEMANAGER_H
#define CORESTATEMANAGER_H

#include <vector>
#include <mutex>

class CoreStateManager {
public:
    static CoreStateManager& getInstance();
    void setCoreState(int coreID, bool state);
    const std::vector<bool>& getCoreStates() const;
    void initialize(int nCore);

private:
    CoreStateManager() = default;
    CoreStateManager(const CoreStateManager&) = delete;
    CoreStateManager& operator=(const CoreStateManager&) = delete;
    std::vector<bool> coreStates;
    mutable std::mutex mutex;
};

#endif // CORESTATEMANAGER_H
