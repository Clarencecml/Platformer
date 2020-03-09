/******************************************************************************/
/*!
\file		GameState_Asteroids.h
\author 	DigiPen
\par    	email: digipen\@digipen.edu
\date   	February 01, 2017
\brief

Copyright (C) 2017 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#ifndef CS230_GAME_STATE_PLAY_H_
#define CS230_GAME_STATE_PLAY_H_

// Include Random Generator
#include "Random.h"

// ---------------------------------------------------------------------------

void GameStatePlatformLoad(void);
void GameStatePlatformInit(void);
void GameStatePlatformUpdate(void);
void GameStatePlatformDraw(void);

void GameStatePlatformLoad_1(void);
void GameStatePlatformInit_1(void);
void GameStatePlatformDraw_1(void);


void GameStatePlatformLoad_2(void);
void GameStatePlatformInit_2(void);
void GameStatePlatformDraw_2(void);

void GameStatePlatformFree(void);
void GameStatePlatformUnload(void);
// ---------------------------------------------------------------------------

#endif // CS230_GAME_STATE_PLAY_H_