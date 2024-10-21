#ifndef CORESTATEMANAGER_H
#define CORESTATEMANAGER_H

#include <vector>
#include <mutex>

class CoreStateManager {
public:
    // Singleton instance
    static CoreStateManager& getInstance();

    // Set the state of a core (true = busy, false = idle)
    void setCoreState(int coreID, bool state);
    
    // Get the state of an individual core (true = busy, false = idle)
    bool getCoreState(int coreID);

    // Get all core states (true = busy, false = idle)
    const std::vector<bool>& getCoreStates() const;

    // Initialize the cores (set all to idle, false)
    void initialize(int nCore);

private:
    // Private constructor to enforce Singleton pattern
    CoreStateManager() = default;
    
    // Disable copy constructor and assignment operator
    CoreStateManager(const CoreStateManager&) = delete;
    CoreStateManager& operator=(const CoreStateManager&) = delete;

    // Vector to store the state of each core (true = busy, false = idle)
    std::vector<bool> coreStates;
    
    // Mutex for thread-safe access to coreStates
    mutable std::mutex mutex;
};

#endif // CORESTATEMANAGER_H
