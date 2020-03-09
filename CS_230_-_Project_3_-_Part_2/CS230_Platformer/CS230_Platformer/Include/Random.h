/* Start Header **************************************************************/
/*!
\file		Random.h
\author		Chye Min Liang, Clarence, chye.m, 390004119
\par		chye.m\@digipen.edu
\date		Feb 09, 2020
\brief		Declarations for the Random functions in Random.cpp

Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header ****************************************************************/

#ifndef RANDOM_H
#define RANDOM_H

/*****************************************************************************/
/*!
\fn						void Random_Init()

\brief					Initializes the random system to get the random number.
*/
/*****************************************************************************/
void Random_Init();


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
int Random_Range(int min_n, int max_n);


#endif // RANDOM_H
