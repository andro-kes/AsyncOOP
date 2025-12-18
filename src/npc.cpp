#include "npc.h"
#include <cmath>

thread_local std::mt19937 NPC::rng_(std::random_device{}());

NPC::NPC(Type type, const std::string& name, int x, int y)
    : type_(type), name_(name), x_(x), y_(y), alive_(true) {}

void NPC::setPosition(int x, int y) {
    x_ = x;
    y_ = y;
}

void NPC::kill() {
    alive_ = false;
}

int NPC::rollDice() const {
    static thread_local std::uniform_int_distribution<int> dist(1, 6);
    return dist(rng_);
}

double NPC::distanceTo(const NPC& other) const {
    int dx = x_ - other.x_;
    int dy = y_ - other.y_;
    return std::sqrt(dx * dx + dy * dy);
}

std::string NPC::typeToString(Type type) {
    switch (type) {
        case Type::Knight: return "Knight";
        case Type::Squirrel: return "Squirrel";
        case Type::Pegasus: return "Pegasus";
        default: return "Unknown";
    }
}
