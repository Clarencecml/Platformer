/******************************************************************************/
/*!
\file		GameState_Asteroids.cpp
\author 	DigiPen
\par    	email: digipen\@digipen.edu
\date   	February 01, 2017
\brief

Copyright (C) 2017 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#include "main.h"

// Font to be used for printing
static u32 arial_Font;

// Countdown timer to quit game
float timer;

void GameStateMenuLoad(void)
{
   
    arial_Font = AEGfxCreateFont("Arial", 30, true, true);
    AEGfxSetBackgroundColor(0.f, 0.f, 0.f);
}

void GameStateMenuInit(void)
{
    timer = 0.f;
}

void GameStateMenuUpdate(void)
{
    if (AEInputCheckCurr(AEVK_1))
    {
        gGameStateNext = GS_PLATFORM;
    }
    if (AEInputCheckCurr(AEVK_2))
    {
        gGameStateNext = GS_PLATFORM2;
    }
    if (AEInputCheckCurr(AEVK_Q))
    {
        timer += g_dt;
        if (timer >= 1.f)
        {
            gGameStateNext = GS_QUIT;
        }
    }
    if (AEInputCheckReleased(AEVK_Q))
    {
        timer = 0.f;
    }
}

void GameStateMenuDraw(void)
{
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxTextureSet(NULL, 0, 0);
    AEGfxPrint(arial_Font, "WELCOME TO HOP EMPEROR", (s32)(AEGfxGetWinMinX() + 10.f), (s32)(AEGfxGetWinMinY() + 500), 1, 1, 0);
    AEGfxPrint(arial_Font, "Press '1' for level 1.",(s32) (AEGfxGetWinMinX() + 10.f), (s32) (AEGfxGetWinMinY() + 350), 1, 1, 0);
    AEGfxPrint(arial_Font, "Press '2' for level 2.",(s32) (AEGfxGetWinMinX() + 10.f), (s32) (AEGfxGetWinMinY() + 300), 1, 1, 0);
    AEGfxPrint(arial_Font, "Hold 'Q' to quit the game.", (s32) (AEGfxGetWinMinX() + 10.f), (s32) (AEGfxGetWinMinY() + 250), 1, 1, 0);
}

void GameStateMenuFree(void)
{

}

void GameStateMenuUnload(void)
{
    AEGfxDestroyFont(arial_Font);
}