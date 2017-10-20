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
		//	('local' points to contiguous array of all matrices and floats)
		state_out->localSpaceTransforms = (p3mat4 *)malloc(count2 * sizeof(p3mat4) + count * sizeof(float) + count * sizeof(a3_HierarchyNodePose));
		state_out->objectSpaceTransforms = (state_out->localSpaceTransforms + count);
		state_out->localPoseList = (a3_HierarchyNodePose*)(state_out->objectSpaceTransforms + count);

		// set all matrices to identity
		for (i = 0; i < count2; ++i)
			p3real4x4SetIdentity(state_out->localSpaceTransforms[i].m);

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
extern inline int a3hierarchyPoseSetCreate(a3_HierarchyPoseSet * poseSet_out, const a3_Hierarchy *hierarchy, const unsigned int keyPoseCount)
{
	// validtate
	if (poseSet_out && !poseSet_out->hierarchy && hierarchy && hierarchy->nodes && keyPoseCount > 0)
	{
		const unsigned int nodeCount = hierarchy->numNodes;
		const unsigned int totalPoses = nodeCount * (keyPoseCount + 1);

		unsigned int i;
		a3_HierarchyNodePose *posePtr;

		poseSet_out->hierarchy = hierarchy;
		poseSet_out->keyPoseCount = keyPoseCount;

		// SALT ALLOCATION
		poseSet_out->poseListContiguous = (a3_HierarchyNodePose*)malloc(totalPoses * sizeof(a3_HierarchyNodePose) + nodeCount * sizeof(a3_HierarchyNodePose*));
		poseSet_out->poseList = (a3_HierarchyNodePose**)(poseSet_out->poseListContiguous + totalPoses);

		// resety the poses
		for (i = 0; i < totalPoses; ++i)
			a3hierarchyNodePoseReset(poseSet_out->poseListContiguous + i);

		// set key node pointers
		for (i = 0, posePtr = poseSet_out->poseListContiguous; i < nodeCount; ++i, posePtr += keyPoseCount + 1)
		{
			poseSet_out->poseList[i] = posePtr;
		}


		return keyPoseCount;
	}
	return -1;
}

extern inline int a3hierarchyPoseSetRelease(a3_HierarchyPoseSet * poseSet)
{
	if (poseSet && poseSet->hierarchy)
	{
		free(poseSet->poseListContiguous);
		poseSet->hierarchy = 0;
		poseSet->poseList = 0;
		poseSet->poseListContiguous = 0;
		poseSet->keyPoseCount = 0;

		return 1;
	}
	return -1;
}


// reset to 'identity'
extern inline int a3hierarchyNodePoseReset(a3_HierarchyNodePose *pose)
{
	if (pose)
	{
		pose->orientation = p3wVec4;
		pose->translation = p3zeroVec3;
		pose->scale = p3oneVec3;
	}

	return -1;
}








// consider making scale and translation vec4s as additional metadata storage
extern inline int a3hierarchyNodePoseSet(a3_HierarchyNodePose * pose, const p3vec4 orientation, const p3vec3 translation, const p3vec3 scale)
{
	if (pose)
	{
		pose->orientation = orientation;
		pose->translation = translation;
		pose->scale = scale;

		// anything else

		return 1;
	}

	return -1;
}

extern inline int a3hierarchyPoseSetInitNodePose(const a3_HierarchyPoseSet * poseSet, const a3_HierarchyNodePose * pose, const unsigned int nodeIndex, const unsigned int keyPoseIndex)
{
	if (poseSet && pose && poseSet->hierarchy && nodeIndex < poseSet->hierarchy->numNodes && (int)keyPoseIndex <= (int)poseSet->keyPoseCount)
	{
		// select pose
		const unsigned int poseIndex = (int)keyPoseIndex >= 0 ? keyPoseIndex : poseSet->keyPoseCount;

		poseSet->poseList[nodeIndex][poseIndex] = *pose;
		return poseIndex;
	}

	return -1;
}

extern inline int a3hierarchyNodePoseCopyNodePose(a3_HierarchyNodePose * pose_out, const a3_HierarchyPoseSet * poseSet, const unsigned int nodeIndex, const unsigned int keyPoseIndex)
{
	if (poseSet && pose_out && poseSet->hierarchy && nodeIndex < poseSet->hierarchy->numNodes && (int)keyPoseIndex <= (int)poseSet->keyPoseCount)
	{
		// select pose
		const unsigned int poseIndex = (int)keyPoseIndex >= 0 ? keyPoseIndex : poseSet->keyPoseCount;

		*pose_out = poseSet->poseList[nodeIndex][poseIndex];
		return poseIndex;
	}



	return -1;
}









// copy a keypose to the current state
extern inline int a3hierarchyStateCopyKeyPose(const a3_HierarchyState *state, const a3_HierarchyPoseSet *poseSet, const unsigned int keyPoseIndex)
{
	if (state && poseSet && state->hierarchy && poseSet->hierarchy && (int)keyPoseIndex <= poseSet->keyPoseCount)
	{
		const unsigned int poseIndex = (int)keyPoseIndex >= 0 ? keyPoseIndex : poseSet->keyPoseCount;
		unsigned int i;
		// if not base pose, copy deltas
		if (poseIndex != poseSet->keyPoseCount)
		{

			for (i = 0; i < state->hierarchy->numNodes; ++i)
			{
				state->localPoseList[i] = poseSet->poseList[i][keyPoseIndex];
			}
		}
		else // reset local pose index in our state
		{
			for (i = 0; i < state->hierarchy->numNodes; ++i)
				a3hierarchyNodePoseReset(state->localPoseList + i);
		}




		return keyPoseIndex;
	}

	return -1;
}

extern inline int a3hierarchyStateCalcInBetweenPoses(const a3_HierarchyState * state, const a3_HierarchyPoseSet * poseSet, unsigned int keyPoseIndex0, unsigned int keyPoseIndex1, const float t, const int usingQuaternions)
{
	if (state && poseSet && state->hierarchy && poseSet->hierarchy && (int)keyPoseIndex0 <= poseSet->keyPoseCount && (int)keyPoseIndex1 <= poseSet->keyPoseCount)
	{
		const unsigned int poseIndex0 = (int)keyPoseIndex0 >= 0 ? keyPoseIndex0 : poseSet->keyPoseCount;
		const unsigned int poseIndex1 = (int)keyPoseIndex1 >= 0 ? keyPoseIndex1 : poseSet->keyPoseCount;
		unsigned int i;

		// interpolate between the poses
		// slerp rotations
		// lerp positions
		const a3_HierarchyNodePose *nodePtr0, *nodePtr1;
		a3_HierarchyNodePose *resultPosePtr;

		// 4 ways to do this shit
		// if either is a base pose, we have a special case
		// neither are base
		// one or other is base x2
		// both are base
		if (poseIndex0 != poseSet->keyPoseCount && poseIndex1 != poseSet->keyPoseCount)
		{
			// interpolate between both
			if (usingQuaternions)
			{
				for (i = 0; i < state->hierarchy->numNodes; ++i)
				{
					resultPosePtr = state->localPoseList + i;
					nodePtr0 = poseSet->poseList[i] + poseIndex0;
					nodePtr1 = poseSet->poseList[i] + poseIndex1;

					a3quatUnitSLERP(resultPosePtr->orientation.v, nodePtr0->orientation.v, nodePtr1->orientation.v, t);
					p3real3Lerp(resultPosePtr->translation.v, nodePtr0->translation.v, nodePtr1->translation.v, t);
					p3real3Lerp(resultPosePtr->scale.v, nodePtr0->scale.v, nodePtr1->scale.v, t);
				}
			}
			else
			{
				for (i = 0; i < state->hierarchy->numNodes; ++i)
				{
					resultPosePtr = state->localPoseList + i;
					nodePtr0 = poseSet->poseList[i] + poseIndex0;
					nodePtr1 = poseSet->poseList[i] + poseIndex1;

					p3real3Lerp(resultPosePtr->orientation.v, nodePtr0->orientation.v, nodePtr1->orientation.v, t);
					p3real3Lerp(resultPosePtr->translation.v, nodePtr0->translation.v, nodePtr1->translation.v, t);
					p3real3Lerp(resultPosePtr->scale.v, nodePtr0->scale.v, nodePtr1->scale.v, t);
				}
			}
		}
		else if (poseIndex0 == poseSet->keyPoseCount)// && poseIndex1 != poseSet->keyPoseCount)
		{
			if (usingQuaternions)
			{
				for (i = 0; i < state->hierarchy->numNodes; ++i)
				{
					resultPosePtr = state->localPoseList + i;
					nodePtr1 = poseSet->poseList[i] + poseIndex1;

					a3quatUnitSLERP(resultPosePtr->orientation.v, p3wVec4.v, nodePtr1->orientation.v, t);
					p3real3Lerp(resultPosePtr->translation.v, p3zeroVec3.v, nodePtr1->translation.v, t);
					p3real3Lerp(resultPosePtr->scale.v, p3oneVec3.v, nodePtr1->scale.v, t);
				}
			}
			else
			{
				for (i = 0; i < state->hierarchy->numNodes; ++i)
				{
					resultPosePtr = state->localPoseList + i;
					nodePtr1 = poseSet->poseList[i] + poseIndex1;

					p3real3Lerp(resultPosePtr->orientation.v, p3wVec4.v, nodePtr1->orientation.v, t);
					p3real3Lerp(resultPosePtr->translation.v, p3zeroVec3.v, nodePtr1->translation.v, t);
					p3real3Lerp(resultPosePtr->scale.v, p3oneVec3.v, nodePtr1->scale.v, t);
				}
			}
		}
		else if (/*poseIndex0 != poseSet->keyPoseCount && */poseIndex1 == poseSet->keyPoseCount)
		{
			if (usingQuaternions)
			{
				for (i = 0; i < state->hierarchy->numNodes; ++i)
				{
					resultPosePtr = state->localPoseList + i;
					nodePtr0 = poseSet->poseList[i] + poseIndex1;

					a3quatUnitSLERP(resultPosePtr->orientation.v, nodePtr1->orientation.v, p3wVec4.v, t);
					p3real3Lerp(resultPosePtr->translation.v, nodePtr1->translation.v, p3zeroVec3.v, t);
					p3real3Lerp(resultPosePtr->scale.v, nodePtr1->scale.v, p3oneVec3.v, t);
				}
			}
			else
			{
				for (i = 0; i < state->hierarchy->numNodes; ++i)
				{
					resultPosePtr = state->localPoseList + i;
					nodePtr1 = poseSet->poseList[i] + poseIndex1;

					p3real3Lerp(resultPosePtr->orientation.v, nodePtr1->orientation.v, p3wVec4.v, t);
					p3real3Lerp(resultPosePtr->translation.v, nodePtr1->translation.v, p3zeroVec3.v, t);
					p3real3Lerp(resultPosePtr->scale.v, nodePtr1->scale.v, p3oneVec3.v, t);
				}
			}
		}
		else// if (poseIndex0 == poseSet->keyPoseCount && poseIndex1 == poseSet->keyPoseCount)
		{
			// just set it
			for (i = 0; i < state->hierarchy->numNodes; ++i)
				a3hierarchyNodePoseReset(state->localPoseList + i);
		}

	}

	return -1;
}

extern inline int a3hierarchyStateConvertPose(const a3_HierarchyState *state, const a3_HierarchyPoseSet * poseSet, const unsigned int usingQuaternions)
{
	if (state && state->hierarchy)
	{
		// given pose, set local transforms, use to set object transforms

		unsigned int i;
		a3_HierarchyNodePose finalPose, basePose;
		const a3_HierarchyNodePose *posePtr;
		p3mat4 *localMatPtr;

		// convert per node
		for (i = 0; i < state->hierarchy->numNodes; ++i)
		{
			basePose = poseSet->poseList[i][poseSet->keyPoseCount];
			posePtr = state->localPoseList + i;
			localMatPtr = state->localSpaceTransforms + i;

			// combine current with base
			// rot : quat concat or euler angle addition
			// trans: vector add
			// scale: scalar mult

			if (usingQuaternions)
			{
				a3quatConcat(finalPose.orientation.v, basePose.orientation.v, posePtr->orientation.v);
			}
			else
			{
				p3real3Sum(finalPose.orientation.v, basePose.orientation.v, posePtr->orientation.v);
			}

			// translate
			p3real3Sum(finalPose.translation.v, basePose.translation.v, posePtr->translation.v);

			// scale
			finalPose.scale.x = basePose.scale.x * posePtr->scale.x;
			finalPose.scale.y = basePose.scale.x * posePtr->scale.y;
			finalPose.scale.z = basePose.scale.x * posePtr->scale.z;



			// CONVERSE BOIS <conversion>
			if (usingQuaternions)
			{
				a3quatConvertToMat4(localMatPtr->m, finalPose.orientation.v, finalPose.translation.v);
			}
			else
			{
				p3real4x4SetRotateZYX(localMatPtr->m, finalPose.orientation.x, finalPose.orientation.y, finalPose.orientation.z);
				localMatPtr->v3.xyz = finalPose.translation;
			}


			// scale
			p3real3MulS(localMatPtr->v0.v, finalPose.scale.x);
			p3real3MulS(localMatPtr->v1.v, finalPose.scale.y);
			p3real3MulS(localMatPtr->v2.v, finalPose.scale.z);
		}

		return 1;
	}

	return -1;
}