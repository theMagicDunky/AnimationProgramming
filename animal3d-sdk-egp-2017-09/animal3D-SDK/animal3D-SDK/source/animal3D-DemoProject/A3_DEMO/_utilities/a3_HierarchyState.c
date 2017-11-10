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
// quaternion component flag
#define a3poseFlag_quat	0x2


//-----------------------------------------------------------------------------
// internal blending operations

// reset pose
inline void a3hierarchyNodePoseReset_internal(a3_HierarchyNodePose *nodePose)
{
	// set defaults for all channels
	nodePose->orientation = p3wVec4;
	nodePose->translation = p3zeroVec4;
	nodePose->scale = p3oneVec4;
}

inline void a3hierarchyPoseReset_internal(const a3_HierarchyPose *pose, const unsigned int nodeCount)
{
	// iterate through list and reset each one
	a3_HierarchyNodePose *nodePose = pose->nodePose, *const end = nodePose + nodeCount;
	while (nodePose < end)
		a3hierarchyNodePoseReset_internal(nodePose++);
}


// copy pose
inline void a3hierarchyNodePoseCopy_internal(a3_HierarchyNodePose *nodePose_out, const a3_HierarchyNodePose *copyNodePose)
{
	*nodePose_out = *copyNodePose;
}

inline void a3hierarchyPoseCopy_internal(const a3_HierarchyPose *pose_out, const a3_HierarchyPose *copyPose, const unsigned int nodeCount)
{
	a3_HierarchyNodePose *nodePose_out = pose_out->nodePose, *const end = nodePose_out + nodeCount;
	const a3_HierarchyNodePose *copyNodePose = copyPose->nodePose;
	while (nodePose_out < end)
		a3hierarchyNodePoseCopy_internal(nodePose_out++, copyNodePose++);
}


// invert pose
inline void a3hierarchyNodePoseInvert_internal(a3_HierarchyNodePose *nodePose_out, const a3_HierarchyNodePose *invertNodePose)
{
	// rotation: quaternion conjugate covers either quaternions or Euler angles
	a3quatConjugate(nodePose_out->orientation.v, invertNodePose->orientation.v);
	
	// not quite the same idea for translation but functional
	a3quatConjugate(nodePose_out->translation.v, invertNodePose->translation.v);

	// scale, invert components
	nodePose_out->scale.x = recip(invertNodePose->scale.x);
	nodePose_out->scale.y = recip(invertNodePose->scale.y);
	nodePose_out->scale.z = recip(invertNodePose->scale.z);
	nodePose_out->scale.w = recip(invertNodePose->scale.w);
}

inline void a3hierarchyPoseInvert_internal(const a3_HierarchyPose *pose_out, const a3_HierarchyPose *invertPose, const unsigned int nodeCount)
{
	a3_HierarchyNodePose *nodePose_out = pose_out->nodePose, *const end = nodePose_out + nodeCount;
	const a3_HierarchyNodePose *invertNodePose = invertPose->nodePose;
	while (nodePose_out < end)
		a3hierarchyNodePoseInvert_internal(nodePose_out++, invertNodePose++);
}


// ****TO-DO: implement internal node operations

// lerp components
inline void a3hierarchyNodePoseLerp_internal(a3_HierarchyNodePose *nodePose_out,
	const a3_HierarchyNodePose *nodePose0, const a3_HierarchyNodePose *nodePose1, const float param)
{
	p3real4Lerp(nodePose_out->orientation.v, nodePose0->orientation.v, nodePose1->orientation.v, param);

	p3real4Lerp(nodePose_out->translation.v, nodePose0->translation.v, nodePose1->translation.v, param);

	p3real4Lerp(nodePose_out->scale.v, nodePose0->scale.v, nodePose1->scale.v, param);
}

inline void a3hierarchyNodePoseLerp_quat_internal(a3_HierarchyNodePose *nodePose_out,
	const a3_HierarchyNodePose *nodePose0, const a3_HierarchyNodePose *nodePose1, const float param)
{
	a3quatUnitSLERP(nodePose_out->orientation.v, nodePose0->orientation.v, nodePose1->orientation.v, param);

	p3real4Lerp(nodePose_out->translation.v, nodePose0->translation.v, nodePose1->translation.v, param);

	p3real4Lerp(nodePose_out->scale.v, nodePose0->scale.v, nodePose1->scale.v, param);
}

// concatenation (ADD)
inline void a3hierarchyNodePoseConcat_internal(a3_HierarchyNodePose *nodePose_out, 
	const a3_HierarchyNodePose *nodePose0, const a3_HierarchyNodePose *nodePose1)
{
	// rotation add
	p3real4Sum(nodePose_out->orientation.v, nodePose0->orientation.v, nodePose1->orientation.v);

	// translate add
	p3real4Sum(nodePose_out->translation.v, nodePose0->translation.v, nodePose1->translation.v);
	
	// scale multiply
	nodePose_out->scale.x = nodePose0->scale.x * nodePose1->scale.x;
	nodePose_out->scale.y = nodePose0->scale.y * nodePose1->scale.y;
	nodePose_out->scale.z = nodePose0->scale.z * nodePose1->scale.z;
	nodePose_out->scale.w = nodePose0->scale.w * nodePose1->scale.w;
}

inline void a3hierarchyNodePoseConcat_quat_internal(a3_HierarchyNodePose *nodePose_out,
	const a3_HierarchyNodePose *nodePose0, const a3_HierarchyNodePose *nodePose1)
{
	// rotation quat concat
	a3quatConcat(nodePose_out->orientation.v, nodePose0->orientation.v, nodePose1->orientation.v);

	//translate add
	p3real4Sum(nodePose_out->translation.v, nodePose0->translation.v, nodePose1->translation.v);

	//scale multiply
	nodePose_out->scale.x = nodePose0->scale.x * nodePose1->scale.x;
	nodePose_out->scale.y = nodePose0->scale.y * nodePose1->scale.y;
	nodePose_out->scale.z = nodePose0->scale.z * nodePose1->scale.z;
	nodePose_out->scale.w = nodePose0->scale.w * nodePose1->scale.w;
}

// scale: Lerp from identity
inline void a3hierarchyNodePoseScale_internal(a3_HierarchyNodePose *nodePose_out,
	const a3_HierarchyNodePose *nodePose, const float param)
{
	// rotation: scalar multiply
	p3real4ProductS(nodePose_out->orientation.v, nodePose->orientation.v, param);

	// translate: scalar multiply
	p3real4ProductS(nodePose_out->translation.v, nodePose->translation.v, param);

	// scale: lerp from 1 vector
	p3real4Lerp(nodePose_out->scale.v, p3oneVec4.v, nodePose->scale.v, param);
}

inline void a3hierarchyNodePoseScale_quat_internal(a3_HierarchyNodePose *nodePose_out,
	const a3_HierarchyNodePose *nodePose, const float param)
{
	// rotation: scalar multiply
	a3quatUnitSLERP(nodePose_out->orientation.v, p3wVec4.v, nodePose->orientation.v, param);

	// translate: scalar multiply
	p3real4ProductS(nodePose_out->translation.v, nodePose->translation.v, param);

	// scale: lerp from 1 vector
	p3real4Lerp(nodePose_out->scale.v, p3oneVec4.v, nodePose->scale.v, param);
}


// lerp components
inline void a3hierarchyPoseLerp_internal(a3_HierarchyPose *pose_out,
	const a3_HierarchyPose *pose0, const a3_HierarchyPose *pose1, const float param, 
	const unsigned int nodeCount)
{
	a3_HierarchyNodePose *nodePose_out = pose_out->nodePose, *const end = nodePose_out + nodeCount;
	const a3_HierarchyNodePose *nodePose0 = pose0->nodePose, *nodePose1 = pose1->nodePose;
	while (nodePose_out < end)
	{
		a3hierarchyNodePoseLerp_internal(nodePose_out++, nodePose0++, nodePose1++, param);
	}
}

inline void a3hierarchyPoseLerp_quat_internal(a3_HierarchyPose *pose_out,
	const a3_HierarchyPose *pose0, const a3_HierarchyPose *pose1, const float param,
	const unsigned int nodeCount)
{
	a3_HierarchyNodePose *nodePose_out = pose_out->nodePose, *const end = nodePose_out + nodeCount;
	const a3_HierarchyNodePose *nodePose0 = pose0->nodePose, *nodePose1 = pose1->nodePose;
	while (nodePose_out < end)
	{
		a3hierarchyNodePoseLerp_quat_internal(nodePose_out++, nodePose0++, nodePose1++, param);
	}
}

// concatenation (ADD)
inline void a3hierarchyPoseConcat_internal(a3_HierarchyPose *pose_out,
	const a3_HierarchyPose *pose0, const a3_HierarchyPose *pose1,
	const unsigned int nodeCount)
{
	a3_HierarchyNodePose *nodePose_out = pose_out->nodePose, *const end = nodePose_out + nodeCount;
	const a3_HierarchyNodePose *nodePose0 = pose0->nodePose, *nodePose1 = pose1->nodePose;
	while (nodePose_out < end)
	{
		a3hierarchyNodePoseConcat_internal(nodePose_out++, nodePose0++, nodePose1++);
	}
}

inline void a3hierarchyPoseConcat_quat_internal(a3_HierarchyPose *pose_out,
	const a3_HierarchyPose *pose0, const a3_HierarchyPose *pose1,
	const unsigned int nodeCount)
{
	a3_HierarchyNodePose *nodePose_out = pose_out->nodePose, *const end = nodePose_out + nodeCount;
	const a3_HierarchyNodePose *nodePose0 = pose0->nodePose, *nodePose1 = pose1->nodePose;
	while (nodePose_out < end)
	{
		a3hierarchyNodePoseConcat_quat_internal(nodePose_out++, nodePose0++, nodePose1++);
	}
}

// scale: Lerp from identity
inline void a3hierarchyPoseScale_internal(a3_HierarchyPose *pose_out,
	const a3_HierarchyPose *pose, const float param,
	const unsigned int nodeCount)
{
	a3_HierarchyNodePose *nodePose_out = pose_out->nodePose, *const end = nodePose_out + nodeCount;
	const a3_HierarchyNodePose *nodePose0 = pose->nodePose;
	while (nodePose_out < end)
	{
		a3hierarchyNodePoseScale_internal(nodePose_out++, nodePose0++, param);
	}
}

inline void a3hierarchyPoseScale_quat_internal(a3_HierarchyPose *pose_out,
	const a3_HierarchyPose *pose, const float param,
	const unsigned int nodeCount)
{
	a3_HierarchyNodePose *nodePose_out = pose_out->nodePose, *const end = nodePose_out + nodeCount;
	const a3_HierarchyNodePose *nodePose0 = pose->nodePose;
	while (nodePose_out < end)
	{
		a3hierarchyNodePoseScale_quat_internal(nodePose_out++, nodePose0++, param);
	}
}

// blend: weighted average between two poses
inline void a3hierarchyNodePoseBlend_internal(a3_HierarchyNodePose *nodePose_out,
	const a3_HierarchyNodePose *nodePose0, const a3_HierarchyNodePose *nodePose1,
	const float param0, const float param1)
{
	// first  "tree" example
	// - scale poses by weights
	// - add results
	a3_HierarchyNodePose tmpPose0[1], tmpPose1[1];
	a3hierarchyNodePoseScale_internal(tmpPose0, nodePose0, param0);
	a3hierarchyNodePoseScale_internal(tmpPose1, nodePose1, param1);
	a3hierarchyNodePoseConcat_internal(nodePose_out, tmpPose0, tmpPose1);
}

inline void a3hierarchyNodePoseBlend_quat_internal(a3_HierarchyNodePose *nodePose_out,
	const a3_HierarchyNodePose *nodePose0, const a3_HierarchyNodePose *nodePose1,
	const float param0, const float param1)
{
	// first  "tree" example
	// - scale poses by weights
	// - add results
	a3_HierarchyNodePose tmpPose0[1], tmpPose1[1];
	a3hierarchyNodePoseScale_quat_internal(tmpPose0, nodePose0, param0);
	a3hierarchyNodePoseScale_quat_internal(tmpPose1, nodePose1, param1);
	a3hierarchyNodePoseConcat_quat_internal(nodePose_out, tmpPose0, tmpPose1);
}

inline void a3hierarchyNodePoseTriLerp_internal(a3_HierarchyNodePose *nodePose_out,
	const a3_HierarchyNodePose *nodePose0, const a3_HierarchyNodePose *nodePose1, const a3_HierarchyNodePose *nodePose2,
	const float param0, const float param1, const float param2)
{
	// second "tree" example
	a3_HierarchyNodePose tmpPose0[1], tmpPose1[1];

	a3hierarchyNodePoseBlend_internal(tmpPose0, nodePose0, nodePose1, param0, param1);
	a3hierarchyNodePoseScale_internal(tmpPose1, nodePose2, param2);
	a3hierarchyNodePoseConcat_internal(nodePose_out, tmpPose0, tmpPose1);
}

inline void a3hierarchyNodePoseTriLerp_quat_internal(a3_HierarchyNodePose *nodePose_out,
	const a3_HierarchyNodePose *nodePose0, const a3_HierarchyNodePose *nodePose1, const a3_HierarchyNodePose *nodePose2,
	const float param0, const float param1, const float param2)
{
	// second "tree" example
	a3_HierarchyNodePose tmpPose0[1], tmpPose1[1];

	a3hierarchyNodePoseBlend_quat_internal(tmpPose0, nodePose0, nodePose1, param0, param1);
	a3hierarchyNodePoseScale_quat_internal(tmpPose1, nodePose2, param2);
	a3hierarchyNodePoseConcat_quat_internal(nodePose_out, tmpPose0, tmpPose1);
}


// scale: Lerp from identity
inline void a3hierarchyPoseBlend_internal(a3_HierarchyPose *pose_out,
	const a3_HierarchyPose *pose0, const a3_HierarchyPose *pose1,
	const float param0, const float param1,
	const unsigned int nodeCount)
{
	a3_HierarchyNodePose *nodePose_out = pose_out->nodePose, *const end = nodePose_out + nodeCount;
	const a3_HierarchyNodePose *nodePose0 = pose0->nodePose, *nodePose1 = pose1->nodePose;
	while (nodePose_out < end)
	{
		a3hierarchyNodePoseBlend_internal(nodePose_out++, nodePose0++, nodePose1++, param0, param1);
	}
}

// scale: Lerp from identity
inline void a3hierarchyPoseBlend_quat_internal(a3_HierarchyPose *pose_out,
	const a3_HierarchyPose *pose0, const a3_HierarchyPose *pose1,
	const float param0, const float param1,
	const unsigned int nodeCount)
{
	a3_HierarchyNodePose *nodePose_out = pose_out->nodePose, *const end = nodePose_out + nodeCount;
	const a3_HierarchyNodePose *nodePose0 = pose0->nodePose, *nodePose1 = pose1->nodePose;
	while (nodePose_out < end)
	{
		a3hierarchyNodePoseBlend_quat_internal(nodePose_out++, nodePose0++, nodePose1++, param0, param1);
	}
}


inline void a3hierarchyPoseTriLerp_internal(a3_HierarchyPose *pose_out,
	const a3_HierarchyPose *pose0, const a3_HierarchyPose *pose1, const a3_HierarchyPose *pose2,
	const float param0, const float param1, const float param2,
	const unsigned int nodeCount)
{
	a3_HierarchyNodePose *nodePose_out = pose_out->nodePose, *const end = nodePose_out + nodeCount;
	const a3_HierarchyNodePose *nodePose0 = pose0->nodePose, *nodePose1 = pose1->nodePose, *nodePose2 = pose2->nodePose;
	while (nodePose_out < end)
	{
		a3hierarchyNodePoseTriLerp_internal(nodePose_out++, nodePose0++, nodePose1++, nodePose2++, param0, param1, param2);
	}
}

inline void a3hierarchyPoseTriLerp_quat_internal(a3_HierarchyPose *pose_out,
	const a3_HierarchyPose *pose0, const a3_HierarchyPose *pose1, const a3_HierarchyPose *pose2,
	const float param0, const float param1, const float param2,
	const unsigned int nodeCount)
{
	a3_HierarchyNodePose *nodePose_out = pose_out->nodePose, *const end = nodePose_out + nodeCount;
	const a3_HierarchyNodePose *nodePose0 = pose0->nodePose, *nodePose1 = pose1->nodePose, *nodePose2 = pose2->nodePose;
	while (nodePose_out < end)
	{
		a3hierarchyNodePoseTriLerp_quat_internal(nodePose_out++, nodePose0++, nodePose1++, nodePose2++, param0, param1, param2);
	}
}

// convert pose to transformation matrix
// different versions for efficiency when calling per-set functions
//	(significantly reduces the number of comparisons)
// all of the combos (wouldn't it be great if we had 'flags'): 
//	-> none
//	-> quaternion
//	-> quaternion, scale,
//	-> quaternion, scale, translate
//	-> quaternion, translate
//	-> euler
//	-> euler, scale,
//	-> euler, scale, translate
//	-> euler, translate
//	-> scale
//	-> scale, translate
//	-> translate
inline void a3hierarchyNodePoseConvert_identity_internal(p3mat4 *mat_out, const a3_HierarchyNodePose *nodePose)
{
	// 1. output identity
	p3real4x4SetIdentity(mat_out->m);
}

inline void a3hierarchyNodePoseConvert_quaternion_internal(p3mat4 *mat_out, const a3_HierarchyNodePose *nodePose)
{
	// 2. convert quaternion to matrix
	a3quatConvertToMat4(mat_out->m, nodePose->orientation.v, p3zeroVec3.v);
}

inline void a3hierarchyNodePoseConvert_quaternion_scale_internal(p3mat4 *mat_out, const a3_HierarchyNodePose *nodePose)
{
	// 3. convert quaternion to matrix
	a3quatConvertToMat4(mat_out->m, nodePose->orientation.v, p3zeroVec3.v);

	// adjust scale
	p3real3MulS(mat_out->v0.v, nodePose->scale.x);
	p3real3MulS(mat_out->v1.v, nodePose->scale.y);
	p3real3MulS(mat_out->v2.v, nodePose->scale.z);
}

inline void a3hierarchyNodePoseConvert_quaternion_scale_translate_internal(p3mat4 *mat_out, const a3_HierarchyNodePose *nodePose)
{
	// 4. convert quaternion to matrix, with translation
	a3quatConvertToMat4(mat_out->m, nodePose->orientation.v, nodePose->translation.v);

	// adjust scale
	p3real3MulS(mat_out->v0.v, nodePose->scale.x);
	p3real3MulS(mat_out->v1.v, nodePose->scale.y);
	p3real3MulS(mat_out->v2.v, nodePose->scale.z);
}

inline void a3hierarchyNodePoseConvert_quaternion_translate_internal(p3mat4 *mat_out, const a3_HierarchyNodePose *nodePose)
{
	// 5. convert quaternion to matrix, with translation
	a3quatConvertToMat4(mat_out->m, nodePose->orientation.v, nodePose->translation.v);
}

inline void a3hierarchyNodePoseConvert_euler_internal(p3mat4 *mat_out, const a3_HierarchyNodePose *nodePose)
{
	// 6. convert euler to matrix (assume ZYX, very common)
	p3real4x4SetRotateZYX(mat_out->m, nodePose->orientation.x, nodePose->orientation.y, nodePose->orientation.z);
}

inline void a3hierarchyNodePoseConvert_euler_scale_internal(p3mat4 *mat_out, const a3_HierarchyNodePose *nodePose)
{
	// 7. convert euler to matrix
	p3real4x4SetRotateZYX(mat_out->m, nodePose->orientation.x, nodePose->orientation.y, nodePose->orientation.z);

	// adjust scale
	p3real3MulS(mat_out->v0.v, nodePose->scale.x);
	p3real3MulS(mat_out->v1.v, nodePose->scale.y);
	p3real3MulS(mat_out->v2.v, nodePose->scale.z);
}

inline void a3hierarchyNodePoseConvert_euler_scale_translate_internal(p3mat4 *mat_out, const a3_HierarchyNodePose *nodePose)
{
	// 8. convert euler to matrix, with translation
	p3real4x4SetRotateZYX(mat_out->m, nodePose->orientation.x, nodePose->orientation.y, nodePose->orientation.z);
	mat_out->v3.xyz = nodePose->translation.xyz;

	// adjust scale
	p3real3MulS(mat_out->v0.v, nodePose->scale.x);
	p3real3MulS(mat_out->v1.v, nodePose->scale.y);
	p3real3MulS(mat_out->v2.v, nodePose->scale.z);
}

inline void a3hierarchyNodePoseConvert_euler_translate_internal(p3mat4 *mat_out, const a3_HierarchyNodePose *nodePose)
{
	// 9. convert euler to matrix, with translation
	p3real4x4SetRotateZYX(mat_out->m, nodePose->orientation.x, nodePose->orientation.y, nodePose->orientation.z);
	mat_out->v3.xyz = nodePose->translation.xyz;
}

inline void a3hierarchyNodePoseConvert_scale_internal(p3mat4 *mat_out, const a3_HierarchyNodePose *nodePose)
{
	// 10. scale
	p3real4x4SetIdentity(mat_out->m);
	mat_out->m00 = nodePose->scale.x;
	mat_out->m11 = nodePose->scale.y;
	mat_out->m22 = nodePose->scale.z;
}

inline void a3hierarchyNodePoseConvert_scale_translate_internal(p3mat4 *mat_out, const a3_HierarchyNodePose *nodePose)
{
	// 11. scale with translation
	p3real4x4SetIdentity(mat_out->m);
	mat_out->m00 = nodePose->scale.x;
	mat_out->m11 = nodePose->scale.y;
	mat_out->m22 = nodePose->scale.z;
	mat_out->v3.xyz = nodePose->translation.xyz;
}

inline void a3hierarchyNodePoseConvert_translate_internal(p3mat4 *mat_out, const a3_HierarchyNodePose *nodePose)
{
	// 12. translation
	p3real4x4SetIdentity(mat_out->m);
	mat_out->v3.xyz = nodePose->translation.xyz;
}


// same as the above with loops
inline void a3hierarchyPoseConvert_identity_internal(const a3_HierarchyTransform *transform_out, const a3_HierarchyPose *pose, const unsigned int nodeCount)
{
	p3mat4 *mat_out = transform_out->transform;
	const a3_HierarchyNodePose *nodePose = pose->nodePose, *const end = nodePose + nodeCount;
	while (nodePose < end)
		a3hierarchyNodePoseConvert_identity_internal(mat_out++, nodePose++);
}

inline void a3hierarchyPoseConvert_quaternion_internal(const a3_HierarchyTransform *transform_out, const a3_HierarchyPose *pose, const unsigned int nodeCount)
{
	p3mat4 *mat_out = transform_out->transform;
	const a3_HierarchyNodePose *nodePose = pose->nodePose, *const end = nodePose + nodeCount;
	while (nodePose < end)
		a3hierarchyNodePoseConvert_quaternion_internal(mat_out++, nodePose++);
}

inline void a3hierarchyPoseConvert_quaternion_scale_internal(const a3_HierarchyTransform *transform_out, const a3_HierarchyPose *pose, const unsigned int nodeCount)
{
	p3mat4 *mat_out = transform_out->transform;
	const a3_HierarchyNodePose *nodePose = pose->nodePose, *const end = nodePose + nodeCount;
	while (nodePose < end)
		a3hierarchyNodePoseConvert_quaternion_scale_internal(mat_out++, nodePose++);
}

inline void a3hierarchyPoseConvert_quaternion_scale_translate_internal(const a3_HierarchyTransform *transform_out, const a3_HierarchyPose *pose, const unsigned int nodeCount)
{
	p3mat4 *mat_out = transform_out->transform;
	const a3_HierarchyNodePose *nodePose = pose->nodePose, *const end = nodePose + nodeCount;
	while (nodePose < end)
		a3hierarchyNodePoseConvert_quaternion_scale_translate_internal(mat_out++, nodePose++);
}

inline void a3hierarchyPoseConvert_quaternion_translate_internal(const a3_HierarchyTransform *transform_out, const a3_HierarchyPose *pose, const unsigned int nodeCount)
{
	p3mat4 *mat_out = transform_out->transform;
	const a3_HierarchyNodePose *nodePose = pose->nodePose, *const end = nodePose + nodeCount;
	while (nodePose < end)
		a3hierarchyNodePoseConvert_quaternion_translate_internal(mat_out++, nodePose++);
}

inline void a3hierarchyPoseConvert_euler_internal(const a3_HierarchyTransform *transform_out, const a3_HierarchyPose *pose, const unsigned int nodeCount)
{
	p3mat4 *mat_out = transform_out->transform;
	const a3_HierarchyNodePose *nodePose = pose->nodePose, *const end = nodePose + nodeCount;
	while (nodePose < end)
		a3hierarchyNodePoseConvert_euler_internal(mat_out++, nodePose++);
}

inline void a3hierarchyPoseConvert_euler_scale_internal(const a3_HierarchyTransform *transform_out, const a3_HierarchyPose *pose, const unsigned int nodeCount)
{
	p3mat4 *mat_out = transform_out->transform;
	const a3_HierarchyNodePose *nodePose = pose->nodePose, *const end = nodePose + nodeCount;
	while (nodePose < end)
		a3hierarchyNodePoseConvert_euler_scale_internal(mat_out++, nodePose++);
}

inline void a3hierarchyPoseConvert_euler_scale_translate_internal(const a3_HierarchyTransform *transform_out, const a3_HierarchyPose *pose, const unsigned int nodeCount)
{
	p3mat4 *mat_out = transform_out->transform;
	const a3_HierarchyNodePose *nodePose = pose->nodePose, *const end = nodePose + nodeCount;
	while (nodePose < end)
		a3hierarchyNodePoseConvert_euler_scale_translate_internal(mat_out++, nodePose++);
}

inline void a3hierarchyPoseConvert_euler_translate_internal(const a3_HierarchyTransform *transform_out, const a3_HierarchyPose *pose, const unsigned int nodeCount)
{
	p3mat4 *mat_out = transform_out->transform;
	const a3_HierarchyNodePose *nodePose = pose->nodePose, *const end = nodePose + nodeCount;
	while (nodePose < end)
		a3hierarchyNodePoseConvert_euler_translate_internal(mat_out++, nodePose++);
}

inline void a3hierarchyPoseConvert_scale_internal(const a3_HierarchyTransform *transform_out, const a3_HierarchyPose *pose, const unsigned int nodeCount)
{
	p3mat4 *mat_out = transform_out->transform;
	const a3_HierarchyNodePose *nodePose = pose->nodePose, *const end = nodePose + nodeCount;
	while (nodePose < end)
		a3hierarchyNodePoseConvert_scale_internal(mat_out++, nodePose++);
}

inline void a3hierarchyPoseConvert_scale_translate_internal(const a3_HierarchyTransform *transform_out, const a3_HierarchyPose *pose, const unsigned int nodeCount)
{
	p3mat4 *mat_out = transform_out->transform;
	const a3_HierarchyNodePose *nodePose = pose->nodePose, *const end = nodePose + nodeCount;
	while (nodePose < end)
		a3hierarchyNodePoseConvert_scale_translate_internal(mat_out++, nodePose++);
}

inline void a3hierarchyPoseConvert_translate_internal(const a3_HierarchyTransform *transform_out, const a3_HierarchyPose *pose, const unsigned int nodeCount)
{
	p3mat4 *mat_out = transform_out->transform;
	const a3_HierarchyNodePose *nodePose = pose->nodePose, *const end = nodePose + nodeCount;
	while (nodePose < end)
		a3hierarchyNodePoseConvert_translate_internal(mat_out++, nodePose++);
}


//-----------------------------------------------------------------------------

// initialize pose set given an initialized hierarchy and key pose count
extern inline int a3hierarchyPoseGroupCreate(a3_HierarchyPoseGroup *poseGroup_out, const a3_Hierarchy *hierarchy, const unsigned int poseCount)
{
	if (poseGroup_out && hierarchy && !poseGroup_out->hierarchy && hierarchy->nodes)
	{
		// allocate contiguous list of size: nodes * poses
		const unsigned int nodeCount = hierarchy->numNodes;
		const unsigned int totalPoses = nodeCount * poseCount;
		unsigned int i;

		// pointer to pose list for current node
		a3_HierarchyNodePose *nodePosePtr;
		a3_HierarchyPose *posePtr;

		// set hierarchy and count
		poseGroup_out->hierarchy = hierarchy;
		poseGroup_out->poseCount = poseCount;

		// allocate contiguous data and pointers
		poseGroup_out->nodePoseContiguous = (a3_HierarchyNodePose *)malloc(totalPoses * sizeof(a3_HierarchyNodePose) + poseCount * sizeof(a3_HierarchyPose *));
		poseGroup_out->pose = (a3_HierarchyPose *)(poseGroup_out->nodePoseContiguous + totalPoses);

		// set all pointers and reset all poses
		for (i = 0, nodePosePtr = poseGroup_out->nodePoseContiguous, posePtr = poseGroup_out->pose; 
			i < poseCount; 
			++i, nodePosePtr += nodeCount, ++posePtr)
		{
			posePtr->nodePose = nodePosePtr;
			a3hierarchyPoseReset_internal(posePtr, nodeCount);
		}

		// return pose count
		return poseCount;
	}
	return -1;
}

// release pose set
extern inline int a3hierarchyPoseGroupRelease(a3_HierarchyPoseGroup *poseGroup)
{
	if (poseGroup && poseGroup->hierarchy)
	{
		free(poseGroup->nodePoseContiguous);
		poseGroup->hierarchy = 0;
		poseGroup->nodePoseContiguous = 0;
		poseGroup->pose = 0;
		poseGroup->poseCount = 0;

		// done
		return 1;
	}
	return -1;
}

// get offset to hierarchy pose in contiguous set
extern inline int a3hierarchyPoseGroupGetPoseOffsetIndex(const a3_HierarchyPoseGroup *poseGroup, const unsigned int poseIndex)
{
	if (poseGroup && poseGroup->hierarchy)
		return (poseIndex * poseGroup->hierarchy->numNodes);
	return -1;
}

// get offset to single node pose in contiguous set
extern inline int a3hierarchyPoseGroupGetNodePoseOffsetIndex(const a3_HierarchyPoseGroup *poseGroup, const unsigned int poseIndex, const unsigned int nodeIndex)
{
	if (poseGroup && poseGroup->hierarchy)
		return (poseIndex * poseGroup->hierarchy->numNodes + nodeIndex);
	return -1;
}


//-----------------------------------------------------------------------------

// initialize hierarchy state given an initialized hierarchy
extern inline int a3hierarchyStateCreate(a3_HierarchyState *state_out, const a3_HierarchyPoseGroup *poseGroup)
{
	// validate params and initialization states
	//	(output is not yet initialized, hierarchy is initialized)
	if (state_out && poseGroup && !state_out->poseGroup && poseGroup->hierarchy && poseGroup->hierarchy->nodes)
	{
		const a3_Hierarchy *hierarchy = poseGroup->hierarchy;

		const unsigned int count = hierarchy->numNodes;
		const unsigned int count2 = count + count;
		unsigned int i;

		// set pose set pointer
		state_out->poseGroup = poseGroup;

		// allocate set of matrices in state
		//	('local' points to contiguous array of all matrices and floats)
		state_out->localPose->nodePose = (a3_HierarchyNodePose *)malloc(count2 * sizeof(p3mat4) + count * sizeof(a3_HierarchyNodePose));
		state_out->localSpace->transform = (p3mat4 *)(state_out->localPose->nodePose + count);
		state_out->objectSpace->transform = (p3mat4 *)(state_out->localSpace->transform + count);

		// set all matrices to identity
		for (i = 0; i < count2; ++i)
			p3real4x4SetIdentity(state_out->localSpace->transform[i].m);

		// set all poses to default values
		a3hierarchyPoseReset_internal(state_out->localPose, hierarchy->numNodes);

		// return number of nodes
		return count;
	}
	return -1;
}


// release hierarchy state
extern inline int a3hierarchyStateRelease(a3_HierarchyState *state)
{
	// validate param exists and is initialized
	if (state && state->poseGroup)
	{
		// release matrices
		//	(local points to contiguous array of all matrices)
		free(state->localPose->nodePose);

		// reset pointers
		state->localPose->nodePose = 0;
		state->localSpace->transform = 0;
		state->objectSpace->transform = 0;

		// done
		return 1;
	}
	return -1;
}


//-----------------------------------------------------------------------------
// ****TO-DO: implement single-node blend operations
// ****TO-DO: implement full-pose blend operations

// reset single node pose
extern inline int a3hierarchyNodePoseReset(a3_HierarchyNodePose *nodePose_inout)
{
	if (nodePose_inout)
	{
		a3hierarchyNodePoseReset_internal(nodePose_inout);
		return 1;
	}
	return -1;
}

// copy single node pose
extern inline int a3hierarchyNodePoseCopy(a3_HierarchyNodePose *nodePose_out, const a3_HierarchyNodePose *copyNodePose)
{
	if (nodePose_out && copyNodePose)
	{
		a3hierarchyNodePoseCopy_internal(nodePose_out, copyNodePose);
		return 1;
	}
	return -1;
}

// invert single node pose
extern inline int a3hierarchyNodePoseInvert(a3_HierarchyNodePose *nodePose_out, const a3_HierarchyNodePose *invertNodePose)
{
	if (nodePose_out && invertNodePose)
	{
		a3hierarchyNodePoseInvert_internal(nodePose_out, invertNodePose);
		return 1;
	}
	return -1;
}

// LERP single node pose
extern inline int a3hierarchyNodePoseLERP(a3_HierarchyNodePose *nodePose_out, const a3_HierarchyNodePose *nodePose0, const a3_HierarchyNodePose *nodePose1, const float param, const a3_HierarchyPoseFlag flag)
{
	if (nodePose_out && nodePose0 && nodePose1)
	{
		if (flag & a3poseFlag_quat)
			a3hierarchyNodePoseLerp_quat_internal(nodePose_out, nodePose0, nodePose1, param);
		else
			a3hierarchyNodePoseLerp_internal(nodePose_out, nodePose0, nodePose1, param);
		return 1;
	}
	return -1;
}

// add/concat single node pose
extern inline int a3hierarchyNodePoseConcat(a3_HierarchyNodePose *nodePose_out, const a3_HierarchyNodePose *nodePose0, const a3_HierarchyNodePose *nodePose1, const a3_HierarchyPoseFlag flag)
{
	if (nodePose_out && nodePose0 && nodePose1)
	{
		if (flag & a3poseFlag_quat)
			a3hierarchyNodePoseConcat_quat_internal(nodePose_out, nodePose0, nodePose1);
		else
			a3hierarchyNodePoseConcat_internal(nodePose_out, nodePose0, nodePose1);
		return 1;
	}
	return -1;
}

// scale single node pose
extern inline int a3hierarchyNodePoseScale(a3_HierarchyNodePose *nodePose_out, const a3_HierarchyNodePose *nodePoseScale, const float param, const a3_HierarchyPoseFlag flag)
{
	if (nodePose_out && nodePoseScale)
	{
		if (flag & a3poseFlag_quat)
			a3hierarchyNodePoseScale_quat_internal(nodePose_out, nodePoseScale, param);
		else
			a3hierarchyNodePoseScale_internal(nodePose_out, nodePoseScale, param);
		return 1;
	}
	return -1;
}

// blend single node pose
extern inline int a3hierarchyNodePoseBlend(a3_HierarchyNodePose *nodePose_out, const a3_HierarchyNodePose *nodePose0, const a3_HierarchyNodePose *nodePose1, const float weight0, const float weight1, const a3_HierarchyPoseFlag flag)
{
	if (nodePose_out && nodePose0 && nodePose1)
	{
		if (flag & a3poseFlag_quat)
			a3hierarchyNodePoseBlend_quat_internal(nodePose_out, nodePose0, nodePose1, weight0, weight1);
		else
			a3hierarchyNodePoseBlend_internal(nodePose_out, nodePose0, nodePose1, weight0, weight1);
		return 1;
	}
	return -1;
}

// triangular LERP single node pose
extern inline int a3hierarchyNodePoseTriangularLERP(a3_HierarchyNodePose *nodePose_out, const a3_HierarchyNodePose *nodePose0, const a3_HierarchyNodePose *nodePose1, const a3_HierarchyNodePose *nodePose2, const float param0, const float param1, const a3_HierarchyPoseFlag flag)
{
	if (nodePose_out && nodePose0 && nodePose1 && nodePose2)
	{
		const float param2 = 1.0f - param0 - param1;

		if (flag & a3poseFlag_quat)
			a3hierarchyNodePoseTriLerp_quat_internal(nodePose_out, nodePose0, nodePose1, nodePose2, param0, param1, param2);
		else
			a3hierarchyNodePoseTriLerp_internal(nodePose_out, nodePose0, nodePose1, nodePose2, param0, param1, param2);
		return 1;
	}
	return -1;
}

// convert single node pose to matrix
extern inline int a3hierarchyNodePoseConvert(p3mat4 *mat_out, const a3_HierarchyNodePose *nodePose, const a3_HierarchyPoseFlag flag)
{
	// switch looks ugly but will potentially save a ton of processing time
	// NOTE: sort cases by LIKELIHOOD to save even more time
	//	probably start with revolute (rotating) transforms
	if (mat_out && nodePose)
	{
		switch (flag)
		{
			// pure cases
		case (a3poseFlag_rotate_q):
			// pure revolute
			a3hierarchyNodePoseConvert_quaternion_internal(mat_out, nodePose);
			break;
		case (a3poseFlag_rotate):
			// pure revolute Euler
			a3hierarchyNodePoseConvert_euler_internal(mat_out, nodePose);
			break;
		case (a3poseFlag_translate):
			// pure prismatic
			a3hierarchyNodePoseConvert_translate_internal(mat_out, nodePose);
			break;
		case (a3poseFlag_scale):
			// non-rigid
			a3hierarchyNodePoseConvert_scale_internal(mat_out, nodePose);
			break;

			// combo cases
		case (a3poseFlag_rotate_q | a3poseFlag_translate):
			// revolute, prismatic
			a3hierarchyNodePoseConvert_quaternion_translate_internal(mat_out, nodePose);
			break;
		case (a3poseFlag_rotate | a3poseFlag_translate):
			// revolute Euler, prismatic
			a3hierarchyNodePoseConvert_euler_translate_internal(mat_out, nodePose);
			break;
		case (a3poseFlag_rotate_q | a3poseFlag_scale):
			// revolute, non-rigid
			a3hierarchyNodePoseConvert_quaternion_scale_internal(mat_out, nodePose);
			break;
		case (a3poseFlag_rotate | a3poseFlag_scale):
			// revolute Euler, non-rigid
			a3hierarchyNodePoseConvert_euler_scale_internal(mat_out, nodePose);
			break;
		case (a3poseFlag_translate | a3poseFlag_scale):
			// prismatic, non-rigid
			a3hierarchyNodePoseConvert_scale_translate_internal(mat_out, nodePose);
			break;

			// complete cases
		case (a3poseFlag_rotate_q | a3poseFlag_translate | a3poseFlag_scale):
			// revolute, prismatic, non-rigid
			a3hierarchyNodePoseConvert_quaternion_scale_translate_internal(mat_out, nodePose);
			break;
		case (a3poseFlag_rotate | a3poseFlag_translate | a3poseFlag_scale):
			// revolute Euler, prismatic, non-rigid
			a3hierarchyNodePoseConvert_euler_scale_translate_internal(mat_out, nodePose);
			break;

			// no transform case
		default: 
			// none
			a3hierarchyNodePoseConvert_identity_internal(mat_out, nodePose);
			break;
		}

		// end
		return 1;
	}
	return -1;
}


// reset full hierarchy pose
extern inline int a3hierarchyPoseReset(const a3_HierarchyPose *pose_inout, const unsigned int nodeCount)
{
	if (pose_inout && pose_inout->nodePose)
	{
		a3hierarchyPoseReset_internal(pose_inout, nodeCount);
		return nodeCount;
	}
	return -1;
}

// copy full hierarchy pose
extern inline int a3hierarchyPoseCopy(const a3_HierarchyPose *pose_out, const a3_HierarchyPose *copyPose, const unsigned int nodeCount)
{
	if (pose_out && copyPose && pose_out->nodePose && copyPose->nodePose)
	{
		a3hierarchyPoseCopy_internal(pose_out, copyPose, nodeCount);
		return nodeCount;
	}
	return -1;
}

// invert full hierarchy pose
extern inline int a3hierarchyPoseInvert(const a3_HierarchyPose *pose_out, const a3_HierarchyPose *invertPose, const unsigned int nodeCount)
{
	if (pose_out && invertPose && pose_out->nodePose && invertPose->nodePose)
	{
		a3hierarchyPoseInvert_internal(pose_out, invertPose, nodeCount);
		return nodeCount;
	}
	return -1;
}

// LERP full hierarchy pose
extern inline int a3hierarchyPoseLERP(const a3_HierarchyPose *pose_out, const a3_HierarchyPose *pose0, const a3_HierarchyPose *pose1, const float param, const unsigned int nodeCount, const a3_HierarchyPoseFlag flag)
{
	if (pose_out && pose0 && pose1 && pose_out->nodePose && pose0->nodePose && pose1->nodePose)
	{
		if (flag & a3poseFlag_quat)
			a3hierarchyPoseLerp_quat_internal(pose_out, pose0, pose1, param, nodeCount);
		else
			a3hierarchyPoseLerp_internal(pose_out, pose0, pose1, param, nodeCount);
		return nodeCount;
	}
	return -1;
}

// add/concat full hierarchy pose
extern inline int a3hierarchyPoseConcat(const a3_HierarchyPose *pose_out, const a3_HierarchyPose *pose0, const a3_HierarchyPose *pose1, const unsigned int nodeCount, const a3_HierarchyPoseFlag flag)
{
	if (pose_out && pose0 && pose1 && pose_out->nodePose && pose0->nodePose && pose1->nodePose)
	{
		if (flag & a3poseFlag_quat)
			a3hierarchyPoseConcat_quat_internal(pose_out, pose0, pose1, nodeCount);
		else
			a3hierarchyPoseConcat_internal(pose_out, pose0, pose1, nodeCount);
		return nodeCount;
	}
	return -1;
}

// scale full hierarchy pose
extern inline int a3hierarchyPoseScale(const a3_HierarchyPose *pose_out, const a3_HierarchyPose *poseScale, const float param, const unsigned int nodeCount, const a3_HierarchyPoseFlag flag)
{
	if (pose_out && poseScale && pose_out->nodePose && poseScale->nodePose)
	{
		if (flag & a3poseFlag_quat)
			a3hierarchyPoseScale_quat_internal(pose_out, poseScale, param, nodeCount);
		else
			a3hierarchyPoseScale_internal(pose_out, poseScale, param, nodeCount);
		return nodeCount;
	}
	return -1;
}

// blend full hierarchy pose
extern inline int a3hierarchyPoseBlend(const a3_HierarchyPose *pose_out, const a3_HierarchyPose *pose0, const a3_HierarchyPose *pose1, const float weight0, const float weight1, const unsigned int nodeCount, const a3_HierarchyPoseFlag flag)
{
	if (pose_out && pose0 && pose1 && pose_out->nodePose && pose0->nodePose && pose1->nodePose)
	{
		if (flag & a3poseFlag_quat)
			a3hierarchyPoseBlend_quat_internal(pose_out, pose0, pose1, weight0, weight1, nodeCount);
		else
			a3hierarchyPoseBlend_internal(pose_out, pose0, pose1, weight0, weight1, nodeCount);
		return nodeCount;
	}
	return -1;
}

// triangular LERP full hierarchy pose
extern inline int a3hierarchyPoseTriangularLERP(const a3_HierarchyPose *pose_out, const a3_HierarchyPose *pose0, const a3_HierarchyPose *pose1, const a3_HierarchyPose *pose2, const float param0, const float param1, const unsigned int nodeCount, const a3_HierarchyPoseFlag flag)
{
	if (pose_out && pose0 && pose1 && pose2 && pose_out->nodePose && pose0->nodePose && pose1->nodePose && pose2->nodePose)
	{
		const float param2 = 1.0f - param0 - param1;

		if (flag & a3poseFlag_quat)
			a3hierarchyPoseTriLerp_quat_internal(pose_out, pose0, pose1, pose2, param0, param1, param2, nodeCount);
		else
			a3hierarchyPoseTriLerp_internal(pose_out, pose0, pose1, pose2, param0, param1, param2, nodeCount);
		return nodeCount;
	}
	return -1;
}

// convert full hierarchy pose to hierarchy transforms
extern inline int a3hierarchyPoseConvert(const a3_HierarchyTransform *transform_out, const a3_HierarchyPose *pose, const unsigned int nodeCount, const a3_HierarchyPoseFlag flag)
{
	if (transform_out && pose && transform_out->transform && pose->nodePose)
	{
		switch (flag)
		{
			// pure cases
		case (a3poseFlag_rotate_q):
			// pure revolute
			a3hierarchyPoseConvert_quaternion_internal(transform_out, pose, nodeCount);
			break;
		case (a3poseFlag_rotate):
			// pure revolute Euler
			a3hierarchyPoseConvert_euler_internal(transform_out, pose, nodeCount);
			break;
		case (a3poseFlag_translate):
			// pure prismatic
			a3hierarchyPoseConvert_translate_internal(transform_out, pose, nodeCount);
			break;
		case (a3poseFlag_scale):
			// non-rigid
			a3hierarchyPoseConvert_scale_internal(transform_out, pose, nodeCount);
			break;

			// combo cases
		case (a3poseFlag_rotate_q | a3poseFlag_translate):
			// revolute, prismatic
			a3hierarchyPoseConvert_quaternion_translate_internal(transform_out, pose, nodeCount);
			break;
		case (a3poseFlag_rotate | a3poseFlag_translate):
			// revolute Euler, prismatic
			a3hierarchyPoseConvert_euler_translate_internal(transform_out, pose, nodeCount);
			break;
		case (a3poseFlag_rotate_q | a3poseFlag_scale):
			// revolute, non-rigid
			a3hierarchyPoseConvert_quaternion_scale_internal(transform_out, pose, nodeCount);
			break;
		case (a3poseFlag_rotate | a3poseFlag_scale):
			// revolute Euler, non-rigid
			a3hierarchyPoseConvert_euler_scale_internal(transform_out, pose, nodeCount);
			break;
		case (a3poseFlag_translate | a3poseFlag_scale):
			// prismatic, non-rigid
			a3hierarchyPoseConvert_scale_translate_internal(transform_out, pose, nodeCount);
			break;

			// complete cases
		case (a3poseFlag_rotate_q | a3poseFlag_translate | a3poseFlag_scale):
			// revolute, prismatic, non-rigid
			a3hierarchyPoseConvert_quaternion_scale_translate_internal(transform_out, pose, nodeCount);
			break;
		case (a3poseFlag_rotate | a3poseFlag_translate | a3poseFlag_scale):
			// revolute Euler, prismatic, non-rigid
			a3hierarchyPoseConvert_euler_scale_translate_internal(transform_out, pose, nodeCount);
			break;

			// no transform case
		default:
			// none
			a3hierarchyPoseConvert_identity_internal(transform_out, pose, nodeCount);
			break;
		}

		// end
		return nodeCount;
	}
	return -1;
}


//-----------------------------------------------------------------------------
