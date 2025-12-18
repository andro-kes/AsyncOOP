#pragma once

#include "npc.h"
#include <vector>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <queue>
#include <condition_variable>

struct CombatEvent {
    NPCPtr attacker;
    NPCPtr defender;
};

class Game {
public:
    Game(int mapSize, int npcCount, int duration);
    ~Game();

    void run();

private:
    void initializeNPCs();
    void movementThread();
    void combatThread();
    void printThread();
    
    void moveNPC(NPCPtr npc);
    void detectCombats();
    bool processCombat(const CombatEvent& event);
    void printMap();
    
    int mapSize_;
    int npcCount_;
    int duration_;
    
    std::vector<NPCPtr> npcs_;
    std::queue<CombatEvent> combatQueue_;
    
    // Synchronization primitives
    std::shared_mutex npcsMutex_;
    std::mutex combatQueueMutex_;
    std::mutex coutMutex_;
    std::condition_variable combatCV_;
    
    std::atomic<bool> running_;
    
    std::thread movementThread_;
    std::thread combatThread_;
    std::thread printThread_;
};
