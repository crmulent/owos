#include "../include/CoreStateManager.h"

// This Class requires that cores starts at ID 1

CoreStateManager& CoreStateManager::getInstance() {
    static CoreStateManager instance;
    return instance;
}

void CoreStateManager::setCoreState(int coreID, bool state) {
    std::lock_guard<std::mutex> lock(mutex);

    //due to starting ID being 1
    coreID--;
    if (coreID >= 0 && coreID < coreStates.size()) {
        coreStates[coreID] = state;
    }
}

const std::vector<bool>& CoreStateManager::getCoreStates() const {
    return coreStates;
}

void CoreStateManager::initialize(int nCore) {
    coreStates.resize(nCore, false);
}
