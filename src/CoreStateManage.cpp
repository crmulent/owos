#include "../include/CoreStateManager.h"
#include <iostream> // For error logging

// This Class requires that cores starts at ID 1

CoreStateManager& CoreStateManager::getInstance() {
    static CoreStateManager instance;
    return instance;
}

void CoreStateManager::setCoreState(int coreID, bool state) {
    std::lock_guard<std::mutex> lock(mutex);

    // Adjust for core ID starting at 1
    coreID--;
    
    if (coreID >= 0 && coreID < coreStates.size()) {
        coreStates[coreID] = state;
    } else {
        std::cerr << "Error: Core ID " << (coreID + 1) << " is out of range!" << std::endl;
    }
}

bool CoreStateManager::getCoreState(int coreID) {
    std::lock_guard<std::mutex> lock(mutex);

    // Adjust for core ID starting at 1
    coreID--;

    if (coreID >= 0 && coreID < coreStates.size()) {
        return coreStates[coreID];
    } else {
        std::cerr << "Error: Core ID " << (coreID + 1) << " is out of range!" << std::endl;
        return false; // Default return for invalid coreID
    }
}

const std::vector<bool>& CoreStateManager::getCoreStates() const {
    return coreStates;
}

void CoreStateManager::initialize(int nCore) {
    std::lock_guard<std::mutex> lock(mutex);
    coreStates.resize(nCore, false); // Initialize all cores to idle (false)
}

