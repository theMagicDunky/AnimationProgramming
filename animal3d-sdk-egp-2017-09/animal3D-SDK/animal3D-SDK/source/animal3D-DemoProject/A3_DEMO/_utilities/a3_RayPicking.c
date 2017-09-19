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
	
	a3_RayPicking.c
	Ray casting and picking implementations.
*/

#include "a3_RayPicking.h"


//-----------------------------------------------------------------------------

// create ray given start and end points
extern inline int a3rayCreate(a3_Ray *ray_out, const p3vec3 origin, const p3vec3 end)
{
	if (ray_out)
	{
		// set origin
		ray_out->origin.xyz = origin;
		ray_out->origin.w = realOne;

		// set direction
		p3real3Diff(ray_out->direction.v, end.v, origin.v);
		p3real3Normalize(ray_out->direction.v);
		ray_out->direction.w = realZero;

		// done
		return 1;
	}
	return 0;
}

// create ray given NDC coordinate and an inverse projection matrix
extern inline int a3rayCreateUnprojected(a3_Ray *ray_out, const p3vec3 v_ndc, const p3mat4 invProj)
{
	if (ray_out)
	{
		// set origin at zero vector
		ray_out->origin = p3wVec4;

		// calculate direction
		//	(inverse projection method: reverse perspective divide)
		ray_out->direction.xyz = v_ndc;
		ray_out->direction.w = realOne;
		p3real4Real4x4Mul(invProj.m, ray_out->direction.v);
		p3real3DivS(ray_out->direction.v, ray_out->direction.w);
		p3real3Normalize(ray_out->direction.v);
		ray_out->direction.w = realZero;

		// done
		return 1;
	}
	return 0;
}

// pick against shapes
extern inline int a3rayTestSphere(a3_RayHit *hit_out, const a3_Ray *ray, const a3_Sphere *sphere)
{
	if (hit_out && ray && sphere)
	{
		// ray vs sphere test: 
		// good resource: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
		// good resource: http://antongerdelan.net/opengl/raycasting.html
		//	- calculate difference from ray origin to sphere center: L
		//	- project difference vector onto ray direction vector (unit)
		//		- shortcut: if dot product is negative, ray originates 
		//			within or ahead of sphere; test fails: d = L dot D
		//			(if the ray direction vector is unit length, the dot 
		//			product represents the length of the projected vector)
		//	- calculate distance from center of sphere to projected point: h
		//		- Pythagorean theorem:	d^2 + h^2 = L^2
		//								h^2 = L^2 - d^2
		//	- check if intersection occurred: 
		//		- Pythagorean again:	h^2 + b^2 = r^2
		//								b^2 = r^2 - h^2
		p3vec3 originToCenter;
		p3real passingDist;

		p3real3Diff(originToCenter.v, sphere->center.v, ray->origin.v);
		passingDist = p3real3Dot(originToCenter.v, ray->direction.v);
		if (passingDist >= realZero)
		{
			// "passing point" is in front of ray origin
			// get distance from "passing point" to center of sphere
			const p3real h_sq = p3real3LengthSquared(originToCenter.v) - (passingDist * passingDist);

			// test if intersection
			const p3real r_sq = sphere->radius * sphere->radius;

			if (r_sq >= h_sq)
			{
				// passing point is within sphere: ray collides
				// calculate offset from passing point to intersections
				const p3real b = (p3real)p3sqrt((p3f64)(r_sq - h_sq));

				hit_out->param0 = passingDist - b;
				hit_out->param1 = passingDist + b;
				
				p3real4ProductS(hit_out->hit0.v, ray->direction.v, hit_out->param0);
				p3real4ProductS(hit_out->hit1.v, ray->direction.v, hit_out->param1);
				p3real4Add(hit_out->hit0.v, ray->origin.v);
				p3real4Add(hit_out->hit1.v, ray->origin.v);

				// done
				return 1;
			}
		}

		// bad result
		hit_out->hit0 = hit_out->hit1 = p3zeroVec4;
		hit_out->param0 = hit_out->param1 = realZero;
	}
	return 0;
}


//-----------------------------------------------------------------------------
