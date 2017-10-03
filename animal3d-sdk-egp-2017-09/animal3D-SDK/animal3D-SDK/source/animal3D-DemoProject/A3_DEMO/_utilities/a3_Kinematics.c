/*
	Copyright 2011-2017 Daniel S. Buckstein

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

/*
	animal3D SDK: Minimal 3D Animation Framework
	By Daniel S. Buckstein
	
	a3_Kinematics.c
	Implementation of kinematics solvers.
*/

#include "a3_Kinematics.h"


//-----------------------------------------------------------------------------

// FK solver
extern inline int a3kinematicsSolveForward(const a3_HierarchyState *hierarchyState)
{
	return a3kinematicsSolveForwardPartial(hierarchyState, 0);
}

// partial FK solver
extern inline int a3kinematicsSolveForwardPartial(const a3_HierarchyState *hierarchyState, const unsigned int firstIndex)
{
	if (hierarchyState && hierarchyState->hierarchy && firstIndex < hierarchyState->hierarchy->numNodes)
	{
		// ****TO-DO: implement forward kinematics algorithm
		//	- for all nodes starting at first index
		//		- if node is root (no parent)
		//			- copy local matrix to object matrix
		//		- else
		//			- object matrix = parent object matrix * local matrix

		unsigned int i;
		int parentIndex;
		for (i = firstIndex; i < hierarchyState->hierarchy->numNodes; ++i)
		{
			parentIndex = hierarchyState->hierarchy->nodes[i].parentIndex;

			if(parentIndex < 0)
				hierarchyState->objectSpaceTransforms[i] = hierarchyState->localSpaceTransforms[i];
			else
			{
				p3real4x4Product(hierarchyState->objectSpaceTransforms[i].m, 
					hierarchyState->objectSpaceTransforms[parentIndex].m, 
					hierarchyState->localSpaceTransforms[i].m);
			}
		}

		// done
		return 1;
	}
	return -1;
}


//-----------------------------------------------------------------------------
