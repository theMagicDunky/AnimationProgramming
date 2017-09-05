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
	
	drawLine_vs4x.glsl
	Geometry shader that draws a line segment given a set of waypoints and the 
		index of the first in the segment.

	**DO NOT MODIFY THIS FILE**
*/

#version 410

#define MAX_WAYPOINTS 64
#define MAX_SAMPLES 2

layout (points) in;
layout (line_strip, max_vertices = MAX_SAMPLES) out;

in int vInstanceID[];

uniform mat4 uMVP;
uniform vec3 uWaypoints[MAX_WAYPOINTS];

out vec4 vPassColor;


// draw a single line segment
void drawLineSegment(in vec4 p0, in vec4 p1)
{
	gl_Position = uMVP * p0;
	EmitVertex();
	gl_Position = uMVP * p1;
	EmitVertex();
	EndPrimitive();
}


void main()
{
	int instanceID = vInstanceID[0];
	vec4 p0 = vec4(uWaypoints[instanceID], 1.0);
	vec4 p1 = vec4(uWaypoints[instanceID + 1], 1.0);

	// draw line segment
	vPassColor = vec4(0.5, 0.5, 1.0, 1.0);
	drawLineSegment(p0, p1);
}
