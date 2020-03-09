/* Start Header **************************************************************/
/*!
\file		Collision.cpp
\author		Chye Min Liang, Clarence, chye.m, 390004119
\par		chye.m\@digipen.edu
\date		Feb 09, 2020
\brief		Definitions for the Collision.h files. Handles collision checks.

Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header ****************************************************************/

#include "main.h"

/*****************************************************************************/
/*!
\fn bool CollisionIntersection_RectRect(const AABB & aabb1, const AEVec2 & vel1
									  ,const AABB & aabb2, const AEVec2 & vel2)

\brief				Collision Checker using aabb.
\param aabb1		Rectangle Box for object 1
\param aabb2		Rectangle Box for object 2
\param vel1			Velocity for object 1
\param vel2			Velocity for object 2
\returns			True if there is collision, false if there is no collision
*/
/*****************************************************************************/
bool CollisionIntersection_RectRect(const AABB& aabb1, const AEVec2& vel1,
	const AABB& aabb2, const AEVec2& vel2)
{
	if (aabb1.min.x > aabb2.max.x || aabb1.max.x < aabb2.min.x ||
		aabb1.min.y > aabb2.max.y || aabb1.max.y < aabb2.min.y)
	{
		AEVec2 Vb;
		float tFirst, tLast;
		Vb.x = vel1.x - vel2.x;
		Vb.y = vel1.y - vel2.y;
		tFirst = 0;
		tLast = g_dt;

		if (Vb.x == 0)
		{
			if (aabb2.max.x < aabb1.min.x)
			{
				return false;
			}
			else if (aabb2.min.x > aabb1.max.x)
			{
				return false;
			}
		}

		if (Vb.x < 0)
		{
			if (aabb1.min.x > aabb2.max.x) // Case 1
			{
				return false;
			}
			if (aabb1.max.x < aabb2.min.x) // Case 4.1
			{
				tFirst = max(((aabb1.max.x - aabb2.min.x) / Vb.x), tFirst);
			}
			if (aabb1.min.x < aabb2.max.x) // Case 4.2
			{
				tLast = min(((aabb1.min.x - aabb2.max.x) / Vb.x), tLast);
			}
		}

		if (Vb.x > 0)
		{
			if (aabb1.min.x > aabb2.max.x) // Case 2.1
			{
				tFirst = max(((aabb1.min.x - aabb2.max.x) / Vb.x), tFirst);
			}
			if (aabb1.max.x > aabb2.min.x) // Case 2.2
			{
				tLast = min(((aabb1.max.x - aabb2.min.x) / Vb.x), tLast);
			}
			if (aabb1.max.x < aabb2.min.x) // Case 3
			{
				return false;
			}
		}

		if (tFirst > tLast)
		{
			return false;
		}

		if (Vb.y == 0)
		{
			if (aabb2.max.y < aabb1.min.y)
			{
				return false;
			}
			else if (aabb2.min.y > aabb1.max.y)
			{
				return false;
			}
		}

		if (Vb.y < 0)
		{
			if (aabb1.min.y > aabb2.max.y) // Case 1
			{
				return false;
			}
			if (aabb1.max.y < aabb2.min.y) // Case 4.1
			{
				tFirst = max(((aabb1.max.y - aabb2.min.y) / Vb.y), tFirst);
			}
			if (aabb1.min.y < aabb2.max.y) // Case 4.2
			{
				tLast = min(((aabb1.min.y - aabb2.max.y) / Vb.y), tLast);
			}
		}

		if (Vb.y > 0)
		{
			if (aabb1.min.y > aabb2.max.y) // Case 2.1
			{
				tFirst = max(((aabb1.min.y - aabb2.max.y) / Vb.y), tFirst);
			}
			if (aabb1.max.y > aabb2.min.y) // Case 2.2
			{
				tLast = min(((aabb1.max.y - aabb2.min.y) / Vb.y), tLast);
			}
			if (aabb1.max.y < aabb2.min.y) // Case 3
			{
				return false;
			}
		}

		if (tFirst > tLast)
		{
			return false;
		}
		return true;
	}

	else
	{
		return true;
	}
}