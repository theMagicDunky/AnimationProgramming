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
	
	drawPhong_obj_fs4x.glsl
	Use UVs and lighting vectors from previous stage to calculate the Phong 
		shading model with textures.

	**DO NOT MODIFY THIS FILE**
*/

#version 410

in vec2 vPassTexcoord;

in vbLightingData
{
	vec4 position;
	vec4 normal;
	vec4 lightVec;
	vec4 eyeVec;
} vLighting_obj;

uniform sampler2D uTex_dm;
uniform sampler2D uTex_sm;

out vec4 rtFragColor;

void main()
{
	// texture samples
	vec4 surfaceDiffuse = texture(uTex_dm, vPassTexcoord);
	vec4 surfaceSpecular = texture(uTex_sm, vPassTexcoord);

	// lighting vectors
	vec4 N = normalize(vLighting_obj.normal);
	vec4 L = normalize(vLighting_obj.lightVec);
	vec4 V = normalize(vLighting_obj.eyeVec);

	// diffuse coefficient
	float kd = dot(N, L);

	// reflected light vector
	vec4 R = (kd + kd)*N - L;

	// specular highlight
	float ks = dot(V, R);

	// clamp values
	kd = max(0.0, kd);
	ks = max(0.0, ks);

	// specular exponent
	ks *= ks;
	ks *= ks;
	ks *= ks;
	ks *= ks;

	// basic Phong: diffuse + specular
	rtFragColor = surfaceDiffuse * kd + surfaceSpecular * ks;

	// DEBUGGING: 
	// show every step, one at a time
//	rtFragColor = surfaceDiffuse;
//	rtFragColor = surfaceSpecular;
//	rtFragColor = N * 0.5 + 0.5;
//	rtFragColor = L * 0.5 + 0.5;
//	rtFragColor = V * 0.5 + 0.5;
//	rtFragColor = R * 0.5 + 0.5;
//	rtFragColor = vec4(kd);
//	rtFragColor = vec4(ks);


	// full alpha
	rtFragColor.a = 1.0;
}
