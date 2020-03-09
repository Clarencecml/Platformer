/* Start Header **************************************************************/
/*!
\file		Random.cpp
\author		Chye Min Liang, Clarence, chye.m, 390004119
\par		chye.m\@digipen.edu
\date		Feb 09, 2020
\brief		Definitions for the Random.h functions to get random number.

Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header ****************************************************************/

#include "Random.h"
#include <time.h>
#include <stdlib.h>

/*****************************************************************************/
/*!
\fn						void Random_Init()

\brief					Initializes the random system to get the random number.
*/
/*****************************************************************************/
void Random_Init()
{
	srand((unsigned int)time(NULL));
}

/*****************************************************************************/
/*!
\fn						int Random_Range(int min_n, int max_n)

\brief					Get the random number, not including 0,
						from a range of numbers 
\param	min_n			Minimum number to input
\param	max_n			Maximum number to input
\returns				The random number in integer form.
*/
/*****************************************************************************/
int Random_Range(int min_n, int max_n)
{
	int Random = 0;
	do 
	{
		Random = rand() % (max_n - min_n + 1) + min_n;
	} 
	while (Random == 0);

	return Random;
}