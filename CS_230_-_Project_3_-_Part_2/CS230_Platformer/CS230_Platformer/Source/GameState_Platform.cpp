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
#include "iostream"
#include <string>
/******************************************************************************/
/*!
	Defines
*/
/******************************************************************************/
const unsigned int	GAME_OBJ_NUM_MAX		= 32;	//The total number of different objects (Shapes)
const unsigned int	GAME_OBJ_INST_NUM_MAX	= 2048;	//The total number of different game object instances

//Gameplay related variables and values
const float			GRAVITY					= -20.0f;
const float			JUMP_VELOCITY			= 11.0f;
const float			MOVE_VELOCITY_HERO		= 4.0f;
const float			MOVE_VELOCITY_ENEMY		= 7.5f;
const double		ENEMY_IDLE_TIME			= 2.0;
const int			HERO_LIVES				= 3;
const float			BOUNDING_RECT_SIZE		= 0.5f;			// default rectangle size

//Flags
const unsigned int	FLAG_ACTIVE				= 0x00000001;
const unsigned int	FLAG_VISIBLE			= 0x00000002;
const unsigned int	FLAG_NON_COLLIDABLE		= 0x00000004;

//Collision flags
const unsigned int	COLLISION_LEFT			= 0x00000001;	//0001
const unsigned int	COLLISION_RIGHT			= 0x00000002;	//0010
const unsigned int	COLLISION_TOP			= 0x00000004;	//0100
const unsigned int	COLLISION_BOTTOM		= 0x00000008;	//1000


enum class TYPE_OBJECT : unsigned int
{
	TYPE_OBJECT_EMPTY,			//0
	TYPE_OBJECT_COLLISION,		//1
	TYPE_OBJECT_HERO,			//2
	TYPE_OBJECT_ENEMY1,			//3
	TYPE_OBJECT_COIN,			//4
	TYPE_OBJECT_PARTICLE		//5
};

//State machine states
enum class STATE : unsigned int
{
	STATE_NONE,
	STATE_GOING_LEFT,
	STATE_GOING_RIGHT
};

//State machine inner states
enum class INNER_STATE : unsigned int
{
	INNER_STATE_ON_ENTER,
	INNER_STATE_ON_UPDATE,
	INNER_STATE_ON_EXIT
};


/******************************************************************************/
/*!
	Struct/Class Definitions
*/
/******************************************************************************/
struct GameObj
{
	unsigned int		type;		// object type
	AEGfxVertexList *	pMesh;		// pbject
};


struct GameObjInst
{
	GameObj *		pObject;	// pointer to the 'original'
	unsigned int	flag;		// bit flag or-ed together
	float			scale;
	AEVec2			posCurr;	// object current position
	AEVec2			velCurr;	// object current velocity
	float			dirCurr;	// object current direction

	AEMtx33			transform;	// object drawing matrix
	
	AABB			boundingBox;// object bouding box that encapsulates the object

	//Used to hold the current 
	int				gridCollisionFlag;

	// pointer to custom data specific for each object type
	void*			pUserData;

	//State of the object instance
	enum class		STATE state;
	enum class	 	INNER_STATE innerState;

	//General purpose counter (This variable will be used for the enemy state machine)
	double			counter;
};


/******************************************************************************/
/*!
	File globals
*/
/******************************************************************************/
static int				HeroLives;
static int				Hero_Initial_X;
static int				Hero_Initial_Y;
static int				TotalCoins;
static int				TotalParticles;
// list of original objects
static GameObj			*sGameObjList;
static unsigned int		sGameObjNum;

// list of object instances
static GameObjInst		*sGameObjInstList;
static unsigned int		sGameObjInstNum;

//Binary map data
static int				**MapData;
static int				**BinaryCollisionArray;
static int				BINARY_MAP_WIDTH;
static int				BINARY_MAP_HEIGHT;
static GameObjInst		*pBlackInstance;
static GameObjInst		*pWhiteInstance;
static GameObjInst		*pCoinInstance;
static GameObjInst		*pEnemyInstance;
static AEMtx33			MapTransform;

int						GetCellValue(int X, int Y);
int						CheckInstanceBinaryMapCollision(float PosX, float PosY, 
														float scaleX, float scaleY);
void					SnapToCell(float *Coordinate);
int						ImportMapDataFromFile(char *FileName);
void					FreeMapData(void);
void					cam_Update(); // To update camera
void					cam_Reset(); // To reset camera
// function to create/destroy a game object instance
static GameObjInst*		gameObjInstCreate (unsigned int type, float scale, 
											AEVec2* pPos, AEVec2* pVel, 
											float dir, enum class STATE startState);
static void				gameObjInstDestroy(GameObjInst* pInst);

//We need a pointer to the hero's instance for input purposes
static GameObjInst		*pHero;

//State machine functions
void					EnemyStateMachine(GameObjInst *pInst);

//To change lives displayed
static bool				onValueChange;

//Particle pointer
static GameObjInst*		pParticle;

// Font to be used for printing
static u32				arial_Font;


/******************************************************************************/
/*!

*/
/******************************************************************************/
void GameStatePlatformLoad(void)
{
	sGameObjList = (GameObj *)calloc(GAME_OBJ_NUM_MAX, sizeof(GameObj));
	sGameObjInstList = (GameObjInst *)calloc(GAME_OBJ_INST_NUM_MAX, sizeof(GameObjInst));
	sGameObjNum = 0;
	sGameObjInstNum = 0;

	arial_Font = AEGfxCreateFont("Arial", 30, true, true);

	GameObj* pObj;

	//Creating the black object
	pObj		= sGameObjList + sGameObjNum++;
	pObj->type	= (unsigned int)TYPE_OBJECT::TYPE_OBJECT_EMPTY;


	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f,
		 0.5f,  -0.5f, 0xFF000000, 0.0f, 0.0f, 
		-0.5f,  0.5f, 0xFF000000, 0.0f, 0.0f);
	
	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f,
		 0.5f,  -0.5f, 0xFF000000, 0.0f, 0.0f, 
		0.5f,  0.5f, 0xFF000000, 0.0f, 0.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");
		
	
	//Creating the white object
	pObj		= sGameObjList + sGameObjNum++;
	pObj->type	= (unsigned int)TYPE_OBJECT::TYPE_OBJECT_COLLISION;


	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		 0.5f,  -0.5f, 0xFFFFFFFF, 0.0f, 0.0f, 
		-0.5f,  0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	
	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f, 
		 0.5f,  -0.5f, 0xFFFFFFFF, 0.0f, 0.0f, 
		0.5f,  0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");


	//Creating the hero object
	pObj		= sGameObjList + sGameObjNum++;
	pObj->type	= (unsigned int)TYPE_OBJECT::TYPE_OBJECT_HERO;


	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFF0000FF, 0.0f, 0.0f, 
		 0.5f,  -0.5f, 0xFF0000FF, 0.0f, 0.0f, 
		-0.5f,  0.5f, 0xFF0000FF, 0.0f, 0.0f);
	
	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFF0000FF, 0.0f, 0.0f,
		 0.5f,  -0.5f, 0xFF0000FF, 0.0f, 0.0f, 
		0.5f,  0.5f, 0xFF0000FF, 0.0f, 0.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");


	//Creating the enemey1 object
	pObj		= sGameObjList + sGameObjNum++;
	pObj->type	= (unsigned int)TYPE_OBJECT::TYPE_OBJECT_ENEMY1;


	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f, 
		 0.5f,  -0.5f, 0xFFFF0000, 0.0f, 0.0f, 
		-0.5f,  0.5f, 0xFFFF0000, 0.0f, 0.0f);
	
	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFFFF0000, 0.0f, 0.0f, 
		 0.5f,  -0.5f, 0xFFFF0000, 0.0f, 0.0f, 
		0.5f,  0.5f, 0xFFFF0000, 0.0f, 0.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");


	//Creating the Coin object
	pObj		= sGameObjList + sGameObjNum++;
	pObj->type	= (unsigned int)TYPE_OBJECT::TYPE_OBJECT_COIN;


	AEGfxMeshStart();
	//Creating the circle shape
	int Parts = 12;
	for(float i = 0; i < Parts; ++i)
	{
		AEGfxTriAdd(
		0.0f, 0.0f, 0xFFFFFF00, 0.0f, 0.0f, 
		cosf(i*2*PI/Parts)*0.5f,  sinf(i*2*PI/Parts)*0.5f, 0xFFFFFF00, 0.0f, 0.0f, 
		cosf((i+1)*2*PI/Parts)*0.5f,  sinf((i+1)*2*PI/Parts)*0.5f, 0xFFFFFF00, 0.0f, 0.0f);
	}

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");

	//Creating the particle object
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = (unsigned int)TYPE_OBJECT::TYPE_OBJECT_PARTICLE;


	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFF23ACAD, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF23ACAD, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFF23ACAD, 0.0f, 0.0f);

	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");

	//Setting intital binary map values
	MapData = 0;
	BinaryCollisionArray = 0;
	BINARY_MAP_WIDTH = 0;
	BINARY_MAP_HEIGHT = 0;

	//Importing Data
	if (gGameStateCurr == GS_PLATFORM)
	{
		if (!ImportMapDataFromFile("Resources/Exported.txt"))
			gGameStateNext = GS_QUIT;
	}

	if (gGameStateCurr == GS_PLATFORM2)
	{
		if (!ImportMapDataFromFile("Resources/Exported2.txt"))
			gGameStateNext = GS_QUIT;
	}
	//Computing the matrix which take a point out of the normalized coordinates system
	//of the binary map
	/***********
	Compute a transformation matrix and save it in "MapTransform".
	This transformation transforms any point from the normalized coordinates system of the binary map.
	Later on, when rendering each object instance, we should concatenate "MapTransform" with the
	object instance's own transformation matrix

	Compute a translation matrix (-Grid width/2, -Grid height/2) and save it in "trans"
	Compute a scaling matrix and save it in "scale")
	Concatenate scale and translate and save the result in "MapTransform"
	***********/

	AEMtx33 scale, trans;

	// Compute the scaling matrix
	AEMtx33Scale(&scale, 40, 30);
	// Compute the translation matrix
	AEMtx33Trans(&trans, -(f32)(BINARY_MAP_WIDTH / 2.f), -(f32)(BINARY_MAP_HEIGHT / 2.f));
	// Concat the transoformation matrix
	AEMtx33Concat(&MapTransform, &scale, &trans);

}

void GameStatePlatformLoad_1(void)
{
	GameStatePlatformLoad();
}

void GameStatePlatformLoad_2(void)
{
	GameStatePlatformLoad();
}
/******************************************************************************/
/*!

*/
/******************************************************************************/
void GameStatePlatformInit(void)
{
	int i, j;

	pHero = 0;
	pBlackInstance = 0;
	pWhiteInstance = 0;
	pParticle = 0;
	TotalCoins = 0;
	TotalParticles = 0;
	//Create an object instance representing the black cell.
	//This object instance should not be visible. When rendering the grid cells, each time we have
	//a non collision cell, we position this instance in the correct location and then we render it
	pBlackInstance = gameObjInstCreate((unsigned int)TYPE_OBJECT::TYPE_OBJECT_EMPTY, 1.0f, 0, 0, 0.0f, STATE::STATE_NONE);
	pBlackInstance->flag ^= FLAG_VISIBLE;
	pBlackInstance->flag |= FLAG_NON_COLLIDABLE;

	//Create an object instance representing the white cell.
	//This object instance should not be visible. When rendering the grid cells, each time we have
	//a collision cell, we position this instance in the correct location and then we render it
	pWhiteInstance = gameObjInstCreate((unsigned int)TYPE_OBJECT::TYPE_OBJECT_COLLISION, 1.0f, 0, 0, 0.0f, STATE::STATE_NONE);
	pWhiteInstance->flag ^= FLAG_VISIBLE;
	pWhiteInstance->flag |= FLAG_NON_COLLIDABLE;


	//Setting the inital number of hero lives
	HeroLives = HERO_LIVES;



	AEVec2 Pos;

	// creating the main character, the enemies and the coins according 
	// to their initial positions in MapData

	/***********
	Loop through all the array elements of MapData
	(which was initialized in the "GameStatePlatformLoad" function
	from the .txt file
		if the element represents a collidable or non collidable area
			don't do anything

		if the element represents the hero
			Create a hero instance
			Set its position depending on its array indices in MapData
			Save its array indices in Hero_Initial_X and Hero_Initial_Y
			(Used when the hero dies and its position needs to be reset)

		if the element represents an enemy
			Create an enemy instance
			Set its position depending on its array indices in MapData

		if the element represents a coin
			Create a coin instance
			Set its position depending on its array indices in MapData

	***********/
	for (i = 0; i < BINARY_MAP_HEIGHT; ++i)
	{
		for (j = 0; j < BINARY_MAP_WIDTH; ++j)
		{
			f32 a = (f32)j;
			f32 b = (f32)i;
			int Type_unit = MapData[i][j];
			if (Type_unit == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_COLLISION || Type_unit == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_EMPTY)
			{
				continue;
			}
			if (Type_unit == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_HERO)
			{
				SnapToCell(&a);
				SnapToCell(&b);
				AEVec2Set(&Pos, a, b);
				pHero = gameObjInstCreate((unsigned int)TYPE_OBJECT::TYPE_OBJECT_HERO, 1.0f, &Pos, 0, 0.0f, STATE::STATE_NONE);
				Hero_Initial_X = j;
				Hero_Initial_Y = i;
			}
			else if (Type_unit == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_ENEMY1)
			{
				SnapToCell(&a);
				SnapToCell(&b);
				AEVec2Set(&Pos, a, b);
				pEnemyInstance = gameObjInstCreate((unsigned int)TYPE_OBJECT::TYPE_OBJECT_ENEMY1, 1.0f, &Pos, 0, 0.0f, STATE::STATE_GOING_LEFT);
			}
			else if (Type_unit == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_COIN)
			{
				SnapToCell(&a);
				SnapToCell(&b);
				AEVec2Set(&Pos, a, b);
				pCoinInstance = gameObjInstCreate((unsigned int)TYPE_OBJECT::TYPE_OBJECT_COIN, 1.0f, &Pos, 0, 0.0f, STATE::STATE_NONE);
				++TotalCoins;
			}
		}
	}

	// Set the value default to true
	onValueChange = true;
}

void GameStatePlatformInit_1(void)
{
	GameStatePlatformInit();
}

void GameStatePlatformInit_2(void)
{
	GameStatePlatformInit();
}
/******************************************************************************/
/*!

*/
/******************************************************************************/
void GameStatePlatformUpdate(void)
{
	int i;
	GameObjInst* pInst;
	
	//Handle Input
	/***********
	if right is pressed
		Set hero velocity X to MOVE_VELOCITY_HERO
	else
	if left is pressed
		Set hero velocity X to -MOVE_VELOCITY_HERO
	else
		Set hero velocity X to 0

	if space is pressed AND Hero is colliding from the bottom
		Set hero velocity Y to JUMP_VELOCITY

	if Escape is pressed
		Exit to menu
	***********/

	if (AEInputCheckCurr(AEVK_RIGHT))
	{
		pHero->velCurr.x = MOVE_VELOCITY_HERO;
	}
	else if (AEInputCheckCurr(AEVK_LEFT))
	{
		pHero->velCurr.x = -MOVE_VELOCITY_HERO;
	}
	else
	{
		pHero->velCurr.x = 0.0f;
	}
	if (gGameStateCurr == GS_PLATFORM)
	{
		if (AEInputCheckCurr(AEVK_SPACE) && pHero->gridCollisionFlag & COLLISION_BOTTOM)
		{
			pHero->velCurr.y = JUMP_VELOCITY;
		}
	}
	if (gGameStateCurr == GS_PLATFORM2)
	{
		if (AEInputCheckCurr(AEVK_SPACE) && pHero->gridCollisionFlag & COLLISION_BOTTOM)
		{
			pHero->velCurr.y = JUMP_VELOCITY + 4.f;
		}
	}
	if (AEInputCheckCurr(AEVK_Q))
	{
		gGameStateNext = GS_MENU;
	}


	if(TotalParticles < 400)
	{
		// Particle
		AEVec2 x{ (f32)Random_Range(-5,5),(f32)Random_Range(-5,5) };
		float dir = (float)Random_Range(-1, 1);
		// Two instance, one loop
		pParticle = gameObjInstCreate((unsigned int)TYPE_OBJECT::TYPE_OBJECT_PARTICLE, 0.08f, &pHero->posCurr, &x, dir, STATE::STATE_NONE);
		++TotalParticles;
		pParticle = gameObjInstCreate((unsigned int)TYPE_OBJECT::TYPE_OBJECT_PARTICLE, 0.08f, &pHero->posCurr, &x, dir, STATE::STATE_NONE);
		++TotalParticles;
	}


	//Update object instances physics and behavior
	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i)
	{
		pInst = sGameObjInstList + i;

		// skip non-active object
		if (0 == (pInst->flag & FLAG_ACTIVE))
			continue;

		/****************
		Apply gravity
			Velocity Y = Gravity * Frame Time + Velocity Y

		If object instance is an enemy
			Apply enemy state machine
		****************/
		if (pInst->pObject->type == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_HERO)
		{
			pInst->velCurr.y += GRAVITY * g_dt;
		}
		if (pInst->pObject->type == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_ENEMY1)
		{
			pInst->velCurr.y += GRAVITY * g_dt;
			EnemyStateMachine(pInst);
		}
		if (pInst->pObject->type == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_PARTICLE)
		{
			f32 random = (f32)Random_Range(1, 3);
			if (pInst->posCurr.x > pHero->posCurr.x + random || pInst->posCurr.y > pHero->posCurr.y + random ||
				pInst->posCurr.x < pHero->posCurr.x - random || pInst->posCurr.y < pHero->posCurr.y - random)
			{
				gameObjInstDestroy(pInst);
				--TotalParticles;
			}
		}
	}
	//Update object instances positions
	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i)
	{
		pInst = sGameObjInstList + i;

		// skip non-active object
		if (0 == (pInst->flag & FLAG_ACTIVE))
			continue;

		/**********
		update the position using: P1 = V1*dt + P0
		Get the bouding rectangle of every active instance:
			boundingRect_min = -BOUNDING_RECT_SIZE * instance->scale + instance->pos
			boundingRect_max = BOUNDING_RECT_SIZE * instance->scale + instance->pos
		**********/
		// Updates positions of all objects
		pInst->posCurr.x += pInst->velCurr.x * g_dt;
		pInst->posCurr.y += pInst->velCurr.y * g_dt;
		
		// Set the bounding boxes for all objects
		pInst->boundingBox.min.x = -BOUNDING_RECT_SIZE * pInst->scale + pInst->posCurr.x;
		pInst->boundingBox.min.y = -BOUNDING_RECT_SIZE * pInst->scale + pInst->posCurr.y;

		pInst->boundingBox.max.x = BOUNDING_RECT_SIZE * pInst->scale + pInst->posCurr.x;
		pInst->boundingBox.max.y = BOUNDING_RECT_SIZE * pInst->scale + pInst->posCurr.y;
	}
	//Check for grid collision
	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i)
	{
		pInst = sGameObjInstList + i;

		// skip non-active object instances
		if (0 == (pInst->flag & FLAG_ACTIVE))
			continue;

		/*************
		Update grid collision flag
		if collision from bottom
			Snap to cell on Y axis
			Velocity Y = 0

		if collision from top
			Snap to cell on Y axis
			Velocity Y = 0

		if collision from left
			Snap to cell on X axis
			Velocity X = 0

		if collision from right
			Snap to cell on X axis
			Velocity X = 0
		*************/
		if (pInst->pObject->type == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_HERO || pInst->pObject->type == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_ENEMY1)
		{
			pInst->gridCollisionFlag = CheckInstanceBinaryMapCollision(pInst->posCurr.x, pInst->posCurr.y, pInst->scale, pInst->scale);
			if (pInst->gridCollisionFlag & COLLISION_BOTTOM)
			{
				SnapToCell(&pInst->posCurr.y);
				pInst->velCurr.y = 0.0f;
			}
			if (pInst->gridCollisionFlag & COLLISION_TOP)
			{
				SnapToCell(&pInst->posCurr.y);
				pInst->velCurr.y = 0.0f;
			}
			if (pInst->gridCollisionFlag & COLLISION_LEFT)
			{
				SnapToCell(&pInst->posCurr.x);
				pInst->velCurr.x = 0.0f;
			}
			if (pInst->gridCollisionFlag & COLLISION_RIGHT)
			{
				SnapToCell(&pInst->posCurr.x);
				pInst->velCurr.x = 0.0f;
			}
		}
	}


	//Checking for collision among object instances:
	//Hero against enemies
	//Hero against coins

	/**********
	for each game object instance
		Skip if it's inactive or if it's non collidable

		If it's an enemy
			If collision between the enemy instance and the hero (rectangle - rectangle)
				Decrement hero lives
				Reset the hero's position in case it has lives left, otherwise RESTART the level
				  
		If it's a coin
			If collision between the coin instance and the hero (rectangle - rectangle)
				Remove the coin and decrement the coin counter.
				Quit the game level to the menu in case no more coins are left
	**********/
	
	for(i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i)
	{
		GameObjInst* pInst_1 = sGameObjInstList + i;
		// skip non-active object
		if ((pInst_1->flag & FLAG_ACTIVE) == 0)
			continue;

		if (pInst_1->pObject->type == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_ENEMY1)
		{
			if (CollisionIntersection_RectRect(pInst_1->boundingBox,
				pInst_1->velCurr, pHero->boundingBox, pHero->velCurr))
			{
				onValueChange = true;
				--HeroLives;
				if (HeroLives > 0)
				{	
					pHero->posCurr.x = (f32)Hero_Initial_X;
					SnapToCell(&pHero->posCurr.x);
					pHero->posCurr.y = (f32)Hero_Initial_Y;
					SnapToCell(&pHero->posCurr.y);
				}
				else
				{
					gGameStateNext = GS_RESTART;
				}
			}
		}
		if (pInst_1->pObject->type == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_COIN)
		{
			if (CollisionIntersection_RectRect(pInst_1->boundingBox,
				pInst_1->velCurr, pHero->boundingBox, pHero->velCurr))
			{
				gameObjInstDestroy(pInst_1);
				--TotalCoins;
				if (TotalCoins == 0)
				{
					gGameStateNext = GS_PLATFORM2;
				}
			}
		}
	}
	
	//Computing the transformation matrices of the game object instances
	for(i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i)
	{
		AEMtx33 scale, rot, trans,result;
		pInst = sGameObjInstList + i;

		// skip non-active object
		if (0 == (pInst->flag & FLAG_ACTIVE))
			continue;

		AEMtx33Scale(&scale, pInst->scale, pInst->scale);
		AEMtx33Rot(&rot, pInst->dirCurr);

		AEMtx33Trans(&trans, pInst->posCurr.x, pInst->posCurr.y);

		AEMtx33Concat(&result, &trans, &rot);
		AEMtx33Concat(&pInst->transform, &result, &scale);
	}

	if (gGameStateCurr == GS_PLATFORM2)
	{
		cam_Update();
	}
	if (gGameStateCurr == GS_PLATFORM)
	{
		cam_Reset();
	}
	
}

/******************************************************************************/
/*!

*/
/******************************************************************************/
void GameStatePlatformDraw(void)
{
	//Drawing the tile map (the grid)
	int i, j;
	AEMtx33 cellTranslation, cellFinalTransformation;

	//Drawing the tile map
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxTextureSet(NULL, 0, 0);
	/******REMINDER*****
	You need to concatenate MapTransform with the transformation matrix 
	of any object you want to draw. MapTransform transform the instance 
	from the normalized coordinates system of the binary map
	*******************/

	/*********
	for each array element in BinaryCollisionArray (2 loops)
		Compute the cell's translation matrix acoording to its 
		X and Y coordinates and save it in "cellTranslation"
		Concatenate MapTransform with the cell's transformation 
		and save the result in "cellFinalTransformation"
		Send the resultant matrix to the graphics manager using "AEGfxSetTransform"

		Draw the instance's shape depending on the cell's value using "AEGfxMeshDraw"
			Use the black instance in case the cell's value is TYPE_OBJECT_EMPTY
			Use the white instance in case the cell's value is TYPE_OBJECT_COLLISION
	*********/
	for (i = 0; i < BINARY_MAP_HEIGHT; ++i)
	{
		for (j = 0; j < BINARY_MAP_WIDTH; ++j)
		{
			f32 a, b;
			a = (f32)j;
			b = (f32)i;
			SnapToCell(&a);
			SnapToCell(&b);

			AEMtx33Trans(&cellTranslation, a, b);
			AEMtx33Concat(&cellFinalTransformation, &MapTransform, &cellTranslation);
			AEGfxSetTransform(cellFinalTransformation.m);

			if (GetCellValue(j, i) == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_EMPTY)
			{
				AEGfxMeshDraw(pBlackInstance->pObject->pMesh, AE_GFX_MDM_TRIANGLES);
			}
			else if (GetCellValue(j, i) == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_COLLISION)
			{
				AEGfxMeshDraw(pWhiteInstance->pObject->pMesh, AE_GFX_MDM_TRIANGLES);
			}
		}
	}


	//Drawing the object instances
	/**********
	For each active and visible object instance
		Concatenate MapTransform with its transformation matrix
		Send the resultant matrix to the graphics manager using "AEGfxSetTransform"
		Draw the instance's shape using "AEGfxMeshDraw"
	**********/
	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst* pInst = sGameObjInstList + i;

		// skip non-active object
		if (0 == (pInst->flag & FLAG_ACTIVE) || 0 == (pInst->flag & FLAG_VISIBLE))
			continue;

		//Don't forget to concatenate the MapTransform matrix with the transformation of each game object instance
		AEMtx33Concat(&pInst->transform, &MapTransform, &pInst->transform);
		AEGfxSetTransform(pInst->transform.m);
		AEGfxMeshDraw(pInst->pObject->pMesh, AE_GFX_MDM_TRIANGLES);
	}
	
	AEGfxPrint(arial_Font, "Coins Left: ", (s32)(AEGfxGetWinMinX() + 10.f), (s32)(AEGfxGetWinMinY() + 575), 1, 0, 1);
	AEGfxPrint(arial_Font, "Lives: ", (s32)(AEGfxGetWinMinX() + 400.f), (s32)(AEGfxGetWinMinY() + 575), 1, 0, 1);
}

void GameStatePlatformDraw_1(void)
{
	GameStatePlatformDraw();
}

void GameStatePlatformDraw_2(void)
{
	GameStatePlatformDraw();
}

/******************************************************************************/
/*!

*/
/******************************************************************************/
void GameStatePlatformFree(void)
{
	// kill all object in the list
	for (unsigned int i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
		gameObjInstDestroy(sGameObjInstList + i);
}

/******************************************************************************/
/*!

*/
/******************************************************************************/
void GameStatePlatformUnload(void)
{
	// free all CREATED mesh
	for (u32 i = 0; i < sGameObjNum; i++)
		AEGfxMeshFree(sGameObjList[i].pMesh);
		
	free(sGameObjList);
	free(sGameObjInstList);

	/*********
	Free the map data
	*********/
	FreeMapData();
	
	// Destroy the font used
	AEGfxDestroyFont(arial_Font);
}

/******************************************************************************/
/*!

*/
/******************************************************************************/
GameObjInst* gameObjInstCreate(unsigned int type, float scale, 
							   AEVec2* pPos, AEVec2* pVel, 
							   float dir, enum class STATE startState)
{
	AEVec2 zero;
	AEVec2Zero(&zero);

	AE_ASSERT_PARM(type < sGameObjNum);
	
	// loop through the object instance list to find a non-used object instance
	for (unsigned int i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst* pInst = sGameObjInstList + i;

		// check if current instance is not used
		if (pInst->flag == 0)
		{
			// it is not used => use it to create the new instance
			pInst->pObject			 = sGameObjList + type;
			pInst->flag				 = FLAG_ACTIVE | FLAG_VISIBLE;
			pInst->scale			 = scale;
			pInst->posCurr			 = pPos ? *pPos : zero;
			pInst->velCurr			 = pVel ? *pVel : zero;
			pInst->dirCurr			 = dir;
			pInst->pUserData		 = 0;
			pInst->gridCollisionFlag = 0;
			pInst->state			 = startState;
			pInst->innerState		 = INNER_STATE::INNER_STATE_ON_ENTER;
			pInst->counter			 = 0;
			
			// return the newly created instance
			return pInst;
		}
	}

	return 0;
}

/******************************************************************************/
/*!

*/
/******************************************************************************/
void gameObjInstDestroy(GameObjInst* pInst)
{
	// if instance is destroyed before, just return
	if (pInst->flag == 0)
		return;

	// zero out the flag
	pInst->flag = 0;
}

/******************************************************************************/
/*!

*/
/******************************************************************************/
int GetCellValue(int X, int Y)
{
	if (X < BINARY_MAP_WIDTH && Y < BINARY_MAP_HEIGHT && X >= 0 && Y >= 0)
	{
		return BinaryCollisionArray[Y][X];
	}
	return 0;
}

/******************************************************************************/
/*!

*/
/******************************************************************************/
int CheckInstanceBinaryMapCollision(float PosX, float PosY, float scaleX, float scaleY)
{
	//At the end of this function, "Flag" will be used to determine which sides
	//of the object instance are colliding. 2 hot spots will be placed on each side.

	int Flag = 0;
	float x1, x2;
	float y1, y2;

	//Hotspot 1 for left side
	x1 = PosX - scaleX / 2;
	y1 = PosY + scaleY / 4;
	//Hotspot 2 for left side
	x2 = PosX - scaleX / 2;
	y2 = PosY - scaleY / 4;
	if (GetCellValue((int)x1, (int)y1) == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_COLLISION ||
		GetCellValue((int)x2, (int)y2) == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_COLLISION)
	{
		Flag |= COLLISION_LEFT;
	}

	//Hotspot 1 for right side
	x1 = PosX + scaleX / 2;
	y1 = PosY + scaleY / 4;
	//Hotspot 2 for right side
	x2 = PosX + scaleX / 2;
	y2 = PosY - scaleY / 4;
	if (GetCellValue((int)x1, (int)y1) == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_COLLISION ||
		GetCellValue((int)x2, (int)y2) == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_COLLISION)
	{
		Flag |= COLLISION_RIGHT;
	}

	//Hotspot 1 for top side
	x1 = PosX + scaleX / 4;
	y1 = PosY + scaleY / 2;
	//Hotspot 2 for top side
	x2 = PosX - scaleX / 4;
	y2 = PosY + scaleY / 2;
	if (GetCellValue((int)x1, (int)y1) == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_COLLISION ||
		GetCellValue((int)x2, (int)y2) == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_COLLISION)
	{
		Flag |= COLLISION_TOP;
	}

	//Hotspot 1 for bottom side
	x1 = PosX + scaleX / 4;
	y1 = PosY - scaleY / 2;
	//Hotspot 2 for bottom side
	x2 = PosX - scaleX / 4;
	y2 = PosY - scaleY / 2;
	if (GetCellValue((int)x1, (int)y1) == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_COLLISION ||
		GetCellValue((int)x2, (int)y2) == (unsigned int)TYPE_OBJECT::TYPE_OBJECT_COLLISION)
	{
		Flag |= COLLISION_BOTTOM;
	}

	return Flag;
}

/******************************************************************************/
/*!

*/
/******************************************************************************/
void SnapToCell(float *Coordinate)
{
	*Coordinate = (int)*Coordinate + 0.5f;
}

/******************************************************************************/
/*!

*/
/******************************************************************************/
int ImportMapDataFromFile(char *FileName)
{
	FILE* fp;
	errno_t err;
	err = fopen_s(&fp, FileName, "rt");
	int count = 0, number = 0, digit;
	const int buffer_Size = 100; // Size of buffer
	bool allocate_memory = true;
	char buffer[buffer_Size];
	char char_val; // Value of character in string
	if (fp)
	{
		while (!feof(fp) && count < 3)
		{
			// To get the width of the map according to the .txt file
			if (count == 0)
			{
				fgets(buffer, buffer_Size, fp);
				for (int i = 0; i < strlen(buffer); ++i)
				{
					char_val = buffer[i];
					if (char_val >= '0' && char_val <= '9')
					{
						// To convert the char value to int value
						digit = char_val - '0';
						// To save the total value of width
						number = number * 10 + digit;
					}
				}
				BINARY_MAP_WIDTH = number;
			}
			// To get the height of the map according to the txt file
			if (count == 1)
			{
				fgets(buffer, buffer_Size, fp);
				number = 0;
				// To convert the char value to int value
				for (int j = 0; j < strlen(buffer); ++j)
				{
					// Save buffer value into is_char variable
					char_val = buffer[j];
					if (char_val >= '0' && char_val <= '9')
					{
						// To convert the char value to int value
						digit = char_val - '0';
						// To save the total value of width
						number = number * 10 + digit;
					}
				}
				BINARY_MAP_HEIGHT = number;
			}
			// To allocate memory and initialize map states
			if (count == 2)
			{
				// Allocate memory needed for map and array
				if (allocate_memory)
				{
					MapData = new int* [BINARY_MAP_HEIGHT];
					BinaryCollisionArray = new int* [BINARY_MAP_HEIGHT];
					for (int i = 0; i < BINARY_MAP_HEIGHT; ++i)
					{
						MapData[i] = new int[BINARY_MAP_WIDTH];
						BinaryCollisionArray[i] = new int[BINARY_MAP_WIDTH];
					}
					allocate_memory = false;
				}
				// Initialize the map and binary collision array
				for (int i = 0; i < BINARY_MAP_HEIGHT; i++)
				{
					fgets(buffer, buffer_Size, fp);
					for (int j = 0, k = 0; j < BINARY_MAP_WIDTH; j++, k += 2)
					{
						digit = buffer[k] - '0';
						MapData[i][j] = digit;
						if (digit > 1)
						{
							// Set digit to 0 if no collision
							digit = 0;
							BinaryCollisionArray[i][j] = digit;
						}
						BinaryCollisionArray[i][j] = digit;
					}
				}
			}
			count++;
		}
		fclose(fp);
		return 1;
	}
	return 0;
}

/******************************************************************************/
/*!

*/
/******************************************************************************/
void FreeMapData(void)
{
	for (int i = 0; i < BINARY_MAP_HEIGHT;++i)
	{
		free(*(MapData+i));
		free(*(BinaryCollisionArray+i));
	}
	free(MapData);
	free(BinaryCollisionArray);
}

/******************************************************************************/
/*!

*/
/******************************************************************************/
void EnemyStateMachine(GameObjInst* pInst)
{
	/***********
	This state machine has 2 states: STATE_GOING_LEFT and STATE_GOING_RIGHT
	Each state has 3 inner states: INNER_STATE_ON_ENTER, INNER_STATE_ON_UPDATE, INNER_STATE_ON_EXIT
	Use "switch" statements to determine which state and inner state the enemy is currently in.


	STATE_GOING_LEFT
		INNER_STATE_ON_ENTER
			Set velocity X to -MOVE_VELOCITY_ENEMY
			Set inner state to "on update"

		INNER_STATE_ON_UPDATE
			If collision on left side OR bottom left cell is non collidable
				Initialize the counter to ENEMY_IDLE_TIME
				Set inner state to on exit
				Set velocity X to 0


		INNER_STATE_ON_EXIT
			Decrement counter by frame time
			if counter is less than 0 (sprite's idle time is over)
				Set state to "going right"
				Set inner state to "on enter"

	STATE_GOING_RIGHT is basically the same, with few modifications.

	***********/
	switch (pInst->state)
	{
	case STATE::STATE_GOING_LEFT:
		{
			switch (pInst->innerState)
			{
			case INNER_STATE::INNER_STATE_ON_ENTER:
			{
				pInst->velCurr.x = -MOVE_VELOCITY_ENEMY;
				pInst->innerState = INNER_STATE::INNER_STATE_ON_UPDATE;
			}
			break;
			case INNER_STATE::INNER_STATE_ON_UPDATE:
			{
				float a = (pInst->posCurr.x) - 1.0f;
				float b = (pInst->posCurr.y) - 1.0f;
				if ((pInst->gridCollisionFlag & COLLISION_LEFT) || 
					!(CheckInstanceBinaryMapCollision(a, b, pInst->scale,pInst->scale) & COLLISION_BOTTOM))
				{
					pInst->counter = ENEMY_IDLE_TIME;
					pInst->innerState = INNER_STATE::INNER_STATE_ON_EXIT;
					pInst->velCurr.x = 0;
					SnapToCell(&pInst->posCurr.x);
				}
			}
			break;
			case INNER_STATE::INNER_STATE_ON_EXIT:
			{
				pInst->counter -= g_dt;
				if (pInst->counter <= 0)
				{
					pInst->state = STATE::STATE_GOING_RIGHT;
					pInst->innerState = INNER_STATE::INNER_STATE_ON_ENTER;
				}
			}
			break;
			}
		}
		break;
	case STATE::STATE_GOING_RIGHT:
		{
			switch (pInst->innerState)
			{
			case INNER_STATE::INNER_STATE_ON_ENTER:
			{
				pInst->velCurr.x = MOVE_VELOCITY_ENEMY;
				pInst->innerState = INNER_STATE::INNER_STATE_ON_UPDATE;
			}
			break;
			case INNER_STATE::INNER_STATE_ON_UPDATE:
			{
				float a = (pInst->posCurr.x) + 1.0f;
				float b = (pInst->posCurr.y) - 1.0f;
				if ((pInst->gridCollisionFlag & COLLISION_RIGHT) ||
					!(CheckInstanceBinaryMapCollision(a, b, pInst->scale, pInst->scale) & COLLISION_BOTTOM))
				{
					pInst->counter = ENEMY_IDLE_TIME;
					pInst->innerState = INNER_STATE::INNER_STATE_ON_EXIT;
					pInst->velCurr.x = 0;
					SnapToCell(&pInst->posCurr.x);
				}
			}
			break;
			case INNER_STATE::INNER_STATE_ON_EXIT:
			{
				pInst->counter -= g_dt;
				if (pInst->counter <= 0)
				{
					pInst->state = STATE::STATE_GOING_LEFT;
					pInst->innerState = INNER_STATE::INNER_STATE_ON_ENTER;
				}
			}
			break;
			}
		}
		break;
	}
}

void cam_Update()
{

	AEGfxSetCamPosition(pHero->posCurr.x * AEGetWindowWidth() / 20 - AEGetWindowWidth(),
						pHero->posCurr.y * AEGetWindowHeight() / 20 - AEGetWindowHeight());
}

void cam_Reset()
{
	AEGfxSetCamPosition(0, 0);
}
