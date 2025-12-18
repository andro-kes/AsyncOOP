#pragma once

#include <string>
#include <memory>
#include <random>
#include <mutex>
#include <atomic>

class NPC {
public:
    enum class Type {
        Knight,
        Squirrel,
        Pegasus
    };

    NPC(Type type, const std::string& name, int x, int y);
    virtual ~NPC() = default;

    Type getType() const { return type_; }
    std::string getName() const { return name_; }
    
    // Thread-safe position getters
    int getX() const;
    int getY() const;
    void getPosition(int& x, int& y) const;
    
    bool isAlive() const { return alive_; }

    void setPosition(int x, int y);
    void kill();

    virtual int getMovementRange() const = 0;
    virtual int getKillRange() const = 0;
    
    // Combat methods
    int rollDice() const;
    virtual int getAttackBonus() const = 0;
    virtual int getDefenseBonus() const = 0;

    double distanceTo(const NPC& other) const;
    int distanceSquaredTo(const NPC& other) const;
    
    static std::string typeToString(Type type);

protected:
    Type type_;
    std::string name_;
    int x_;
    int y_;
    std::atomic<bool> alive_;
    
    mutable std::mutex posMutex_;
    
    static thread_local std::mt19937 rng_;
};

using NPCPtr = std::shared_ptr<NPC>;
