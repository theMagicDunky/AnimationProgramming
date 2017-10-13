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
	
	a3_HierarchyState.h
	Hierarchy transformation state.
*/

#ifndef __ANIMAL3D_HIERARCHYSTATE_H
#define __ANIMAL3D_HIERARCHYSTATE_H


// A3 hierarchy
#include "animal3D/a3animation/a3_Hierarchy.h"

// math library
#include "P3DM/P3DM.h"


//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#else	// !__cplusplus
	typedef struct a3_HierarchyNodePose a3_HierarchyNodePose;
	typedef struct a3_HierarchyPoseSet a3_HierarchyPoseSet;
	typedef struct a3_HierarchyState	a3_HierarchyState;
#endif	// __cplusplus

	
//-----------------------------------------------------------------------------

	// pose for a single node (key pose)
	struct a3_HierarchyNodePose
	{
		// rotation
		// can be quat or 3 euler angles
		p3vec4 orientation;

		// translation
		p3vec3 translation;

		// scale
		p3vec3 scale;
	};

	struct a3_HierarchyPoseSet
	{
		const a3_Hierarchy* hierarchy;

		// pose set: all poses for all nodes
		// 2d array of unknown size
		// list[nodeIndex][poseIndex]
		// accessing format ^^
		a3_HierarchyNodePose** poseList;

		// contiguous data
		a3_HierarchyNodePose* poseListContiguous;

		// number of keys
		unsigned int keyPoseCount;
	};


	// hierarchy state structure, with a pointer to a hierarchy (decoupled) 
	//	and transformations for kinematics
	struct a3_HierarchyState
	{
		// pointer to hierarchy tree
		const a3_Hierarchy *hierarchy;

		// local transformations (relative to parent's space)
		p3mat4 *localSpaceTransforms;

		// object transformations (relative to root's parent's space)
		p3mat4 *objectSpaceTransforms;

		// local pose list
		// used to calc local transform
		a3_HierarchyNodePose* localPoseList;
	};




//-----------------------------------------------------------------------------

	// initialize hierarchy state given an initialized hierarchy
	inline int a3hierarchyStateCreate(a3_HierarchyState *state_out, const a3_Hierarchy *hierarchy);

	// release hierarchy state
	inline int a3hierarchyStateRelease(a3_HierarchyState *state);


//-----------------------------------------------------------------------------

	// allocate pose set (resource data)
	inline int a3hierarchyPoseSetCreate(a3_HierarchyPoseSet* poseSet_out, const a3_Hierarchy* hierarchy, const unsigned int keyPoseCount);

	// release
	inline int a3hierarchyPostSetRelease(a3_HierarchyPoseSet* poseSet);

	// set default pose
	inline int a3hierarchyNodePoseReset(a3_HierarchyNodePose* pose);

	// copy key pose from set to state (resource to state)
	inline int a3hierarchyStateCopyKeyPose(const a3_HierarchyState* state, const a3_HierarchyPoseSet* poseSet, const unsigned int keyPoseIndex);
	
	// convert current state post to transforms
	inline int a3hierarchyStateConvertPose(const a3_HierarchyState* state, const unsigned int useQuat);

//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif	// __cplusplus


#endif	// !__ANIMAL3D_HIERARCHYSTATE_H