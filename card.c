#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "card.h"

Card* loadCards(s32* nCards)
{
    // Open file
    FILE* file = fopen("cards", "r");
    if (!file) {
        puts("No cards file found");
        exit(EXIT_FAILURE);
    }

    s32 n = 0;
    Card *pCards = malloc(sizeof(Card));

    // Read file
    while (!feof(file))
    {
        Card card;
        card.status     = ACTIV;
        card.magicTimer = 0.0f;

        char buffer[40], faction[20];

        // Name
        fscanf(file, "Nom : %s\n", buffer);
        strncpy(card.name, buffer, 20);

        // Faction
        fscanf(file, "Faction : %s\n", faction);
        if(faction[0] == 'Y') card.faction = YAZAKI;

        // Type
        fscanf(file, "Type : %s\n", buffer);
        if      (buffer[0] == 'G') card.type = GENERAL;
        else if (buffer[0] == 'U') card.type = MOB;
        else if (buffer[0] == 'M') card.type = MAGIC;

        // Stats & nb frames
        if (card.type == GENERAL || card.type == MOB) {
            fscanf(file, "Stats : %i/%i/%i/%i/%i\n", &card.health, &card.damages, &card.mana, &card.reach, &card.dmgFrame);
            fscanf(file, "Frames : %i/%i\n", &card.idleTex.nFrames, &card.attackTex.nFrames);
        } else { // Mana, effect & nb frames
            fscanf(file, "Mana : %i\n", &card.mana);

            fscanf(file, "Effet : %s\n", buffer);
            if      (buffer[0] == 'D') card.effect = DISPEL;
            else if (buffer[0] == 'B') card.effect = BUFF_DAMAGES;

            fscanf(file, "Frames : %i\n", &card.attackTex.nFrames);
        }

        // Icon
        strncpy(buffer, faction, 20);
        strncat(buffer, "_", 40);
        strncat(buffer, card.name, 40);
        strncat(buffer, "_Icon.png", 40);

        card.iconTex = loadTexture(buffer);

        // Spell animation
        if (card.type == MAGIC) {
            strncpy(buffer, faction, 20);
            strncat(buffer, "_", 40);
            strncat(buffer, card.name, 40);
            strncat(buffer, ".png", 40);

            card.attackTex.texture = loadTexture(buffer);
            card.attackTex.currFrame = 0;
            card.attackTex.oneOverNFrames = 1.0f / (f32)card.attackTex.nFrames;
        } else {
            // Idle animation
            strncpy(buffer, faction, 20);
            strncat(buffer, "_", 40);
            strncat(buffer, card.name, 40);
            strncat(buffer, "_Idle.png", 40);

            card.idleTex.texture = loadTexture(buffer);
            card.idleTex.currFrame = 0;
            card.idleTex.oneOverNFrames = 1.0f / (f32)card.idleTex.nFrames;

            // Attack animation
            strncpy(buffer, faction, 20);
            strncat(buffer, "_", 40);
            strncat(buffer, card.name, 40);
            strncat(buffer, "_Attack.png", 40);

            card.attackTex.texture = loadTexture(buffer);
            card.attackTex.currFrame = 0;
            card.attackTex.oneOverNFrames = 1.0f / (f32)card.attackTex.nFrames;
        }

        n += 1;
        pCards = realloc(pCards, n * sizeof(Card));
        pCards[n - 1] = card;
    }

    fclose(file);

    *nCards = n;
    return pCards;
}
