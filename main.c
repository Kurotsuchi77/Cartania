#include "app.h"

s32 main()
{
    App pApp;

    initializeApp(&pApp, 1440, 900, TRUE);

    initializeRenderer(&pApp);

    loadTextures(&pApp);

    loadLabels(&pApp);

    loadGameBoard(&pApp);

    initializeGame(&pApp);

    runMainLoop(&pApp);

    return 0;
}

