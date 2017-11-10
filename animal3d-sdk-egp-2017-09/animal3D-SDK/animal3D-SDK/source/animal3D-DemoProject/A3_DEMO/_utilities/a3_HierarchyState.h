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
	typedef struct a3_HierarchyNodePose		a3_HierarchyNodePose;
	typedef struct a3_HierarchyPose			a3_HierarchyPose;
	typedef struct a3_HierarchyTransform	a3_HierarchyTransform;
	typedef struct a3_HierarchyPoseGroup	a3_HierarchyPoseGroup;
	typedef struct a3_HierarchyState		a3_HierarchyState;
	typedef enum a3_HierarchyPoseFlag		a3_HierarchyPoseFlag;
#endif	// __cplusplus

	
//-----------------------------------------------------------------------------

	// single pose for a single node
	struct a3_HierarchyNodePose
	{
		// 4D vector to hold up to 4 values (quat) or 3 Euler angles
		// default value is identity quaternion (0, 0, 0, 1), which 
		//	also works for default Euler angles (0, 0, 0)
		p3vec4 orientation;

		// 3D vector for translation
		// default value is zero vector (0, 0, 0)
		// can use w to store bone length (distance from parent)
		p3vec4 translation;

		// 3D vector for scale (uniform means all values are the same)
		// scale is not typically used, but it's here in case it's useful
		// default is unit scale (1, 1, 1)
		// can use w to store uniform flag
		p3vec4 scale;
	};


	// single pose for a collection of nodes
	// makes algorithms easier to keep this as a separate data type
	struct a3_HierarchyPose
	{
		a3_HierarchyNodePose *nodePose;
	};


	// collection of matrices for transformation set
	struct a3_HierarchyTransform
	{
		p3mat4 *transform;
	};


	// pose group
	struct a3_HierarchyPoseGroup
	{
		// pointer to hierarchy
		const a3_Hierarchy *hierarchy;

		// contiguous array of all node poses (for efficiency)
		a3_HierarchyNodePose *nodePoseContiguous;

		// list of poses for full hierarchy
		a3_HierarchyPose *pose;

		// number of hierarchy poses in set
		unsigned int poseCount;
	};


	// hierarchy state structure, with a pointer to the source pose group 
	//	and transformations for kinematics
	struct a3_HierarchyState
	{
		// pointer to pose set that the poses come from
		const a3_HierarchyPoseGroup *poseGroup;

		// local poses
		a3_HierarchyPose localPose[1];

		// local transformations (relative to parent's space)
		a3_HierarchyTransform localSpace[1];

		// object transformations (relative to root's parent's space)
		a3_HierarchyTransform objectSpace[1];
	};
	

//-----------------------------------------------------------------------------

	// flags to describe transformation components in use
	enum a3_HierarchyPoseFlag
	{
		a3poseFlag_identity,		// not using transform components
		a3poseFlag_rotate,			// using Euler angles for rotation
		a3poseFlag_rotate_q = 0x3,	// using quaternion for rotation
		a3poseFlag_scale,			// using scale
		a3poseFlag_translate = 0x8,	// using translation
	};
	

//-----------------------------------------------------------------------------

	// initialize pose set given an initialized hierarchy and key pose count
	inline int a3hierarchyPoseGroupCreate(a3_HierarchyPoseGroup *poseGroup_out, const a3_Hierarchy *hierarchy, const unsigned int poseCount);

	// release pose set
	inline int a3hierarchyPoseGroupRelease(a3_HierarchyPoseGroup *poseGroup);

	// get offset to hierarchy pose in contiguous set
	inline int a3hierarchyPoseGroupGetPoseOffsetIndex(const a3_HierarchyPoseGroup *poseGroup, const unsigned int poseIndex);

	// get offset to single node pose in contiguous set
	inline int a3hierarchyPoseGroupGetNodePoseOffsetIndex(const a3_HierarchyPoseGroup *poseGroup, const unsigned int poseIndex, const unsigned int nodeIndex);


//-----------------------------------------------------------------------------

	// initialize hierarchy state given an initialized hierarchy
	inline int a3hierarchyStateCreate(a3_HierarchyState *state_out, const a3_HierarchyPoseGroup *poseGroup);

	// release hierarchy state
	inline int a3hierarchyStateRelease(a3_HierarchyState *state);


//-----------------------------------------------------------------------------

	// reset single node pose
	inline int a3hierarchyNodePoseReset(a3_HierarchyNodePose *nodePose_inout);

	// copy single node pose
	inline int a3hierarchyNodePoseCopy(a3_HierarchyNodePose *nodePose_out, const a3_HierarchyNodePose *copyNodePose);

	// invert single node pose
	inline int a3hierarchyNodePoseInvert(a3_HierarchyNodePose *nodePose_out, const a3_HierarchyNodePose *invertNodePose);

	// LERP single node pose
	inline int a3hierarchyNodePoseLERP(a3_HierarchyNodePose *nodePose_out, const a3_HierarchyNodePose *nodePose0, const a3_HierarchyNodePose *nodePose1, const float param, const a3_HierarchyPoseFlag flag);

	// add/concat single node pose
	inline int a3hierarchyNodePoseConcat(a3_HierarchyNodePose *nodePose_out, const a3_HierarchyNodePose *nodePose0, const a3_HierarchyNodePose *nodePose1, const a3_HierarchyPoseFlag flag);

	// scale single node pose
	inline int a3hierarchyNodePoseScale(a3_HierarchyNodePose *nodePose_out, const a3_HierarchyNodePose *nodePoseScale, const float param, const a3_HierarchyPoseFlag flag);

	// blend single node pose
	inline int a3hierarchyNodePoseBlend(a3_HierarchyNodePose *nodePose_out, const a3_HierarchyNodePose *nodePose0, const a3_HierarchyNodePose *nodePose1, const float weight0, const float weight1, const a3_HierarchyPoseFlag flag);

	// triangular LERP single node pose
	inline int a3hierarchyNodePoseTriangularLERP(a3_HierarchyNodePose *nodePose_out, const a3_HierarchyNodePose *nodePose0, const a3_HierarchyNodePose *nodePose1, const a3_HierarchyNodePose *nodePose2, const float param0, const float param1, const a3_HierarchyPoseFlag flag);

	// convert single node pose to matrix
	inline int a3hierarchyNodePoseConvert(p3mat4 *mat_out, const a3_HierarchyNodePose *nodePose, const a3_HierarchyPoseFlag flag);


	// reset full hierarchy pose
	inline int a3hierarchyPoseReset(const a3_HierarchyPose *pose_inout, const unsigned int nodeCount);

	// copy full hierarchy pose
	inline int a3hierarchyPoseCopy(const a3_HierarchyPose *pose_out, const a3_HierarchyPose *copyPose, const unsigned int nodeCount);

	// invert full hierarchy pose
	inline int a3hierarchyPoseInvert(const a3_HierarchyPose *pose_out, const a3_HierarchyPose *invertPose, const unsigned int nodeCount);

	// LERP full hierarchy pose
	inline int a3hierarchyPoseLERP(const a3_HierarchyPose *pose_out, const a3_HierarchyPose *pose0, const a3_HierarchyPose *pose1, const float param, const unsigned int nodeCount, const a3_HierarchyPoseFlag flag);

	// add/concat full hierarchy pose
	inline int a3hierarchyPoseConcat(const a3_HierarchyPose *pose_out, const a3_HierarchyPose *pose0, const a3_HierarchyPose *pose1, const unsigned int nodeCount, const a3_HierarchyPoseFlag flag);

	// scale full hierarchy pose
	inline int a3hierarchyPoseScale(const a3_HierarchyPose *pose_out, const a3_HierarchyPose *poseScale, const float param, const unsigned int nodeCount, const a3_HierarchyPoseFlag flag);

	// blend full hierarchy pose
	inline int a3hierarchyPoseBlend(const a3_HierarchyPose *pose_out, const a3_HierarchyPose *pose0, const a3_HierarchyPose *pose1, const float weight0, const float weight1, const unsigned int nodeCount, const a3_HierarchyPoseFlag flag);

	// triangular LERP full hierarchy pose
	inline int a3hierarchyPoseTriangularLERP(const a3_HierarchyPose *pose_out, const a3_HierarchyPose *pose0, const a3_HierarchyPose *pose1, const a3_HierarchyPose *pose2, const float param0, const float param1, const unsigned int nodeCount, const a3_HierarchyPoseFlag flag);

	// convert full hierarchy pose to hierarchy transforms
	inline int a3hierarchyPoseConvert(const a3_HierarchyTransform *transform_out, const a3_HierarchyPose *pose, const unsigned int nodeCount, const a3_HierarchyPoseFlag flag);


//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif	// __cplusplus


#endif	// !__ANIMAL3D_HIERARCHYSTATE_H