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

#include "a3_Quaternion.h"


//-----------------------------------------------------------------------------

// FK solver
extern inline int a3kinematicsSolveForward(const a3_HierarchyState *hierarchyState)
{
	return a3kinematicsSolveForwardPartial(hierarchyState, 0, hierarchyState->poseGroup->hierarchy->numNodes);
}

// partial FK solver
extern inline int a3kinematicsSolveForwardPartial(const a3_HierarchyState *hierarchyState, const unsigned int firstIndex, const unsigned int nodeCount)
{
	if (hierarchyState && hierarchyState->poseGroup && 
		firstIndex < hierarchyState->poseGroup->hierarchy->numNodes && nodeCount)
	{
		// ****TO-DO: implement forward kinematics algorithm
		//	- for all nodes starting at first index
		//		- if node is not root (has parent node)
		//			- object matrix = parent object matrix * local matrix
		//		- else
		//			- copy local matrix to object matrix

		int parentIndex;
		unsigned int i, end = firstIndex + nodeCount;
		end = minimum(end, hierarchyState->poseGroup->hierarchy->numNodes);

		for (i = firstIndex; i < end; ++i)
		{
			parentIndex = hierarchyState->poseGroup->hierarchy->nodes[i].parentIndex;
			if (parentIndex >= 0)
				p3real4x4Product(hierarchyState->objectSpace->transform[i].m,
					hierarchyState->objectSpace->transform[parentIndex].m,
					hierarchyState->localSpace->transform[i].m);
			else
				hierarchyState->objectSpace->transform[i] = hierarchyState->localSpace->transform[i];
		}

		// done, return number of nodes updated
		return (end - firstIndex);
	}
	return -1;
}


//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
