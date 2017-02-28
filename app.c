#include "app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "program.h"
#include "image.h"

void initializeApp(App *pApp, s32 xres, s32 yres, bool fullscreen)
{
    srand(time(NULL));

    // Initialize GLFW
    if (!glfwInit()) {
        puts("Error initializing GLFW");
        exit(EXIT_FAILURE);
    }

    // Configure window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Open window
    pApp->xres = xres;
    pApp->yres = yres;
    pApp->pWindow = glfwCreateWindow(xres, yres, "Cartania", fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
    if (!pApp->pWindow) {
        puts("Error opening window");
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(pApp->pWindow);

    // Callbacks
    glfwSetWindowUserPointer(pApp->pWindow, pApp);

    glfwSetKeyCallback(pApp->pWindow, keyCallback);
    glfwSetMouseButtonCallback(pApp->pWindow, mouseButtonCallback);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        puts("Error initializing GLEW");
        exit(EXIT_FAILURE);
    }

    // Cursors
    char *cursorPaths[21] = {
        "Cursor.png",
        "CursorAttack_1.png",
        "CursorAttack_2.png",
        "CursorAttack_3.png",
        "CursorAttack_4.png",
        "CursorAttack_5.png",
        "CursorAttack_6.png",
        "CursorAttack_7.png",
        "CursorMove_1.png",
        "CursorMove_2.png",
        "CursorMove_3.png",
        "CursorMove_4.png",
        "CursorMove_5.png",
        "CursorMove_6.png",
        "CursorMove_7.png",
        "CursorMove_8.png",
        "CursorMove_9.png",
        "CursorMove_10.png",
        "CursorMove_11.png",
        "CursorMove_12.png",
        "CursorMove_13.png"
    };

    for (s32 i = 0; i < 21; i++)
    {
        Image image = loadPng(cursorPaths[i]);

        GLFWimage glfwImage;
        glfwImage.width     = image.xres;
        glfwImage.height    = image.yres;
        glfwImage.pixels    = image.pixels;

        pApp->cursors[i]  = glfwCreateCursor(&glfwImage, 5, 6) ;
    }

    glfwSetCursor(pApp->pWindow, pApp->cursors[0]);

    pApp->cursorAnim        = CURSOR_IDLE;
    pApp->cursorCurrFrame   = 0;
    pApp->cursorAnimTimer   = 0.0f;

     // OpenGL
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void keyCallback(GLFWwindow* pWindow, s32 key, s32 scanCode, s32 action, s32 mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(pWindow, GL_TRUE);
}

void mouseButtonCallback(GLFWwindow* pWindow, s32 button, s32 action, s32 mode)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)
        return;

    App *pApp = (App*)glfwGetWindowUserPointer(pWindow);

    // Hand selection
    for (s32 i = 0; i < 7; i++)
    {
        if (pApp->pHand[i].status == ACTIV && hover(pApp->cursorPos, v2New(-0.5f + (1.0f/6.0f)*i, -0.65f), 0.07f))
        {
            pApp->selected      = SELECTED_HAND;
            pApp->selectedIndex = i;
            return;
        }
    }

    // Play magic card
    if (pApp->currGameState == PLAYER_TURN &&
        pApp->selected == SELECTED_HAND &&
        pApp->pHand[pApp->selectedIndex].type == MAGIC)
    {
        for (s32 i = 0; i < pApp->nMobs; i++)
        {
            if (hover(pApp->cursorPos, pApp->pMobs[i].pos, 0.05f))
            {
                if (pApp->pHand[pApp->selectedIndex].effect == BUFF_DAMAGES &&
                    pApp->pMobs[i].owner == OWNER_PLAYER &&
                    pApp->pMobs[i].card.type == GENERAL &&
                    pApp->pMobs[i].effect == NO_EFFECT &&
                    pApp->playerMana >= pApp->pHand[pApp->selectedIndex].mana)
                {
                    pApp->pHand[pApp->selectedIndex].status = PLAYED;
                    pApp->pHand[pApp->selectedIndex].health = i;

                    pApp->playerMana -= pApp->pHand[pApp->selectedIndex].mana;
                    pApp->selected = SELECTED_NONE;
                    return;
                }

                else if (   pApp->pHand[pApp->selectedIndex].effect == DISPEL &&
                            pApp->pMobs[i].owner == OWNER_ENEMY &&
                            pApp->pMobs[i].effect == BUFF_DAMAGES &&
                            pApp->playerMana >= pApp->pHand[pApp->selectedIndex].mana)
                {
                    pApp->pHand[pApp->selectedIndex].status = PLAYED;
                    pApp->pHand[pApp->selectedIndex].health = i;

                    pApp->playerMana -= pApp->pHand[pApp->selectedIndex].mana;
                    pApp->selected = SELECTED_NONE;
                    return;
                }
            }
        }
    }

    // Mob selection && mob attack
    for (s32 i = 0; i < pApp->nMobs; i++)
    {
        if (hover(pApp->cursorPos, pApp->pMobs[i].pos, 0.05f))
        {
            if (pApp->currGameState == PLAYER_TURN &&
                pApp->selected == SELECTED_MOB &&
                pApp->pMobs[pApp->selectedIndex].owner == OWNER_PLAYER &&
                pApp->pMobs[i].owner == OWNER_ENEMY &&
                v2Len(v2Sub(pApp->pMobs[pApp->selectedIndex].pos, pApp->pMobs[i].pos)) < 0.15f &&
                pApp->pMobs[pApp->selectedIndex].readyToAttack)
            {
                if (pApp->attackInProgress)
                    return;
                else pApp->attackInProgress = TRUE;

                for (s32 n = 0; n < pApp->nMobs; n++)
                {
                    if (pApp->pMobs[n].attacking == ATTACKING_FIRST && pApp->pMobs[n].attackTarget == i)
                        return;
                }

                pApp->pMobs[pApp->selectedIndex].readyToAttack  = FALSE;
                pApp->pMobs[pApp->selectedIndex].attacking      = TRUE;
                pApp->pMobs[pApp->selectedIndex].attackTarget   = i;
                pApp->pMobs[pApp->selectedIndex].attackTimer    = 0.0f;
            }
            else
            {
                // Selection
                pApp->selected      = SELECTED_MOB;
                pApp->selectedIndex = i;
            }

            return;
        }
    }
    if (pApp->currGameState != PLAYER_TURN)
    {
        pApp->selected = SELECTED_NONE;
        return;
    }

    // Mob move
    if (pApp->selected == SELECTED_MOB &&
        pApp->pMobs[pApp->selectedIndex].owner == OWNER_PLAYER &&
        pApp->pMobs[pApp->selectedIndex].attacking == FALSE)
    {
        for (s32 i = 0; i < 55; i++)
        {
            if (hover(pApp->cursorPos, pApp->hexsPos[i], 0.05f))
            {
                f32 len = v2Len(v2Sub(pApp->hexsPos[i], pApp->pMobs[pApp->selectedIndex].pos));

                if ((len < 0.15f * (f32)pApp->pMobs[pApp->selectedIndex].card.reach) && (len > 0.05f) &&
                    pApp->pMobs[pApp->selectedIndex].readyToMove)
                {
                    for (s32 n = 0; n < pApp->nMobs; n++)
                        if (v2Equal(pApp->hexsPos[i], pApp->pMobs[n].destination))
                            return;

                    pApp->pMobs[pApp->selectedIndex].destination = pApp->hexsPos[i];
                    pApp->pMobs[pApp->selectedIndex].readyToMove = FALSE;
                }
                return;
            }
        }
    }

    // Play mob card
    if (pApp->selected == SELECTED_HAND &&
        pApp->pHand[pApp->selectedIndex].type == MOB &&
        pApp->playerMana >= pApp->pHand[pApp->selectedIndex].mana)
    {
        for (s32 i = 0; i < 55; i++)
        {
            if (hover(pApp->cursorPos, pApp->hexsPos[i], 0.05f))
            {
                if (v2Len(v2Sub(pApp->hexsPos[i], pApp->pMobs[GENERAL].destination)) < 0.15f)
                {
                    for (s32 n = 0; n < pApp->nMobs; n++)
                        if (v2Equal(pApp->hexsPos[i], pApp->pMobs[n].destination))
                            return;
                    pApp->nMobs++;
                    pApp->pMobs = realloc(pApp->pMobs, pApp->nMobs * sizeof(Mob));
                    pApp->pMobs[pApp->nMobs - 1] = (Mob){
                        pApp->pHand[pApp->selectedIndex], pApp->hexsPos[i], pApp->hexsPos[i],
                        OWNER_PLAYER, FALSE, FALSE, ATTACKING_FALSE, 0, 0.0f, NO_EFFECT, NO_EFFECT
                    };

                    pApp->pHand[pApp->selectedIndex].status = INACTIV;
                    pApp->playerMana -= pApp->pHand[pApp->selectedIndex].mana;
                }
                return;
            }
        }
    }

    // End Turn
    if (hover(pApp->cursorPos, v2New(0.8f, -0.65f), 0.08f))
    {
        pApp->currGameState = ENEMY_WAIT;
        pApp->playerMana++;
        pApp->enemyMana++;

        for (s32 i = 0; i < pApp->nMobs; i++)
            pApp->pMobs[i].readyToMove = pApp->pMobs[i].readyToAttack = TRUE;

        if (pApp->iDeck < 40)
        {
            for (s32 i = 0; i < 7; i++)
            {
                if (pApp->pHand[i].status == INACTIV)
                {
                    pApp->pHand[i] = pApp->pDeck[pApp->iDeck++];
                    return;
                }
            }
        }
        return;
    }

    pApp->selected = SELECTED_NONE;
}

void initializeRenderer(App *pApp)
{
    // Load overlay program
    pApp->overlayPrg = glCreateProgram();

    glBindAttribLocation(pApp->overlayPrg, 0, "vPosition");
    glBindAttribLocation(pApp->overlayPrg, 1, "vTexCoords");

    loadProgram(pApp->overlayPrg, "overlay.vs", "overlay.fs");

    pApp->overlayPrgOneOverNFrames  = findUniform(pApp->overlayPrg, "oneOverNFrames");
    pApp->overlayPrgCurrFrame       = findUniform(pApp->overlayPrg, "currFrame");
    pApp->overlayPrgMirror          = findUniform(pApp->overlayPrg, "mirror");
    pApp->overlayPrgRes             = findUniform(pApp->overlayPrg, "res");
    pApp->overlayPrgPos             = findUniform(pApp->overlayPrg, "position");
    pApp->overlayPrgSize            = findUniform(pApp->overlayPrg, "size");
    pApp->overlayPrgCol             = findUniform(pApp->overlayPrg, "color");

    initializeProgramSampler(pApp->overlayPrg, "sColor", 0);

    glUniform2f(pApp->overlayPrgRes, 1.0f/pApp->xres, 1.0f/pApp->yres);

    // Load label program
    pApp->labelPrg = glCreateProgram();

    glBindAttribLocation(pApp->labelPrg, 0, "vCoords");

    loadProgram(pApp->labelPrg, "label.vs", "label.fs");

    pApp->labelPrgPos = findUniform(pApp->labelPrg, "position");
    pApp->labelPrgCol = findUniform(pApp->labelPrg, "color");

    initializeProgramSampler(pApp->labelPrg, "sCharacters", 0);

    // Load overlay geometry
    const f32 vertices[] = {
        -1.0f, -1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &pApp->overlayVao);
    glBindVertexArray(pApp->overlayVao);

    GLuint overlayVbo;
    glGenBuffers(1, &overlayVbo);
    glBindBuffer(GL_ARRAY_BUFFER, overlayVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(f32), (GLvoid*)(0*sizeof(f32)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(f32), (GLvoid*)(2*sizeof(f32)));

    // Fonts
    pApp->font12 = loadFont("Font.otf", 18, 2.0f/(f32)pApp->yres);
    pApp->font16 = loadFont("Font.otf", 28, 2.0f/(f32)pApp->yres);
    pApp->font32 = loadFont("Font.otf", 52, 2.0f/(f32)pApp->yres);
}

void loadTextures(App *pApp)
{
    pApp->background    = loadTexture("Background.png");
    pApp->playerPicBg   = loadTexture("PlayerPicBg.png");
    pApp->playerManaBg  = loadTexture("PlayerManaBg.png");
    pApp->yazakiIcon    = loadTexture("Yazaki_Icon.png");
    pApp->cardIconBg    = loadTexture("CardIconBg.png");
    pApp->endTurnButton = loadTexture("EndTurnButton.png");
    pApp->cardManaBg    = loadTexture("CardManaBg.png");
    pApp->mobValueBg    = loadTexture("MobValueBg.png");
    pApp->mobDamageBg   = loadTexture("MobDamageBg.png");
    pApp->mobCircle     = loadTexture("MobCircle.png");

    pApp->cardBgs[GENERAL]  = loadTexture("CardGeneral.png");
    pApp->cardBgs[MOB]      = loadTexture("CardMob.png");
    pApp->cardBgs[MAGIC]    = loadTexture("CardMagic.png");

    pApp->hexs[0] = loadTexture("Hex.png");
    pApp->hexs[1] = loadTexture("HexMana.png");
    pApp->hexs[2] = loadTexture("HexBlue.png");
    pApp->hexs[3] = loadTexture("HexRed.png");

    pApp->hexAlpha = loadTexture("HexAlpha.png");
}

void loadLabels(App *pApp)
{
    pApp->pLabels = malloc(sizeof(Label));
    pApp->nLabels = 0;
}

void loadGameBoard(App *pApp)
{
    for (s32 i = 0; i < 55; i++)
        pApp->hexsIds[i] = 0;

    pApp->hexsIds[ 3] = 1;
    pApp->hexsIds[39] = 2;
    pApp->hexsIds[44] = 3;

    vec2 a = v2New( 0.1f, 0.12f);
    vec2 b = v2New(-0.1f, 0.12f);
    vec2 c = v2New( 0.0f, 0.06f);

    pApp->hexsPos[ 0] = v2Mul(a, v2New(0.0f, -3.0f));
    pApp->hexsPos[ 1] = v2Mul(a, v2New(0.0f, -2.0f));
    pApp->hexsPos[ 2] = v2Mul(a, v2New(0.0f, -1.0f));
    pApp->hexsPos[ 3] = v2Mul(a, v2New(0.0f,  0.0f));
    pApp->hexsPos[ 4] = v2Mul(a, v2New(0.0f,  1.0f));
    pApp->hexsPos[ 5] = v2Mul(a, v2New(0.0f,  2.0f));
    pApp->hexsPos[ 6] = v2Mul(a, v2New(0.0f,  3.0f));

    pApp->hexsPos[ 7] = v2Add(v2Mul(a, v2New(1.0f, -3.0f)), c);
    pApp->hexsPos[ 8] = v2Add(v2Mul(a, v2New(1.0f, -2.0f)), c);
    pApp->hexsPos[ 9] = v2Add(v2Mul(a, v2New(1.0f, -1.0f)), c);
    pApp->hexsPos[10] = v2Add(v2Mul(a, v2New(1.0f,  0.0f)), c);
    pApp->hexsPos[11] = v2Add(v2Mul(a, v2New(1.0f,  1.0f)), c);
    pApp->hexsPos[12] = v2Add(v2Mul(a, v2New(1.0f,  2.0f)), c);

    pApp->hexsPos[13] = v2Add(v2Mul(b, v2New(1.0f, -3.0f)), c);
    pApp->hexsPos[14] = v2Add(v2Mul(b, v2New(1.0f, -2.0f)), c);
    pApp->hexsPos[15] = v2Add(v2Mul(b, v2New(1.0f, -1.0f)), c);
    pApp->hexsPos[16] = v2Add(v2Mul(b, v2New(1.0f,  0.0f)), c);
    pApp->hexsPos[17] = v2Add(v2Mul(b, v2New(1.0f,  1.0f)), c);
    pApp->hexsPos[18] = v2Add(v2Mul(b, v2New(1.0f,  2.0f)), c);

    pApp->hexsPos[19] = v2Mul(a, v2New(2.0f, -2.0f));
    pApp->hexsPos[20] = v2Mul(a, v2New(2.0f, -1.0f));
    pApp->hexsPos[21] = v2Mul(a, v2New(2.0f,  0.0f));
    pApp->hexsPos[22] = v2Mul(a, v2New(2.0f,  1.0f));
    pApp->hexsPos[23] = v2Mul(a, v2New(2.0f,  2.0f));

    pApp->hexsPos[24] = v2Mul(b, v2New(2.0f, -2.0f));
    pApp->hexsPos[25] = v2Mul(b, v2New(2.0f, -1.0f));
    pApp->hexsPos[26] = v2Mul(b, v2New(2.0f,  0.0f));
    pApp->hexsPos[27] = v2Mul(b, v2New(2.0f,  1.0f));
    pApp->hexsPos[28] = v2Mul(b, v2New(2.0f,  2.0f));

    pApp->hexsPos[29] = v2Add(v2Mul(a, v2New(3.0f, -2.0f)), c);
    pApp->hexsPos[30] = v2Add(v2Mul(a, v2New(3.0f, -1.0f)), c);
    pApp->hexsPos[31] = v2Add(v2Mul(a, v2New(3.0f,  0.0f)), c);
    pApp->hexsPos[32] = v2Add(v2Mul(a, v2New(3.0f,  1.0f)), c);

    pApp->hexsPos[33] = v2Add(v2Mul(b, v2New(3.0f, -2.0f)), c);
    pApp->hexsPos[34] = v2Add(v2Mul(b, v2New(3.0f, -1.0f)), c);
    pApp->hexsPos[35] = v2Add(v2Mul(b, v2New(3.0f,  0.0f)), c);
    pApp->hexsPos[36] = v2Add(v2Mul(b, v2New(3.0f,  1.0f)), c);

    pApp->hexsPos[37] = v2Mul(a, v2New(4.0f, -2.0f));
    pApp->hexsPos[38] = v2Mul(a, v2New(4.0f, -1.0f));
    pApp->hexsPos[39] = v2Mul(a, v2New(4.0f,  0.0f));
    pApp->hexsPos[40] = v2Mul(a, v2New(4.0f,  1.0f));
    pApp->hexsPos[41] = v2Mul(a, v2New(4.0f,  2.0f));

    pApp->hexsPos[42] = v2Mul(b, v2New(4.0f, -2.0f));
    pApp->hexsPos[43] = v2Mul(b, v2New(4.0f, -1.0f));
    pApp->hexsPos[44] = v2Mul(b, v2New(4.0f,  0.0f));
    pApp->hexsPos[45] = v2Mul(b, v2New(4.0f,  1.0f));
    pApp->hexsPos[46] = v2Mul(b, v2New(4.0f,  2.0f));

    pApp->hexsPos[47] = v2Add(v2Mul(a, v2New(5.0f, -2.0f)), c);
    pApp->hexsPos[48] = v2Add(v2Mul(a, v2New(5.0f, -1.0f)), c);
    pApp->hexsPos[49] = v2Add(v2Mul(a, v2New(5.0f,  0.0f)), c);
    pApp->hexsPos[50] = v2Add(v2Mul(a, v2New(5.0f,  1.0f)), c);

    pApp->hexsPos[51] = v2Add(v2Mul(b, v2New(5.0f, -2.0f)), c);
    pApp->hexsPos[52] = v2Add(v2Mul(b, v2New(5.0f, -1.0f)), c);
    pApp->hexsPos[53] = v2Add(v2Mul(b, v2New(5.0f,  0.0f)), c);
    pApp->hexsPos[54] = v2Add(v2Mul(b, v2New(5.0f,  1.0f)), c);
}

void initializeGame(App *pApp)
{
    pApp->attackInProgress = FALSE;

    s32   nCards;
    Card *pCards = loadCards(&nCards);

    if (nCards < 2) {
        puts("Not enough cards");
        exit(EXIT_FAILURE);
    }

    // Initialize player & ai decks
    for (s32 i = 0; i < 40; i++) {
        pApp->pDeck[i] = pCards[randInRange(1, nCards - 1)];
        pApp->pAiDeck[i] = pCards[randInRange(1, nCards - 1)];
    }


    // Initialize player & ai hands
    pApp->iDeck = pApp->iAiDeck = 0;
    for (s32 i = 0; i < 6; i++) {
        pApp->pHand[i] = pApp->pDeck[pApp->iDeck++];
        pApp->pAiHand[i] = pApp->pAiDeck[pApp->iAiDeck++];
    }
    pApp->pHand[6].status = INACTIV;
    pApp->pAiHand[6].status = INACTIV;


    // Initialize mobs
    pApp->nMobs = 2;
    pApp->pMobs = malloc(pApp->nMobs * sizeof(Mob));
    pApp->pMobs[0] = (Mob){
        pCards[0], v2New(-0.4,  0.0f), v2New(-0.4,  0.0f), OWNER_PLAYER, TRUE, TRUE, ATTACKING_FALSE, 0, 0.0f, NO_EFFECT, NO_EFFECT
    };
    pApp->pMobs[1] = (Mob){
        pCards[0], v2New( 0.4,  0.0f), v2New( 0.4,  0.0f), OWNER_ENEMY,  TRUE, TRUE, ATTACKING_FALSE, 0, 0.0f, NO_EFFECT, NO_EFFECT
    };

    free(pCards);

    pApp->running = TRUE;

    pApp->playerMana = pApp->enemyMana = 10;

    pApp->selected = SELECTED_NONE;

    for (s32 i = 0; i < 5; i++)
        pApp->pDamages[i].value = 0;
    pApp->iDamages = 0;

    pApp->nDeadMobs = 0;

    pApp->currGameState = PLAYER_TURN;

    pApp->aiWaitTimer = 0.0f;
}

void runMainLoop(App *pApp)
{
    f32 currTime, deltaTime, lastTime = (f32)glfwGetTime();

    while (pApp->running && !glfwWindowShouldClose(pApp->pWindow))
    {
        // Time
        currTime    = (f32)glfwGetTime();
        deltaTime   = currTime - lastTime;
        lastTime    = currTime;

        // Cursor position
        f64 xpos, ypos;
        glfwGetCursorPos(pApp->pWindow, &xpos, &ypos);
        pApp->cursorPos = v2New(((f32)xpos/(f32)pApp->xres)*2.0f - 1.0f, (((f32)ypos/(f32)pApp->yres)*2.0f - 1.0f) * -1.0f);

        // Cursor animation
        bool cursorMove = FALSE, cursorAttack = FALSE;
        if (pApp->selected == SELECTED_MOB &&
            pApp->pMobs[pApp->selectedIndex].owner == OWNER_PLAYER)
        {
            if (pApp->pMobs[pApp->selectedIndex].readyToMove)
            {
                for (s32 i = 0; i < 55; i++)
                {
                    if (hover(pApp->cursorPos, pApp->hexsPos[i], 0.05f))
                    {
                        f32 len = v2Len(v2Sub(pApp->hexsPos[i], pApp->pMobs[pApp->selectedIndex].pos));

                        if ((len < 0.15f * (f32)pApp->pMobs[pApp->selectedIndex].card.reach) && (len > 0.05f))
                        {
                            cursorMove = TRUE;

                            for (s32 n = 0; n < pApp->nMobs; n++)
                                if (v2Equal(pApp->hexsPos[i], pApp->pMobs[n].destination))
                                    cursorMove = FALSE;

                        }
                        else cursorMove = FALSE;
                    }
                }
            }
            if (pApp->pMobs[pApp->selectedIndex].readyToAttack)
            {
                for (s32 i = 0; i < pApp->nMobs; i++)
                {
                    if (hover(pApp->cursorPos, pApp->pMobs[i].pos, 0.05f) &&
                        pApp->pMobs[i].owner == OWNER_ENEMY &&
                        v2Len(v2Sub(pApp->pMobs[pApp->selectedIndex].pos, pApp->pMobs[i].pos)) < 0.15f)
                    {
                        cursorAttack = TRUE;
                    }
                }
            }
        }

        if (cursorAttack)
        {
            if (pApp->cursorAnim != CURSOR_ATTACK)
            {
                pApp->cursorAnim        = CURSOR_ATTACK;
                pApp->cursorCurrFrame   = 0;
                pApp->cursorAnimTimer   = 0.0f;

                glfwSetCursor(pApp->pWindow, pApp->cursors[1 + pApp->cursorCurrFrame]);
            }

            pApp->cursorAnimTimer += deltaTime;
            if (pApp->cursorAnimTimer > 0.1f && pApp->cursorCurrFrame < 6)
            {
                pApp->cursorAnimTimer = 0.0f;
                pApp->cursorCurrFrame++;

                glfwSetCursor(pApp->pWindow, pApp->cursors[1 + pApp->cursorCurrFrame]);
            }
        }
        else if (cursorMove)
        {
            if (pApp->cursorAnim != CURSOR_MOVE)
            {
                pApp->cursorAnim        = CURSOR_MOVE;
                pApp->cursorCurrFrame   = 0;
                pApp->cursorAnimTimer   = 0.0f;

                glfwSetCursor(pApp->pWindow, pApp->cursors[8 + pApp->cursorCurrFrame]);
            }

            pApp->cursorAnimTimer += deltaTime;
            if (pApp->cursorAnimTimer > 0.1f)
            {
                pApp->cursorAnimTimer = 0.0f;

                if (++pApp->cursorCurrFrame == 13)
                      pApp->cursorCurrFrame = 0;

                glfwSetCursor(pApp->pWindow, pApp->cursors[8 + pApp->cursorCurrFrame]);
            }
        }
        else if (pApp->cursorAnim != CURSOR_IDLE)
        {
            pApp->cursorAnim = CURSOR_IDLE;
            glfwSetCursor(pApp->pWindow, pApp->cursors[0]);
        }

        // AI
        if (pApp->currGameState == ENEMY_WAIT)
        {
            pApp->aiWaitTimer += deltaTime;

            if (pApp->aiWaitTimer > 3.0f)
            {
                pApp->aiWaitTimer = 0.0f;
                pApp->currGameState = ENEMY_TURN;

                if (pApp->iAiDeck < 40)
                {
                    for (s32 i = 0; i < 7; i++)
                    {
                        if (pApp->pAiHand[i].status != ACTIV)
                        {
                            pApp->pAiHand[i] = pApp->pAiDeck[pApp->iAiDeck++];
                            break;
                        }
                    }
                }

                pApp->nAiActions = pApp->iAiActions = 0;

                s32 currMana = pApp->enemyMana;

                for (s32 i = 0; i < 7; i++)
                {
                    if (pApp->pAiHand[i].status == INACTIV || currMana < pApp->pAiHand[i].mana)
                        continue;

                    if (pApp->pAiHand[i].type == MAGIC)
                    {
                        if (pApp->pAiHand[i].effect == BUFF_DAMAGES)
                        {
                            for (s32 j = 0; j < pApp->nMobs; j++)
                            {
                                if (pApp->pMobs[j].card.type == GENERAL &&
                                    pApp->pMobs[j].owner == OWNER_ENEMY &&
                                    pApp->pMobs[j].effect == NO_EFFECT &&
                                    pApp->pMobs[j].aiEffect == NO_EFFECT)
                                {
                                    pApp->pMobs[j].aiEffect = BUFF_DAMAGES;

                                    currMana -= pApp->pAiHand[i].mana;

                                    pApp->pAiActions[pApp->nAiActions].handIndex    = i;
                                    pApp->pAiActions[pApp->nAiActions].mobIndex     = j;
                                    pApp->nAiActions++;
                                    break;
                                }
                            }
                        }
                        else if (pApp->pAiHand[i].effect == DISPEL)
                        {
                            for (s32 j = 0; j < pApp->nMobs; j++)
                            {
                                if (pApp->pMobs[j].card.type == GENERAL &&
                                    pApp->pMobs[j].owner == OWNER_PLAYER &&
                                    pApp->pMobs[j].effect == BUFF_DAMAGES &&
                                    pApp->pMobs[j].aiEffect == BUFF_DAMAGES)
                                {
                                    pApp->pMobs[j].aiEffect = NO_EFFECT;

                                    currMana -= pApp->pAiHand[i].mana;

                                    pApp->pAiActions[pApp->nAiActions].handIndex    = i;
                                    pApp->pAiActions[pApp->nAiActions].mobIndex     = j;
                                    pApp->nAiActions++;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        for (s32 j = 0; j < 55; j++)
                        {
                            bool avaliable = FALSE;
                            if (v2Len(v2Sub(pApp->hexsPos[j], pApp->pMobs[1].destination)) < 0.15f)
                            {
                                avaliable = TRUE;
                                for (s32 k = 0; k < pApp->nMobs; k++)
                                {
                                    if (v2Equal(pApp->hexsPos[j], pApp->pMobs[k].destination))
                                    {
                                        avaliable = FALSE;
                                        break;
                                    }
                                }

                                for (s32 k = 0; k < pApp->nAiActions; k++)
                                {
                                    if (v2Equal(pApp->hexsPos[j], pApp->pAiActions[k].pos))
                                    {
                                        avaliable = FALSE;
                                        break;
                                    }
                                }

                            }

                            if (avaliable)
                            {
                                currMana -= pApp->pAiHand[i].mana;

                                pApp->pAiActions[pApp->nAiActions].handIndex    = i;
                                pApp->pAiActions[pApp->nAiActions].pos          = pApp->hexsPos[j];
                                pApp->nAiActions++;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (pApp->currGameState == ENEMY_TURN)
        {
            if (pApp->nAiActions == 0) pApp->currGameState = PLAYER_TURN;
            else
            {
                if (pApp->pAiHand[pApp->pAiActions[pApp->iAiActions].handIndex].type == MAGIC)
                {
                    pApp->pAiHand[pApp->pAiActions[pApp->iAiActions].handIndex].status = PLAYED;
                    pApp->pAiHand[pApp->pAiActions[pApp->iAiActions].handIndex].health = pApp->pAiActions[pApp->iAiActions].mobIndex;
                }
                else
                {
                    pApp->nMobs++;
                    pApp->pMobs = realloc(pApp->pMobs, pApp->nMobs * sizeof(Mob));
                    pApp->pMobs[pApp->nMobs - 1] = (Mob){
                        pApp->pAiHand[pApp->pAiActions[pApp->iAiActions].handIndex],
                        pApp->pAiActions[pApp->iAiActions].pos, pApp->pAiActions[pApp->iAiActions].pos,
                        OWNER_ENEMY, FALSE, FALSE, ATTACKING_FALSE, 0, 0.0f, NO_EFFECT, NO_EFFECT
                    };
                }

                pApp->enemyMana -= pApp->pHand[pApp->pAiActions[pApp->iAiActions].handIndex].mana;

                pApp->iAiActions++;
                if (pApp->iAiActions == pApp->nAiActions)
                    pApp->currGameState = PLAYER_TURN;

            }
        }

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(pApp->overlayPrg);
        glUniform4f(pApp->overlayPrgCol, 1.0f, 1.0f, 1.0f, 1.0f);
        glBindVertexArray(pApp->overlayVao);

        renderInterface(pApp);
        renderGameBoard(pApp);

        if (pApp->selected == SELECTED_HAND)
        {
            renderCard(pApp, pApp->pHand[pApp->selectedIndex], v2New(-0.8f, 0.0f));

            if (pApp->pHand[pApp->selectedIndex].type == MOB)
                renderReach(pApp, pApp->pMobs[GENERAL].destination, 1, v4New(1.0f, 1.0f, 0.0f, 1.0f));
        }
        else if (pApp->selected == SELECTED_MOB)
        {
            renderCard(pApp, pApp->pMobs[pApp->selectedIndex].card, pApp->pMobs[pApp->selectedIndex].owner == OWNER_PLAYER ?
                       v2New(-0.8f, 0.0f) : v2New(0.8f, 0.0f));

            if (pApp->pMobs[pApp->selectedIndex].owner == OWNER_PLAYER && pApp->pMobs[pApp->selectedIndex].readyToMove)
                renderReach(pApp, pApp->pMobs[pApp->selectedIndex].pos,
                            pApp->pMobs[pApp->selectedIndex].card.reach, v4New(0.5f, 1.0f, 0.5f, 1.0f));
        }

        glBindVertexArray(pApp->overlayVao);

        renderMobs(pApp, deltaTime);

        // Played magic cards
        for (s32 i = 0; i < 7; i++)
        {
            if (pApp->pHand[i].status == PLAYED)
            {
                pApp->pHand[i].magicTimer += deltaTime;
                if (pApp->pHand[i].magicTimer > 0.1f)
                {
                    pApp->pHand[i].magicTimer = 0.0f;
                    pApp->pHand[i].attackTex.currFrame++;

                    if (pApp->pHand[i].attackTex.currFrame == pApp->pHand[i].attackTex.nFrames)
                    {
                        if (pApp->pHand[i].effect == BUFF_DAMAGES)
                        {
                            pApp->pMobs[pApp->pHand[i].health].effect = BUFF_DAMAGES;
                            pApp->pMobs[pApp->pHand[i].health].aiEffect = BUFF_DAMAGES;
                            pApp->pMobs[pApp->pHand[i].health].card.damages += 2;
                        }
                        else if (pApp->pHand[i].effect == DISPEL)
                        {
                            pApp->pMobs[pApp->pHand[i].health].effect = NO_EFFECT;
                            pApp->pMobs[pApp->pHand[i].health].aiEffect = NO_EFFECT;
                            pApp->pMobs[pApp->pHand[i].health].card.damages -= 2;
                        }

                        pApp->pHand[i].attackTex.currFrame = 0;
                            pApp->pHand[i].status = INACTIV;
                    }
                }

                renderAnimOverlay(
                    pApp, pApp->pHand[i].attackTex,
                    v2Add(pApp->pMobs[pApp->pHand[i].health].pos, v2New(0.0f, 0.2f)), FALSE
                );
            }
        }

        // Played ai card
        for (s32 i = 0; i < 7; i++)
        {
            if (pApp->pAiHand[i].status == PLAYED)
            {
                pApp->pAiHand[i].magicTimer += deltaTime;
                if (pApp->pAiHand[i].magicTimer > 0.1f)
                {
                    pApp->pAiHand[i].magicTimer = 0.0f;
                    pApp->pAiHand[i].attackTex.currFrame++;

                    if (pApp->pAiHand[i].attackTex.currFrame == pApp->pAiHand[i].attackTex.nFrames)
                    {
                        if (pApp->pAiHand[i].effect == BUFF_DAMAGES)
                        {
                            pApp->pMobs[pApp->pAiHand[i].health].effect = BUFF_DAMAGES;
                            pApp->pMobs[pApp->pAiHand[i].health].aiEffect = BUFF_DAMAGES;
                            pApp->pMobs[pApp->pAiHand[i].health].card.damages += 2;
                        }
                        else if (pApp->pAiHand[i].effect == DISPEL)
                        {
                            pApp->pMobs[pApp->pAiHand[i].health].effect = NO_EFFECT;
                            pApp->pMobs[pApp->pAiHand[i].health].aiEffect = NO_EFFECT;
                            pApp->pMobs[pApp->pAiHand[i].health].card.damages -= 2;
                        }

                        pApp->pAiHand[i].attackTex.currFrame = 0;
                        pApp->pAiHand[i].status = INACTIV;
                    }
                }

                renderAnimOverlay(
                    pApp, pApp->pAiHand[i].attackTex,
                    v2Add(pApp->pMobs[pApp->pAiHand[i].health].pos, v2New(0.0f, 0.2f)), FALSE
                );
            }
        }

        renderHand(pApp);

        addLabel(pApp, newLabel(
            v2New(2.0f/(f32)pApp->xres, 2.0f/(f32)pApp->yres), &pApp->font32, "PLAYER",
            ALIGN_LEFT, v2New(-0.45f, 0.75f), v4New(0.5f, 1.0f, 0.5f, 1.0f)
        ));
        addLabel(pApp, newLabel(
            v2New(2.0f/(f32)pApp->xres, 2.0f/(f32)pApp->yres), &pApp->font32, "ENEMY",
            ALIGN_RIGHT, v2New(0.45f, 0.75f), v4New(1.0f, 0.5f, 0.5f, 1.0f)
        ));

        char str[10];
        snprintf(str, 10, "%d / %d", 40 - pApp->iDeck, 40);
        addLabel(pApp, newLabel(
            v2New(2.0f/(f32)pApp->xres, 2.0f/(f32)pApp->yres), &pApp->font16, str,
            ALIGN_CENTER, v2New(-0.8f, -0.6f), v4New(1.0f, 1.0f, 1.0f, 1.0f)
        ));

        renderLabels(pApp);

        // Dead mobs
        if (pApp->nDeadMobs > 0)
        {
            Mob *pMobsCpy = malloc(pApp->nMobs * sizeof(Mob));
            memcpy(pMobsCpy, pApp->pMobs, pApp->nMobs * sizeof(Mob));

            pApp->pMobs = realloc(pApp->pMobs, (pApp->nMobs - pApp->nDeadMobs) * sizeof(Mob));

            for (s32 i = 0, n = 0; i < pApp->nMobs; i++)
            {
                if (pMobsCpy[i].card.health > 0)
                    pApp->pMobs[n++] = pMobsCpy[i];
                else if (pMobsCpy[i].card.type == GENERAL)
                    pApp->running = FALSE;
                else if (pApp->selected == SELECTED_MOB && pApp->selectedIndex == i)
                    pApp->selected = SELECTED_NONE;
            }

            free(pMobsCpy);
            pApp->nMobs -= pApp->nDeadMobs;
            pApp->nDeadMobs = 0;
        }

        glfwSwapBuffers(pApp->pWindow);
        glfwPollEvents();
    }
}

void renderInterface(App *pApp)
{
    renderOverlay(pApp, pApp->background, v2New(0.0f, 0.0f));

    renderOverlay(pApp, pApp->playerPicBg, v2New(-0.6f, 0.75f));
    renderOverlay(pApp, pApp->playerPicBg, v2New( 0.6f, 0.75f));

    renderOverlay(pApp, pApp->playerManaBg, v2New(-0.4f, 0.65f));
    renderOverlay(pApp, pApp->playerManaBg, v2New( 0.4f, 0.65f));

    renderOverlay(pApp, pApp->yazakiIcon, v2New(-0.55f, 0.62f));
    renderOverlay(pApp, pApp->yazakiIcon, v2New( 0.55f, 0.62f));

    if (pApp->currGameState == PLAYER_TURN)
    {
        if (hover(pApp->cursorPos, v2New(0.8f, -0.65f), 0.1f))
        {
            glUniform4f(pApp->overlayPrgCol, 0.5f, 1.0f, 0.5f, 1.0f);
            renderOverlay(pApp, pApp->endTurnButton, v2New(0.8f, -0.6f));
            glUniform4f(pApp->overlayPrgCol, 1.0f, 1.0f, 1.0f, 1.0f);

            addLabel(pApp, newLabel(
                v2New(2.0f/(f32)pApp->xres, 2.0f/(f32)pApp->yres), &pApp->font16, "End Turn",
                ALIGN_CENTER, v2New(0.8f, -0.62f), v4New(0.5f, 1.0f, 0.5f, 1.0f)
            ));
        }
        else
        {
            renderOverlay(pApp, pApp->endTurnButton, v2New(0.8f, -0.6f));

            addLabel(pApp, newLabel(
                v2New(2.0f/(f32)pApp->xres, 2.0f/(f32)pApp->yres), &pApp->font16, "End Turn",
                ALIGN_CENTER, v2New(0.8f, -0.62f), v4New(0.8f, 0.8f, 0.8f, 1.0f)
            ));
        }
    }
    else
    {
        glUniform4f(pApp->overlayPrgCol, 1.0f, 1.0f, 0.5f, 1.0f);
        renderOverlay(pApp, pApp->endTurnButton, v2New(0.8f, -0.6f));
        glUniform4f(pApp->overlayPrgCol, 1.0f, 1.0f, 1.0f, 1.0f);

        addLabel(pApp, newLabel(
            v2New(2.0f/(f32)pApp->xres, 2.0f/(f32)pApp->yres), &pApp->font16, "Enemy Turn",
            ALIGN_CENTER, v2New(0.8f, -0.62f), v4New(1.0f, 1.0f, 0.5f, 1.0f)
        ));
    }

    glBindVertexArray(pApp->overlayVao);

    for (s32 i = 0; i < 7; i++)
        renderOverlay(pApp, pApp->cardIconBg, v2New(-0.5f + (1.0f/6.0f)*i, -0.65f));

    addLabel(pApp, newIntegerLabel(
            v2New(2.0f/(f32)pApp->xres, 2.0f/(f32)pApp->yres), &pApp->font16, pApp->playerMana,
            ALIGN_CENTER, v2New(-0.4f, 0.65f - 0.018f), v4New(0.75f, 0.75f, 1.0f, 1.0f)
    ));
    addLabel(pApp, newIntegerLabel(
            v2New(2.0f/(f32)pApp->xres, 2.0f/(f32)pApp->yres), &pApp->font16, pApp->enemyMana,
            ALIGN_CENTER, v2New( 0.4f, 0.65f - 0.018f), v4New(0.75f, 0.75f, 1.0f, 1.0f)
    ));
    glBindVertexArray(pApp->overlayVao);
}

void renderGameBoard(App *pApp)
{
    for (s32 i = 0; i < 55; i++)
        renderOverlay(pApp, pApp->hexs[pApp->hexsIds[i]], pApp->hexsPos[i]);
}

void renderMobs(App *pApp, f32 deltaTime)
{
    const f32 mobSpeed = 0.15f;

    pApp->idleAnimTimer += deltaTime;
    if (pApp->idleAnimTimer > 0.5f)
    {
        pApp->idleAnimTimer = 0.0f;
        for (s32 i = 0; i < pApp->nMobs; i++)
        {
            pApp->pMobs[i].card.idleTex.currFrame++;
            if (pApp->pMobs[i].card.idleTex.currFrame == pApp->pMobs[i].card.idleTex.nFrames)
                pApp->pMobs[i].card.idleTex.currFrame = 0;
        }
    }

    for (s32 i = 0; i < pApp->nMobs; i++)
    {
        if (pApp->pMobs[i].owner == OWNER_ENEMY)
            glUniform4f(pApp->overlayPrgCol, 1.0f, 0.5f, 0.5f, 1.0f);
        else
            glUniform4f(pApp->overlayPrgCol, 0.5f, 1.0f, 0.5f, 1.0f);

        renderOverlay(pApp, pApp->mobCircle, pApp->pMobs[i].pos);
        glUniform4f(pApp->overlayPrgCol, 1.0f, 1.0f, 1.0f, 1.0f);

        // Attacking
        if (pApp->pMobs[i].attacking != ATTACKING_FALSE)
        {
            pApp->pMobs[i].attackTimer += deltaTime;
            if (pApp->pMobs[i].attackTimer > 0.1f)
            {
                pApp->pMobs[i].attackTimer = 0.0f;
                pApp->pMobs[i].card.attackTex.currFrame++;

                if (pApp->pMobs[i].card.attackTex.currFrame == pApp->pMobs[i].card.dmgFrame)
                {
                    pApp->pMobs[pApp->pMobs[i].attackTarget].card.health -= pApp->pMobs[i].card.damages;

                    if (pApp->pMobs[pApp->pMobs[i].attackTarget].card.health < 0)
                        pApp->pMobs[pApp->pMobs[i].attackTarget].card.health = 0;

                    pApp->iDamages++;
                    if (pApp->iDamages == 5) pApp->iDamages = 0;
                    pApp->pDamages[pApp->iDamages].value = pApp->pMobs[i].card.damages;
                    pApp->pDamages[pApp->iDamages].timer = 0.0f;
                    pApp->pDamages[pApp->iDamages].pos = v2Add(pApp->pMobs[pApp->pMobs[i].attackTarget].pos, v2New(0.0f, 0.05f));
                }
                else if (pApp->pMobs[i].card.attackTex.currFrame == pApp->pMobs[i].card.attackTex.nFrames)
                {
                    if ((pApp->pMobs[i].attacking == ATTACKING_FIRST) && (pApp->pMobs[pApp->pMobs[i].attackTarget].card.health > 0))
                    {
                        pApp->pMobs[pApp->pMobs[i].attackTarget].attacking      = ATTACKING_SECOND;
                        pApp->pMobs[pApp->pMobs[i].attackTarget].attackTarget   = i;
                        pApp->pMobs[pApp->pMobs[i].attackTarget].attackTimer    = 0.0f;
                    }

                    if (pApp->pMobs[i].attacking == ATTACKING_SECOND || pApp->pMobs[pApp->pMobs[i].attackTarget].card.health == 0)
                        pApp->attackInProgress = FALSE;

                    if (pApp->pMobs[pApp->pMobs[i].attackTarget].card.health == 0)
                        pApp->nDeadMobs++;

                    pApp->pMobs[i].attacking                = ATTACKING_FALSE;
                    pApp->pMobs[i].card.attackTex.currFrame = 0;
                }
            }

            renderAnimOverlay(pApp, pApp->pMobs[i].card.attackTex,
                              v2Add(pApp->pMobs[i].pos, v2New(0.0f, 0.05f)), pApp->pMobs[i].owner == OWNER_PLAYER);
        }
        else
        {
            if (!v2Equal(pApp->pMobs[i].pos, pApp->pMobs[i].destination))
            {
                vec2 dir = v2Sub(pApp->pMobs[i].destination, pApp->pMobs[i].pos);
                vec2 vel = v2Mulf(v2Normalize(dir), deltaTime * mobSpeed);
                if (v2Len(vel) > v2Len(dir))
                    pApp->pMobs[i].pos = pApp->pMobs[i].destination;
                else
                    pApp->pMobs[i].pos = v2Add(pApp->pMobs[i].pos, vel);
            }

            if (hover(pApp->cursorPos, pApp->pMobs[i].pos, 0.05f))
            {
                // OUTLINE
            }

            renderAnimOverlay(pApp, pApp->pMobs[i].card.idleTex,
                              v2Add(pApp->pMobs[i].pos, v2New(0.0f, 0.05f)), pApp->pMobs[i].owner == OWNER_PLAYER);
        }

        renderOverlay(pApp, pApp->mobValueBg, v2Add(pApp->pMobs[i].pos, v2New(-0.03f, -0.03f)));
        renderOverlay(pApp, pApp->mobValueBg, v2Add(pApp->pMobs[i].pos, v2New( 0.03f, -0.03f)));

        addLabel(pApp, newIntegerLabel(
            v2New(2.0f/(f32)pApp->xres, 2.0f/(f32)pApp->yres), &pApp->font12, pApp->pMobs[i].card.damages,
            ALIGN_CENTER, v2Add(pApp->pMobs[i].pos, v2New(-0.03f, -0.04f)), v4New(1.0f, 0.5f, 0.5f, 1.0f)
        ));
        addLabel(pApp, newIntegerLabel(
            v2New(2.0f/(f32)pApp->xres, 2.0f/(f32)pApp->yres), &pApp->font12, pApp->pMobs[i].card.health,
            ALIGN_CENTER, v2Add(pApp->pMobs[i].pos, v2New( 0.03f, -0.04f)), v4New(0.5f, 1.0f, 0.5f, 1.0f)
        ));
        glBindVertexArray(pApp->overlayVao);
    }

    for (s32 i = 0; i < 5; i++)
    {
        if (!pApp->pDamages[i].value) continue;

        pApp->pDamages[i].timer += deltaTime;
        if(pApp->pDamages[i].timer > 1.0f)
            pApp->pDamages[i].value = 0;
        else
        {
            renderOverlay(pApp, pApp->mobDamageBg, pApp->pDamages[i].pos);
            addLabel(pApp, newIntegerLabel(
                v2New(2.0f/(f32)pApp->xres, 2.0f/(f32)pApp->yres), &pApp->font16, pApp->pDamages[i].value,
                ALIGN_CENTER, v2Add(pApp->pDamages[i].pos, v2New(0.0f, -0.015f)), v4New(1.0f, 0.2f, 0.2f, 1.0f)
            ));
        }
    }

    renderLabels(pApp);

    glUseProgram(pApp->overlayPrg);
    glUniform4f(pApp->overlayPrgCol, 1.0f, 1.0f, 1.0f, 1.0f);
    glBindVertexArray(pApp->overlayVao);
}

void renderHand(App *pApp)
{
    for (s32 i = 0; i < 7; i++)
    {
        if (pApp->pHand[i].status != ACTIV) continue;

        // Icon
        vec2 pos = v2New(-0.5f + (1.0f/6.0f)*i, -0.65f);

        if (hover(pApp->cursorPos, pos, 0.07f)) {
            Texture iconTex = pApp->pHand[i].iconTex;
            iconTex.size = v2Mulf(iconTex.size, 1.25f);
            renderOverlay(pApp, iconTex, pos);

            renderCard(pApp, pApp->pHand[i], v2Add(pos, v2New(0.0f, 0.5f)));
        } else
            renderOverlay(pApp, pApp->pHand[i].iconTex, pos);

        // Mana background
        renderOverlay(pApp, pApp->cardManaBg, v2New(pos.x, pos.y - 0.1f));

        addLabel(pApp, newIntegerLabel(
            v2New(2.0f/(f32)pApp->xres, 2.0f/(f32)pApp->yres), &pApp->font16, pApp->pHand[i].mana,
            ALIGN_CENTER, v2New(pos.x, pos.y - 0.118f), v4New(0.75f, 0.75f, 1.0f, 1.0f)
        ));
        glBindVertexArray(pApp->overlayVao);
    }
}

void renderLabels(App* pApp)
{
    glUseProgram(pApp->labelPrg);

    for (s32 i = 0; i < pApp->nLabels; i++)
    {
        renderLabel(pApp, &pApp->pLabels[i]);

        glDeleteVertexArrays(1, &pApp->pLabels[i].vao);
        glDeleteBuffers(1, &pApp->pLabels[i].vbo);
    }

    pApp->pLabels = realloc(pApp->pLabels, sizeof(Label));
    pApp->nLabels = 0;
}

void renderOverlay(App *pApp, Texture texture, vec2 pos)
{
    glUniform1f(pApp->overlayPrgOneOverNFrames, 1.0f);
    glUniform1f(pApp->overlayPrgCurrFrame, 0.0f);
    glUniform1f(pApp->overlayPrgMirror, 1.0f);
    glUniform2f(pApp->overlayPrgPos, pos.x, pos.y);
    glUniform2f(pApp->overlayPrgSize, texture.size.x, texture.size.y);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void renderAnimOverlay(App *pApp, TextureAnim anim, vec2 pos, bool mirror)
{
    glUniform1f(pApp->overlayPrgOneOverNFrames, anim.oneOverNFrames);
    glUniform1f(pApp->overlayPrgCurrFrame, (f32)anim.currFrame);
    glUniform1f(pApp->overlayPrgMirror, mirror ? -1.0f : 1.0f);
    glUniform2f(pApp->overlayPrgPos, pos.x, pos.y);
    glUniform2f(pApp->overlayPrgSize, anim.texture.size.x, anim.texture.size.y);
    glBindTexture(GL_TEXTURE_2D, anim.texture.id);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void renderLabel(App *pApp, Label *pLabel)
{
    glUniform2f(pApp->labelPrgPos, pLabel->anchorPosX, pLabel->pos.y);
    glUniform4f(pApp->labelPrgCol, pLabel->color.x, pLabel->color.y, pLabel->color.z, pLabel->color.w);
    glBindTexture(GL_TEXTURE_2D, pLabel->charTex);
    glBindVertexArray(pLabel->vao);
    glDrawArrays(GL_TRIANGLES, 0, pLabel->nVertices);
}

void renderCard(App *pApp, Card card, vec2 pos)
{
    // Overlays
    renderOverlay(pApp, pApp->cardBgs[card.type], pos);
    renderOverlay(pApp, pApp->yazakiIcon, v2Add(pos, v2New(0.0f, -0.08f)));
    renderOverlay(pApp, card.iconTex, v2Add(pos, v2New(0.0f, 0.2f)));

    // Labels
    addLabel(pApp, newLabel(
        v2New(2.0f/(f32)pApp->xres, 2.0f/(f32)pApp->yres), &pApp->font16, card.name,
        ALIGN_CENTER, v2Add(pos, v2New(0.0f, 0.06f)), v4New(0.8f, 0.8f, 1.0f, 1.0f)
    ));
    addLabel(pApp, newIntegerLabel(
        v2New(2.0f/(f32)pApp->xres, 2.0f/(f32)pApp->yres), &pApp->font16, card.mana,
        ALIGN_CENTER, v2Add(pos, v2New(0.0f, -0.005f)), v4New(1.0f, 1.0f, 1.0f, 1.0f)
    ));

    if (card.type != MAGIC)
    {
        addLabel(pApp, newIntegerLabel(
            v2New(2.0f/(f32)pApp->xres, 2.0f/(f32)pApp->yres), &pApp->font16, card.damages,
            ALIGN_CENTER, v2Add(pos, v2New(-0.03f, -0.05f)), v4New(1.0f, 1.0f, 1.0f, 1.0f)
        ));
        addLabel(pApp, newIntegerLabel(
            v2New(2.0f/(f32)pApp->xres, 2.0f/(f32)pApp->yres), &pApp->font16, card.health,
            ALIGN_CENTER, v2Add(pos, v2New(0.03f, -0.05f)), v4New(1.0f, 1.0f, 1.0f, 1.0f)
        ));
    }

    glBindVertexArray(pApp->overlayVao);
}

void renderReach(App *pApp, vec2 pos, s32 reach, vec4 color)
{
    glUniform4f(pApp->overlayPrgCol, color.x, color.y, color.z, color.w);

    for (s32 i = 0; i < 55; i++)
    {
        f32 len = v2Len(v2Sub(pApp->hexsPos[i], pos));

        if ((len < 0.15f * (f32)reach) && (len > 0.05f))
        {
            bool draw = TRUE;
            for (s32 n = 0; n < pApp->nMobs; n++)
                if (v2Equal(pApp->hexsPos[i], pApp->pMobs[n].destination))
                    draw = FALSE;

            if (draw) renderOverlay(pApp, pApp->hexAlpha, pApp->hexsPos[i]);
        }
    }
    glUniform4f(pApp->overlayPrgCol, 1.0f, 1.0f, 1.0f, 1.0f);

}

void addLabel(App *pApp, Label label)
{
    pApp->nLabels++;
    pApp->pLabels = realloc(pApp->pLabels, pApp->nLabels * sizeof(Label));
    pApp->pLabels[pApp->nLabels - 1] = label;
}

bool hover(vec2 mousePos, vec2 pos, f32 radius)
{
    const vec2 ar = v2New(1.0f, 10.0f/16.0f);

    return v2Len(v2Sub(v2Mul(mousePos, ar), v2Mul(pos, ar))) < radius;
}
