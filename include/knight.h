#pragma once

#include "npc.h"

class Knight : public NPC {
public:
    Knight(const std::string& name, int x, int y);

    int getMovementRange() const override { return 30; }
    int getKillRange() const override { return 10; }
    
    int getAttackBonus() const override { return 1; }
    int getDefenseBonus() const override { return 1; }
};
