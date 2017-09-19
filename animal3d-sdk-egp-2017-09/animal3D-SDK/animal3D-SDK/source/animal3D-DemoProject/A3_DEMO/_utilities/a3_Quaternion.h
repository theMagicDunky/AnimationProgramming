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
	
	a3_Quaternion.h
	Quaternion utilities and algorithms.
*/

#ifndef __ANIMAL3D_QUATERNION_H
#define __ANIMAL3D_QUATERNION_H


// math library
#include "P3DM/P3DM.h"


//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif	// __cplusplus


//-----------------------------------------------------------------------------

	// quaternion declaration
	typedef p3real4		a3quat;
	typedef p3real4p	a3quatp;


//-----------------------------------------------------------------------------

	// create identity quaternion
	inline int a3quatCreateIdentity(a3quatp q_out);

	// create quaternion from normalized axis and angle
	inline int a3quatCreateAxisAngle(a3quatp q_out, const p3real3p axis_unit, const p3real angle_degrees);

	// create quaternion from two normalized end vectors
	inline int a3quatCreateDelta(a3quatp q_out, const p3real3p v0, const p3real3p v1);

	// extract axis-angle from quaternion
	inline int a3quatGetAxisAngle(p3real3p axis_out, p3real *angle_degrees_out, const a3quatp q);

	// conjugate
	inline int a3quatConjugate(a3quatp qConj_out, const a3quatp q);

	// inverse
	inline int a3quatInverse(a3quatp qInv_out, const a3quatp q);

	// concatenate (multiplication)
	inline int a3quatConcat(a3quatp qConcat_out, const a3quatp qL, const a3quatp qR);

	// rotate 3D vector
	inline int a3quatRotateVec3(p3real3p vRot_out, const a3quatp q, const p3real3p v);

	// rotate 4D vector/point
	inline int a3quatRotateVec4(p3real4p vRot_out, const a3quatp q, const p3real4p v);

	// SLERP between two unit quaternions
	inline int a3quatUnitSLERP(a3quatp qSlerp_out, const a3quatp q0_unit, const a3quatp q1_unit, const p3real t);

	// convert to mat3
	inline int a3quatConvertToMat3(p3real3x3p m_out, const a3quatp q);

	// convert to mat4 with translation
	inline int a3quatConvertToMat4(p3real4x4p m_out, const a3quatp q, const p3real3p translate);


//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif	// __cplusplus


#endif	// !__ANIMAL3D_QUATERNION_H