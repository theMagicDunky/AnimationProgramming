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
typedef struct a3_HierarchyNodePose	a3_HierarchyNodePose;
typedef struct a3_HierarchyPoseSet	a3_HierarchyPoseSet;
typedef struct a3_HierarchyState	a3_HierarchyState;
#endif	// __cplusplus


//-----------------------------------------------------------------------------

// pose for a single node (key pose)
// the actual "sample" data for a single node at a time
struct a3_HierarchyNodePose
{
	// all the stuff you need for your mat4
	// rotation
	// quaternion or euler angles
	p3vec4 orientation;

	// translation
	p3vec3 translation;

	// scale
	// uniform or non uniform
	p3vec3 scale;
};

// container for all the poses for all nodes in a hierarchy
struct a3_HierarchyPoseSet
{
	const a3_Hierarchy *hierarchy;

	// all poses for all nodes
	// ***********BE CONSOISTENT WITH ACCESSING ELEMENTS
	// eg list[nodeindex][poseindex]
	a3_HierarchyNodePose **poseList;

	a3_HierarchyNodePose *poseListContiguous;

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


	// have a list of your node poses
	// one per joint

	// joints define the local state
	a3_HierarchyNodePose *localPoseList;
};


//-----------------------------------------------------------------------------

// initialize hierarchy state given an initialized hierarchy
inline int a3hierarchyStateCreate(a3_HierarchyState *state_out, const a3_Hierarchy *hierarchy);

// release hierarchy state
inline int a3hierarchyStateRelease(a3_HierarchyState *state);


//-----------------------------------------------------------------------------


// NEW DLC DROPPING
// [salt] allocate pose set (resource data)
inline int a3hierarchyPoseSetCreate(a3_HierarchyPoseSet * poseSet_out, const a3_Hierarchy *hierarchy, const unsigned int keyPoseCount);

// [salsa] release
inline int a3hierarchyPoseSetRelease(a3_HierarchyPoseSet * poseSet);

// set default pose
inline int a3hierarchyNodePoseReset(a3_HierarchyNodePose *pose);

// set pose
inline int a3hierarchyNodePoseSet(a3_HierarchyNodePose *pose, const p3vec4 orientation, const p3vec3 translation, const p3vec3 scale);

// store a single node pose in a set, -1 is base pose
inline int a3hierarchyPoseSetInitNodePose(const a3_HierarchyPoseSet *poseSet, const a3_HierarchyNodePose *pose, const unsigned int nodeIndex, const unsigned int keyPoseIndex);

// copy single node pose from the set, if negative, base pose
inline int a3hierarchyNodePoseCopyNodePose(a3_HierarchyNodePose *pose_out, const a3_HierarchyPoseSet *poseSet, const unsigned int nodeIndex, const unsigned int keyPoseIndex);


// copy key pose from set to state
inline int a3hierarchyStateCopyKeyPose(const a3_HierarchyState *state, const a3_HierarchyPoseSet *poseSet, const unsigned int keyPoseIndex);

// calc in between pose between key poses
inline int a3hierarchyStateCalcInBetweenPoses(const a3_HierarchyState *state, const a3_HierarchyPoseSet *poseSet, unsigned int keyPoseIndex0, unsigned int keyPoseIndex1, const float t, const int usingQuaternions);

// convert the current state pose to transforms
inline int a3hierarchyStateConvertPose(const a3_HierarchyState *state, const a3_HierarchyPoseSet * poseSet, const unsigned int usingQuaternions);

//-----------------------------------------------------------------------------




#ifdef __cplusplus
}
#endif	// __cplusplus


#endif	// !__ANIMAL3D_HIERARCHYSTATE_H