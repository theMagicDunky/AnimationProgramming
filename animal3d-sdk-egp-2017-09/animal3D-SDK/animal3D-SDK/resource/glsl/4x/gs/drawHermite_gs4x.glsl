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
	
	drawHermite_vs4x.glsl
	Geometry shader that draws a Hermite curve segment given a list of 
		waypoints and handles, and the index of the first in the segment.

	**DO NOT MODIFY THIS FILE**
*/

#version 410

#define MAX_WAYPOINTS 64
#define MAX_SAMPLES 48

layout (points) in;
layout (line_strip, max_vertices = MAX_SAMPLES) out;

in int vInstanceID[];

uniform mat4 uMVP;
uniform vec3 uWaypoints[MAX_WAYPOINTS];
uniform vec3 uWaypointHandles[MAX_WAYPOINTS];

out vec4 vPassColor;


// draw line segment
void drawLineSegment(in vec4 p0, in vec4 p1)
{
	gl_Position = uMVP * p0;
	EmitVertex();
	gl_Position = uMVP * p1;
	EmitVertex();
	EndPrimitive();
}


// sample using cubic Hermite algorithm
vec4 sampleCubicHermite(in vec4 p0, in vec4 m0, in vec4 p1, in vec4 m1, in float t)
{
	// column-major Hermite kernel
	const mat4 kernel = mat4(	 1.0,	 0.0,	 0.0,	 0.0, 
								 0.0,	 1.0,	 0.0,	 0.0, 
								-3.0,	-2.0,	 3.0,	-1.0, 
								 2.0,	 1.0,	-2.0,	 1.0);
	mat4 influence = mat4(p0, m0, p1, m1);
	vec4 param = vec4(1.0, t, t*t, t*t*t);

	// Hermite sample
	return (influence * (kernel * param));
}

// draw Hermite curve segment
void drawHermiteSegment(in vec4 p0, in vec4 m0, in vec4 p1, in vec4 m1, in int n)
{
	float dt = 1.0 / float(n);
	float t = dt;
	vec4 pt;
	int i;

	// first sample is p0
	gl_Position = uMVP * p0;
	EmitVertex();

	// curve samples
	for (i = 1; i < n; ++i)
	{
		t = dt * float(i);
		pt = sampleCubicHermite(p0, m0, p1, m1, t);
		gl_Position = uMVP * pt;
		EmitVertex();
	}

	// last sample is p1
	gl_Position = uMVP * p1;
	EmitVertex();

	// done
	EndPrimitive();
}


void main()
{
	int instanceID = vInstanceID[0];
	vec4 p0 = vec4(uWaypoints[instanceID], 1.0);
	vec4 p1 = vec4(uWaypoints[instanceID + 1], 1.0);
	vec4 h0 = vec4(uWaypointHandles[instanceID], 1.0);
	vec4 h1 = vec4(uWaypointHandles[instanceID + 1], 1.0);
	vec4 m0 = h0 - p0;
	vec4 m1 = h1 - p1;

	// draw curve segment
	vPassColor = vec4(0.5, 0.5, 1.0, 1.0);
	drawHermiteSegment(p0, m0, p1, m1, 40);

	// draw lines from points to their handles
	// negate end handle vector for bi-directional
	vPassColor = vec4(0.5, 0.5, 0.0, 1.0);
	drawLineSegment(p0, h0);
	drawLineSegment(p1, p1 - m1);
}
