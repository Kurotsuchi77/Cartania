#pragma once
#include "common.h"
#include "card.h"

typedef struct {
    Card card;

    vec2 pos, destination;

    enum {
        OWNER_PLAYER,
        OWNER_ENEMY
    } owner;

    bool readyToMove, readyToAttack;

    enum {
        ATTACKING_FALSE,
        ATTACKING_FIRST,
        ATTACKING_SECOND
    } attacking;
    s32 attackTarget;
    f32 attackTimer;

    Effect effect, aiEffect;
} Mob;
