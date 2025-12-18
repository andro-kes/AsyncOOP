#include "../include/game.h"
#include "../include/knight.h"
#include "../include/squirrel.h"
#include "../include/pegasus.h"
#include <iostream>
#include <random>
#include <chrono>
#include <algorithm>

Game::Game(int mapSize, int npcCount, int duration)
    : mapSize_(mapSize), npcCount_(npcCount), duration_(duration), running_(false) {}

Game::~Game() {
    running_ = false;
    combatCV_.notify_all();
    
    if (movementThread_.joinable()) movementThread_.join();
    if (combatThread_.joinable()) combatThread_.join();
    if (printThread_.joinable()) printThread_.join();
}

void Game::run() {
    initializeNPCs();
    
    running_ = true;
    
    // Start threads
    movementThread_ = std::thread(&Game::movementThread, this);
    combatThread_ = std::thread(&Game::combatThread, this);
    printThread_ = std::thread(&Game::printThread, this);
    
    // Run for specified duration
    std::this_thread::sleep_for(std::chrono::seconds(duration_));
    
    // Stop threads
    running_ = false;
    combatCV_.notify_all();
    
    // Wait for threads to finish
    if (movementThread_.joinable()) movementThread_.join();
    if (combatThread_.joinable()) combatThread_.join();
    if (printThread_.joinable()) printThread_.join();
    
    // Print final statistics
    std::lock_guard<std::mutex> lock(coutMutex_);
    std::cout << "\n=== Game Over ===\n";
    
    int alive = 0;
    std::shared_lock<std::shared_mutex> readLock(npcsMutex_);
    for (const auto& npc : npcs_) {
        if (npc->isAlive()) {
            alive++;
            int x, y;
            npc->getPosition(x, y);
            std::cout << npc->getName() << " (" << NPC::typeToString(npc->getType()) 
                     << ") survived at (" << x << ", " << y << ")\n";
        } else {
            std::cout << npc->getName() << " (" << NPC::typeToString(npc->getType()) 
                     << ") was killed\n";
        }
    }
    std::cout << "\nSurvivors: " << alive << " / " << npcs_.size() << "\n";
}

void Game::initializeNPCs() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> posDist(0, mapSize_ - 1);
    std::uniform_int_distribution<> typeDist(0, 2); // 0=Knight, 1=Squirrel, 2=Pegasus

    int knightCount = 0, squirrelCount = 0, pegasusCount = 0;
    
    for (int i = 0; i < npcCount_; ++i) {
        int x = posDist(gen);
        int y = posDist(gen);
        
        NPCPtr npc;
        int typeRoll = typeDist(gen);
        switch (typeRoll) {
            case 0: {
                std::string name = "Knight_" + std::to_string(++knightCount);
                npc = std::make_shared<Knight>(name, x, y);
                break;
            }
            case 1: {
                std::string name = "Squirrel_" + std::to_string(++squirrelCount);
                npc = std::make_shared<Squirrel>(name, x, y);
                break;
            }
            case 2:
            default: {
                std::string name = "Pegasus_" + std::to_string(++pegasusCount);
                npc = std::make_shared<Pegasus>(name, x, y);
                break;
            }
        }
        
        npcs_.push_back(npc);
    }
    
    std::lock_guard<std::mutex> lock(coutMutex_);
    std::cout << "Initialized " << npcs_.size() << " NPCs on " << mapSize_ 
              << "x" << mapSize_ << " map\n"
              << "  Knights : " << knightCount << "\n"
              << "  Squirrels: " << squirrelCount << "\n"
              << "  Pegasus  : " << pegasusCount << "\n";
}

void Game::movementThread() {
    while (running_) {
        {
            std::shared_lock<std::shared_mutex> readLock(npcsMutex_);
            
            // Move NPCs
            for (auto& npc : npcs_) {
                if (npc->isAlive()) {
                    moveNPC(npc);
                }
            }
        }
        
        // Detect combats
        detectCombats();
        
        // Small delay to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Game::moveNPC(NPCPtr npc) {
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_int_distribution<> dirDist(-1, 1);
    
    int range = npc->getMovementRange();
    
    // Move by a random step within the movement range (avoiding modulo bias)
    std::uniform_int_distribution<> stepDist(0, range);
    int dx = dirDist(gen) * stepDist(gen);
    int dy = dirDist(gen) * stepDist(gen);
    
    int currentX, currentY;
    npc->getPosition(currentX, currentY);
    
    int newX = std::max(0, std::min(mapSize_ - 1, currentX + dx));
    int newY = std::max(0, std::min(mapSize_ - 1, currentY + dy));
    
    npc->setPosition(newX, newY);
}

void Game::detectCombats() {
    std::shared_lock<std::shared_mutex> readLock(npcsMutex_);
    
    for (size_t i = 0; i < npcs_.size(); ++i) {
        if (!npcs_[i]->isAlive()) continue;
        
        for (size_t j = i + 1; j < npcs_.size(); ++j) {
            if (!npcs_[j]->isAlive()) continue;
            
            // Use squared distance to avoid expensive sqrt operation
            int distanceSquared = npcs_[i]->distanceSquaredTo(*npcs_[j]);
            
            // Check if within either NPC's kill range
            int maxKillRange = std::max(npcs_[i]->getKillRange(), npcs_[j]->getKillRange());
            if (distanceSquared <= maxKillRange * maxKillRange) {
                
                // Add to combat queue
                std::lock_guard<std::mutex> lock(combatQueueMutex_);
                combatQueue_.push({npcs_[i], npcs_[j]});
                combatCV_.notify_one();
            }
        }
    }
}

void Game::combatThread() {
    while (running_) {
        std::unique_lock<std::mutex> lock(combatQueueMutex_);
        
        // Wait for combat events or timeout
        combatCV_.wait_for(lock, std::chrono::milliseconds(100), 
            [this] { return !combatQueue_.empty() || !running_; });
        
        if (!combatQueue_.empty()) {
            CombatEvent event = combatQueue_.front();
            combatQueue_.pop();
            lock.unlock();
            
            processCombat(event);
        }
    }
}

bool Game::processCombat(const CombatEvent& event) {
    // Check if both NPCs are still alive
    if (!event.attacker->isAlive() || !event.defender->isAlive()) {
        return false;
    }
    
    // Roll dice for attack and defense (d6)
    int attackRoll = event.attacker->rollDice() + event.attacker->getAttackBonus();
    int defenseRoll = event.defender->rollDice() + event.defender->getDefenseBonus();
    
    std::lock_guard<std::mutex> coutLock(coutMutex_);
    
    std::cout << "\n[COMBAT] " << event.attacker->getName() 
              << " vs " << event.defender->getName() << "\n";
    std::cout << "  Attack: " << attackRoll << " vs Defense: " << defenseRoll << "\n";
    
    if (attackRoll > defenseRoll) {
        event.defender->kill();
        std::cout << "  Result: " << event.defender->getName() << " was killed!\n";
        return true;
    } else {
        std::cout << "  Result: " << event.defender->getName() << " defended successfully!\n";
        return false;
    }
}

void Game::printThread() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        printMap();
    }
}

void Game::printMap() {
    std::lock_guard<std::mutex> coutLock(coutMutex_);
    std::shared_lock<std::shared_mutex> readLock(npcsMutex_);
    
    std::cout << "\n=== Map State ===\n";
    
    // Create a simple map representation
    std::vector<std::vector<char>> map(mapSize_, std::vector<char>(mapSize_, '.'));
    
    int aliveCount = 0;
    for (const auto& npc : npcs_) {
        if (npc->isAlive()) {
            aliveCount++;
            int x, y;
            npc->getPosition(x, y);
            
            char symbol;
            switch (npc->getType()) {
                case NPC::Type::Knight: symbol = 'K'; break;
                case NPC::Type::Squirrel: symbol = 'S'; break;
                case NPC::Type::Pegasus: symbol = 'P'; break;
                default: symbol = '?'; break;
            }
            
            // If position already occupied, use '*' to indicate multiple NPCs
            if (map[y][x] != '.') {
                map[y][x] = '*';
            } else {
                map[y][x] = symbol;
            }
        }
    }
    
    // Print a sample of the map (center 20x20 area)
    int startX = std::max(0, mapSize_ / 2 - 10);
    int endX = std::min(mapSize_, mapSize_ / 2 + 10);
    int startY = std::max(0, mapSize_ / 2 - 10);
    int endY = std::min(mapSize_, mapSize_ / 2 + 10);
    
    std::cout << "Center area (" << startX << "-" << endX << ", " 
              << startY << "-" << endY << "):\n";
    
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            std::cout << map[y][x] << ' ';
        }
        std::cout << '\n';
    }
    
    std::cout << "\nAlive NPCs: " << aliveCount << " / " << npcs_.size() << "\n";
    std::cout << "Legend: K=Knight, *=Multiple, .=Empty\n";
}