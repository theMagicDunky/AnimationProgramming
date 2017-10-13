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
	
	a3_HierarchyState.c
	Implementation of transform hierarchy state.
*/

#include "a3_HierarchyState.h"
#include "a3_Quaternion.h"

#include <stdlib.h>


//-----------------------------------------------------------------------------

// initialize hierarchy state given an initialized hierarchy
extern inline int a3hierarchyStateCreate(a3_HierarchyState *state_out, const a3_Hierarchy *hierarchy)
{
	// validate params and initialization states
	//	(output is not yet initialized, hierarchy is initialized)
	if (state_out && hierarchy && !state_out->hierarchy && hierarchy->nodes)
	{
		const unsigned int count = hierarchy->numNodes;
		const unsigned int count2 = count + count;
		unsigned int i;

		// set hierarchy pointer
		state_out->hierarchy = hierarchy;

		// allocate set of matrices in state
		//	('local' points to contiguous array of all matrices and poses)
		state_out->localSpaceTransforms = (p3mat4 *)malloc(count2 * sizeof(p3mat4) + count * sizeof(a3_HierarchyNodePose));
		state_out->objectSpaceTransforms = (state_out->localSpaceTransforms + count);
		state_out->localPoseList = (a3_HierarchyNodePose*)(state_out->objectSpaceTransforms + count);

		// set all matrices to identity
		for (i = 0; i < count2; ++i)
			p3real4x4SetIdentity(state_out->localSpaceTransforms[i].m);

		// reset all poses
		for (i = 0; i < count; ++i)
			a3hierarchyNodePoseReset(state_out->localPoseList + i);

		// return number of nodes
		return count;
	}
	return -1;
}


// release hierarchy state
extern inline int a3hierarchyStateRelease(a3_HierarchyState *state)
{
	// validate param exists and is initialized
	if (state && state->hierarchy)
	{
		// release matrices
		//	(local points to contiguous array of all matrices)
		free(state->localSpaceTransforms);

		// reset pointers
		state->hierarchy = 0;
		state->localSpaceTransforms = 0;
		state->objectSpaceTransforms = 0;

		// done
		return 1;
	}
	return -1;
}

//-----------------------------------------------------------------------------

// allocate pose set (resource data)
extern inline int a3hierarchyPoseSetCreate(a3_HierarchyPoseSet* poseSet_out, const a3_Hierarchy* hierarchy, const unsigned int keyPoseCount)
{
	if (poseSet_out && !poseSet_out->hierarchy && hierarchy->nodes && keyPoseCount > 0)
	{
		const unsigned int nodeCount = hierarchy->numNodes;
		const unsigned int totalPoses = keyPoseCount * nodeCount;
		unsigned int i;

		a3_HierarchyNodePose* posePtr;
		
		poseSet_out->hierarchy = hierarchy;
		poseSet_out->keyPoseCount = keyPoseCount;

		poseSet_out->poseListContiguous = (a3_HierarchyNodePose*)malloc(totalPoses * sizeof(a3_HierarchyNodePose) + nodeCount * sizeof(a3_HierarchyNodePose*));
		poseSet_out->poseList = (a3_HierarchyNodePose**)(poseSet_out->poseListContiguous + totalPoses);

		for (i = 0; i < totalPoses; ++i)
			a3hierarchyNodePoseReset(poseSet_out->poseListContiguous + i);

		// set node key pointers
		for (i = 0, posePtr = poseSet_out->poseListContiguous; i < nodeCount; ++i, posePtr += keyPoseCount)
		{
			poseSet_out->poseList[i] = posePtr;
		}

		return keyPoseCount;
	}

	return -1;
}

// release
extern inline int a3hierarchyPostSetRelease(a3_HierarchyPoseSet* poseSet)
{
	if (poseSet && poseSet->hierarchy)
	{
		free(poseSet->poseListContiguous);
		poseSet->poseList = 0;
		poseSet->hierarchy = 0;
		poseSet->keyPoseCount = 0;
		poseSet->poseListContiguous = 0;

		return 1;
	}
	return -1;
}

// set default pose
extern inline int a3hierarchyNodePoseReset(a3_HierarchyNodePose* pose)
{
	if (pose)
	{
		pose->orientation = p3wVec4;
		pose->translation = p3zeroVec3;
		pose->scale = p3oneVec3;
		return 1;
	}

	return -1;
}

// copy key pose from set to state (resource to state)
extern inline int a3hierarchyStateCopyKeyPose(const a3_HierarchyState* state, const a3_HierarchyPoseSet* poseSet, const unsigned int keyPoseIndex)
{
	if (state && poseSet && state->hierarchy && poseSet->hierarchy && keyPoseIndex < poseSet->keyPoseCount)
	{
		unsigned int i;

		for (i = 0; i < state->hierarchy->numNodes; ++i)
		{
			state->localPoseList[i] = poseSet->poseList[i][keyPoseIndex];
		}

		return keyPoseIndex;
	}

	return -1;
}

// convert current state post to transforms
extern inline int a3hierarchyStateConvertPose(const a3_HierarchyState* state, const unsigned int useQuat)
{
	if (state && state->hierarchy)
	{
		unsigned int i;
		const a3_HierarchyNodePose* posePtr;
		p3mat4* localMatPtr;

		for (i = 0; i < state->hierarchy->numNodes; ++i)
		{
			posePtr = state->localPoseList + i;
			localMatPtr = state->localSpaceTransforms + i;

			if (useQuat)
			{
				a3quatConvertToMat4(localMatPtr->m, posePtr->orientation.v, posePtr->translation.v);
			}
			else
			{
				p3real4x4SetRotateZYX(localMatPtr->m,
					posePtr->orientation.x, posePtr->orientation.y, posePtr->orientation.z);
				localMatPtr->v3.xyz = posePtr->translation;
			}

			//scale
			p3real3MulS(localMatPtr->v0.v, posePtr->scale.x);
			p3real3MulS(localMatPtr->v1.v, posePtr->scale.y);
			p3real3MulS(localMatPtr->v2.v, posePtr->scale.z);
		}
	}

	return -1;
}