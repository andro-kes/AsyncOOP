#pragma once

#include "npc.h"

class Squirrel : public NPC {
public:
    Squirrel(const std::string& name, int x, int y);

    int getMovementRange() const override { return 5; }
    int getKillRange() const override { return 5; }
    
    int getAttackBonus() const override { return 0; }
    int getDefenseBonus() const override { return 1; }
};
