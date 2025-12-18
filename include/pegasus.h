#pragma once

#include "npc.h"

class Pegasus : public NPC {
public:
    Pegasus(const std::string& name, int x, int y);

    int getMovementRange() const override { return 30; }
    int getKillRange() const override { return 10; }
    
    int getAttackBonus() const override { return 0; }
    int getDefenseBonus() const override { return 2; }
};
