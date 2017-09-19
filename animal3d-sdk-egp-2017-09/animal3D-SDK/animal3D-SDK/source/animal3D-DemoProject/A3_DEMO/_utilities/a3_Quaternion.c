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
	
	a3_Quaternion.c
	Quaternion utility implementations.
*/

#include "a3_Quaternion.h"


//-----------------------------------------------------------------------------

// create identity quaternion
extern inline int a3quatCreateIdentity(a3quatp q_out)
{
	if (q_out)
	{
		// xyz = 0
		// w = 1
		q_out[0] = q_out[1] = q_out[2] = realZero;
		q_out[3] = realOne;

		// done
		return 1;
	}
	return 0;
}

// create quaternion from normalized axis and angle
extern inline int a3quatCreateAxisAngle(a3quatp q_out, const p3real3p axis_unit, const p3real angle_degrees)
{
	if (q_out && axis_unit)
	{
		// ****TO-DO: implement
		// v = sin(angle / 2) * n
		// w = cos(angle / 2)
		const float halfAngle = 0.5f * angle_degrees;
		const float c = p3cosd(halfAngle);
		const float s = p3sind(halfAngle);

		q_out[0] = s * axis_unit[0]; //x
		q_out[1] = s * axis_unit[1]; //y
		q_out[2] = s * axis_unit[2]; //z
		q_out[3] = c;				 //w

		// done
		return 1;
	}
	return 0;
}

extern inline int a3quatCreateDelta(a3quatp q_out, const p3real3p v0_unit, const p3real3p v1_unit)
{
	if (q_out && v0_unit && v1_unit)
	{
		// ****TO-DO: implement
		// SUPER PRO TIP for fast quaternion creation: 
		// Here are some fun facts about unit vectors: 
		//	-> a  dot  b = cos(angle)
		//	-> a cross b = sin(angle) * n
		// Since a quaternion uses half angle, we can solve by using 
		//	the unit halfway vector as 'b'!!!



		// done
		return 1;
	}
	return 0;
}

// extract axis-angle from quaternion
extern inline int a3quatGetAxisAngle(p3real3p axis_out, p3real *angle_degrees_out, const a3quatp q)
{
	if (axis_out && angle_degrees_out && q)
	{
		// ****TO-DO: implement
		// if w is between +/- 1, 
		//	-> extract axis by normalizing vector part
		//	-> extract angle by taking inverse cosine of W and double it
		// else
		//	-> return all zeros

		// done
		return 1;
	}
	return 0;
}

// conjugate
extern inline int a3quatConjugate(a3quatp qConj_out, const a3quatp q)
{
	if (qConj_out && q)
	{
		// ****TO-DO: implement
		// vector part is negative

		// done
		return 1;
	}
	return 0;
}

// inverse
extern inline int a3quatInverse(a3quatp qInv_out, const a3quatp q)
{
	if (qInv_out && q)
	{
		// ****TO-DO: implement
		// conjugate divided by squared magnitude

		// done
		return 1;
	}
	return 0;
}

// concatenate (multiplication)
extern inline int a3quatConcat(a3quatp qConcat_out, const a3quatp qL, const a3quatp qR)
{
	if (qConcat_out && qL && qR)
	{
		// ****TO-DO: implement
		// use full formula, it's faster: 
		//	x = w0x1 + x0w1 + y0z1 - z0y1
		//	y = w0y1 - x0z1 + y0w1 + z0x1
		//	z = w0z1 + x0y1 - y0x1 + z0w1
		//	w = w0w1 - x0x1 - y0y1 - z0z1

		// done
		return 1;
	}
	return 0;
}

// rotate 3D vector
extern inline int a3quatRotateVec3(p3real3p vRot_out, const a3quatp q, const p3real3p v)
{
	if (vRot_out && q && v)
	{
		// ****TO-DO: implement
		// expand shortened formula: 
		//	v' = v + (r + r)x(r x v + wv)

		// done
		return 1;
	}
	return 0;
}

// rotate 4D vector/point
extern inline int a3quatRotateVec4(p3real4p vRot_out, const a3quatp q, const p3real4p v)
{
	if (vRot_out && q && v)
	{
		// ****TO-DO: implement
		// same as above but set w component

		// done
		return 1;
	}
	return 0;
}

// SLERP between two unit quaternions
extern inline int a3quatUnitSLERP(a3quatp qSlerp_out, const a3quatp q0_unit, const a3quatp q1_unit, const p3real t)
{
	if (qSlerp_out && q0_unit && q1_unit)
	{
		// ****TO-DO: implement
		// PRO TIP: if "angle" is negative, flip second quaternion
		// PRO TIP: raw SLERP formula is not enough; what if inputs are parallel?
		
		// q = ((sin(1-t)theta)q0 + sin(t(theta))q1) / sin(theta)
		// theta = acos(q0 dot q1)

		float d = p3real4Dot(q0_unit, q1_unit);

		float theta = p3acosd(d);
		float sInv = 1.0f / p3sind(theta);
		float s0 = p3sind((1.0f - t)*theta);
		float s1 = p3sind(t * theta);

		qSlerp_out[0] = s0 * q0_unit[0] + s1 * q1_unit[0];
		qSlerp_out[1] = s0 * q0_unit[1] + s1 * q1_unit[1];
		qSlerp_out[2] = s0 * q0_unit[2] + s1 * q1_unit[2];
		qSlerp_out[3] = s0 * q0_unit[3] + s1 * q1_unit[3];

		// done
		return 1;
	}
	return 0;
}

// convert to mat3
extern inline int a3quatConvertToMat3(p3real3x3p m_out, const a3quatp q)
{
	if (m_out && q)
	{
		// ****TO-DO: implement
		// start by writing shortcuts, then apply conversion formula
		// NOTE: matrices are COLUMN-MAJOR; index like this: 
		//	m_out[column][row]
		//	e.g. upper-right would be m_out[2][0]

		// done
		return 1;
	}
	return 0;
}

// convert to mat4 with translation
extern inline int a3quatConvertToMat4(p3real4x4p m_out, const a3quatp q, const p3real3p translate)
{
	if (m_out && q)
	{
		// ****TO-DO: implement
		// same as above but copy translate into fourth column
		//	and setting bottom row to (0, 0, 0, 1)
		// NOTE: matrices are COLUMN-MAJOR

		//squares
		const float WW = q[3] * q[3], XX = q[0] * q[0], YY = q[1] * q[1], ZZ = q[2] * q[2];
		const float XY2 = q[0] * q[1] * 2.0f, XZ2 = q[0] * q[2] * 2.0f, YZ2 = q[1] * q[2] * 2.0f, WX2 = q[3] * q[0] * 2.0f, WY2 = q[3] * q[1] * 2.0f, WZ2 = q[2] * q[3] * 2.0f;

		m_out[0][0] = WW + XX - YY - ZZ;
		m_out[0][1] = XY2 + WZ2;
		m_out[0][2] = XZ2 - WY2;

		m_out[1][0] = XY2 - WZ2;
		m_out[1][1] = WW - XX + YY - ZZ;
		m_out[1][2] = YZ2 + WX2;

		m_out[2][0] = XZ2 + WY2;
		m_out[2][1] = YZ2 - WX2;
		m_out[2][2] = WW - XX - YY + ZZ;

		m_out[3][0] = translate[0];
		m_out[3][1] = translate[1];
		m_out[3][2] = translate[2];

		m_out[0][3] = m_out[1][3] = m_out[2][3] = 0.0f;
		m_out[3][3] = 1.0f;

		// done
		return 1;
	}
	return 0;
}


//-----------------------------------------------------------------------------