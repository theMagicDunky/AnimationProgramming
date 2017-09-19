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
	
	a3_RayPicking.h
	Ray casting and picking algorithms.
*/

#ifndef __ANIMAL3D_RAYPICKING_H
#define __ANIMAL3D_RAYPICKING_H


// math library
#include "P3DM/P3DM.h"


//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#else	// !__cplusplus
	typedef struct a3_Ray		a3_Ray;
	typedef struct a3_RayHit	a3_RayHit;
	typedef struct a3_Sphere	a3_Sphere;
#endif	// __cplusplus

	
//-----------------------------------------------------------------------------

	// general scene objects
	struct a3_Ray
	{
		p3vec4 origin;			// point of origin (w = 1)
		p3vec4 direction;		// direction vector (w = 0)
	};

	struct a3_RayHit
	{
		p3vec4 hit0, hit1;		// first and second point of intersection
		p3real param0, param1;	// relative scales along source ray
	};


	// shapes
	struct a3_Sphere
	{
		p3vec4 center;			// center point (w = 1)
		p3real radius;			// radius of sphere (center to surface)
	};


//-----------------------------------------------------------------------------

	// create ray given start and end points
	inline int a3rayCreate(a3_Ray *ray_out, const p3vec3 origin, const p3vec3 end);

	// create ray given NDC coordinate and an inverse projection matrix
	inline int a3rayCreateUnprojected(a3_Ray *ray_out, const p3vec3 v_ndc, const p3mat4 invProj);

	// pick against shapes
	inline int a3rayTestSphere(a3_RayHit *hit_out, const a3_Ray *ray, const a3_Sphere *sphere);


//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif	// __cplusplus


#endif	// !__ANIMAL3D_RAYPICKING_H