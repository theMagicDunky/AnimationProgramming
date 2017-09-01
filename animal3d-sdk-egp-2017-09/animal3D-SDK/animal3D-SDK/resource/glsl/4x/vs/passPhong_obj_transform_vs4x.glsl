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
	
	passPhong_obj_transform_vs4x.glsl
	Transform position attribute, pass UVs and all object-space vectors used  
		in the Phong shading model.

	**DO NOT MODIFY THIS FILE**
*/

#version 410

layout (location = 0) in vec4 aPosition;
layout (location = 2) in vec4 aNormal;
layout (location = 8) in vec4 aTexcoord;

uniform mat4 uMVP;
uniform vec4 uLightPos_obj;
uniform vec4 uEyePos_obj;

out vec2 vPassTexcoord;

out vbLightingData
{
	vec4 position;
	vec4 normal;
	vec4 lightVec;
	vec4 eyeVec;
} vLighting_obj;

void main()
{
	gl_Position = uMVP * aPosition;
	vPassTexcoord = aTexcoord.xy;
	
	vLighting_obj.position = aPosition;
	vLighting_obj.normal = vec4(aNormal.xyz, 0.0);
	vLighting_obj.lightVec = uLightPos_obj - aPosition;
	vLighting_obj.eyeVec = uEyePos_obj - aPosition;
}
