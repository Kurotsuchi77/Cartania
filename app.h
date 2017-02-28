#pragma once
#include "common.h"
#include "image.h"
#include "label.h"
#include "card.h"
#include "mob.h"

typedef struct {
    GLFWwindow  *pWindow;
    s32         xres, yres;
    vec2        cursorPos;
    bool        running;

    GLFWcursor  *cursors[21];
    enum {
        CURSOR_IDLE,
        CURSOR_MOVE,
        CURSOR_ATTACK
    }           cursorAnim;
    s32         cursorCurrFrame;
    f32         cursorAnimTimer;
    bool attackInProgress;
    GLuint      overlayPrg;
    GLint       overlayPrgOneOverNFrames, overlayPrgCurrFrame, overlayPrgMirror, overlayPrgRes, overlayPrgPos, overlayPrgSize, overlayPrgCol;

    GLuint      labelPrg;
    GLint       labelPrgPos, labelPrgCol;

    GLuint      overlayVao;

    Font        font12, font16, font32;

    Texture     background, playerPicBg, playerManaBg, yazakiIcon, cardIconBg, endTurnButton, cardManaBg, mobValueBg, mobDamageBg, mobCircle;
    Texture     cardBgs[3];
    Texture     hexs[4], hexAlpha;

    Label       *pLabels;
    s32          nLabels;

    Mob         *pMobs;
    s32          nMobs;

    s32         nDeadMobs;

    enum {
        PLAYER_TURN,
        ENEMY_WAIT,
        ENEMY_TURN
    } currGameState;

    f32 aiWaitTimer;

    struct {
        s32 value;
        f32 timer;
        vec2 pos;
    }           pDamages[5];
    s32         iDamages;

    f32         idleAnimTimer;

    s32         hexsIds[55];
    vec2        hexsPos[55];

    Card        pDeck[40], pAiDeck[40], pHand[7], pAiHand[7];
    s32         iDeck, iAiDeck;

    struct {
        s32 handIndex, mobIndex;
        vec2 pos;
    }           pAiActions[30];
    s32         iAiActions, nAiActions;

    s32         playerMana, enemyMana;

    enum {
        SELECTED_NONE,
        SELECTED_HAND,
        SELECTED_MOB
    }           selected;
    s32         selectedIndex;
} App;

void initializeApp(App *pApp, s32 xres, s32 yres, bool fullscreen);

void keyCallback(GLFWwindow* pWindow, s32 key, s32 scanCode, s32 action, s32 mode);

void mouseButtonCallback(GLFWwindow* window, s32 button, s32 action, s32 mode);

void initializeRenderer(App *pApp);

void loadTextures(App *pApp);

void loadLabels(App *pApp);

void loadGameBoard(App *pApp);

void initializeGame(App *pApp);

void runMainLoop(App *pApp);

void renderInterface(App *pApp);

void renderGameBoard(App *pApp);

void renderMobs(App *pApp, f32 deltaTime);

void renderHand(App *pApp);

void renderLabels(App* pApp);

void renderOverlay(App *pApp, Texture texture, vec2 pos);

void renderAnimOverlay(App *pApp, TextureAnim anim, vec2 pos, bool mirror);

void renderLabel(App *pApp, Label *pLabel);

void renderCard(App *pApp, Card card, vec2 pos);

void renderReach(App *pApp, vec2 pos, s32 reach, vec4 color);

void addLabel(App *pApp, Label label);

bool hover(vec2 mousePos, vec2 pos, f32 radius);
