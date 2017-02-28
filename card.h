#pragma once
#include "common.h"
#include "image.h"

typedef enum {
    NO_EFFECT,
    DISPEL,
    BUFF_DAMAGES
} Effect;

typedef struct {
    char name[20];
    enum {
        ACTIV,
        INACTIV,
        PLAYED
    } status;
    enum {
        YAZAKI
    } faction;
    enum {
        GENERAL = 0,
        MOB     = 1,
        MAGIC   = 2,
    } type;
    Effect effect;

    s32 health, damages, mana, reach, dmgFrame;
    f32 magicTimer;

    Texture     iconTex;
    TextureAnim idleTex, attackTex;
} Card;

Card* loadCards(s32* nCards);
