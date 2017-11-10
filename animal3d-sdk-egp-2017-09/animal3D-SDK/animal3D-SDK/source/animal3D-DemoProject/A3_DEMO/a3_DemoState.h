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
	
	a3_DemoState.h
	Demo state interface and programmer function declarations.

	********************************************
	*** THIS IS YOUR DEMO'S MAIN HEADER FILE ***
	********************************************
*/

#ifndef __ANIMAL3D_DEMOSTATE_H
#define __ANIMAL3D_DEMOSTATE_H


//-----------------------------------------------------------------------------
// animal3D framework includes

#include "animal3D/animal3D.h"


//-----------------------------------------------------------------------------
// other demo includes

#include "_utilities/a3_DemoSceneObject.h"
#include "_utilities/a3_RayPicking.h"
#include "_utilities/a3_Quaternion.h"
#include "_utilities/a3_HierarchyState.h"
#include "_utilities/a3_Kinematics.h"


//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#else	// !__cplusplus
	typedef struct a3_DemoState					a3_DemoState;
	typedef struct a3_DemoStateShaderProgram	a3_DemoStateShaderProgram;
#endif	// __cplusplus


//-----------------------------------------------------------------------------

	// object maximum counts for easy array storage
	// good idea to make these numbers greater than what you actually need 
	//	and if you end up needing more just increase the count... there's 
	//	more than enough memory to hold extra objects
	enum a3_DemoStateObjectMaxCounts
	{
		demoStateMaxCount_sceneObject = 4,
		demoStateMaxCount_camera = 1,
		demoStateMaxCount_timer = 1,
		demoStateMaxCount_texture = 4,
		demoStateMaxCount_drawDataBuffer = 1,
		demoStateMaxCount_vertexArray = 4,
		demoStateMaxCount_drawable = 16,
		demoStateMaxCount_shaderProgram = 8,
		demoStateMaxCount_shaderProgramUniform = 16,
	};


//-----------------------------------------------------------------------------

	// structure to help with shader program and uniform management
	struct a3_DemoStateShaderProgram
	{
		a3_ShaderProgram program[1];
		union {
			int uniformLocation[demoStateMaxCount_shaderProgramUniform];
			struct {
				int
					// common vertex shader uniforms
					uMVP,						// model-view-projection transform
					uLocal,						// local-space transform (pre-model)
					uLightPos_obj,				// light position in object-space
					uEyePos_obj,				// eye position in object-space

					// geometry shader uniforms
					uWaypoints,					// all waypoint data for curve drawing
					uWaypointHandles,			// all waypoint handle data for Hermite curves
					uWaypointCount,				// number of path waypoints
					uWaypointIndex,				// index of current path waypoint
					uLineColor,					// line segment color
					uCurveColor,				// curve segment color

					// common fragment shader uniforms
					uColor,						// uniform color
					uTex_dm,					// diffuse texture sampler
					uTex_sm;					// specular texture sampler
			};
		};
	};



//-----------------------------------------------------------------------------

	// persistent demo state data structure
	struct a3_DemoState
	{
		//---------------------------------------------------------------------
		// general variables pertinent to the state

		// terminate key pressed
		int exitFlag;

		// global vertical axis: Z = 0, Y = 1
		int verticalAxis;

		// asset streaming between loads enabled (careful!)
		int streaming;

		// window and full-frame dimensions
		unsigned int windowWidth, windowHeight;
		unsigned int frameWidth, frameHeight;
		int frameBorder;


		//---------------------------------------------------------------------
		// objects that have known or fixed instance count in the whole demo

		// text renderer
		int textInit, showText;
		a3_TextRenderer text[1];

		// input
		a3_MouseInput mouse[1];
		a3_KeyboardInput keyboard[1];
		a3_XboxControllerInput xcontrol[4];

		// pointer to fast trig table
		float trigTable[4096 * 4];


		//---------------------------------------------------------------------
		// animation variables and objects

		// update and display modes
		int animationMode, animationModeCount;
		int displayBoneAxes, displayBoneNames;

		// whole object poses (could be a resource)
		a3_HierarchyNodePose objectPoses[2];

		// whole object pose blend result (not a resource)
		a3_HierarchyNodePose objectPoseState_blend[1];

		// skeleton hierarchy (resource)
		a3_Hierarchy skeleton[1];

		// pose set for skeleton (resource)
		a3_HierarchyPoseGroup skeletonPoses[1];

		// pose container for blending (not a resource)
		a3_HierarchyPoseGroup skeletonPoses_blend[1];

		// hierarchy states for different modes (not a resource)
		a3_HierarchyState skeletonState_blend[1];

		// blend control parameter
		float blendAlpha, targetBlendAlpha, targetBlendAlphaSmoothing;

		// pose-to-pose controller
		unsigned int currentKeyPoseIndex, nextKeyPoseIndex;
		float poseTime, poseDuration;


		//---------------------------------------------------------------------
		// object arrays: organized as anonymous unions for two reasons: 
		//	1. easy to manage entire sets of the same type of object using the 
		//		array component
		//	2. at the same time, variables are named pointers

		// scene objects
		union {
			a3_DemoSceneObject sceneObject[demoStateMaxCount_sceneObject];
			struct {
				a3_DemoSceneObject
					cameraObject[1],					// transform for camera
					lightObject[1],						// transform for light
					blendingObject[1],					// pose blending object
					skeletonObject[1];					// skeletal animation object
			};
		};

		// cameras
		union {
			a3_DemoCamera camera[demoStateMaxCount_camera];
			struct {
				a3_DemoCamera
					sceneCamera[1];						// scene viewing camera
			};
		};

		// timers
		union {
			a3_Timer timer[demoStateMaxCount_timer];
			struct {
				a3_Timer
					renderTimer[1];						// render FPS timer
			};
		};


		// textures
		union {
			a3_Texture texture[demoStateMaxCount_texture];
			struct {
				a3_Texture
					tex_checker[1],						// checkered texture
					tex_earth_dm[1],					// earth diffuse texture
					tex_earth_sm[1];					// earth specular texture
			};
		};


		// draw data buffers
		union {
			a3_VertexBuffer drawDataBuffer[demoStateMaxCount_drawDataBuffer];
			struct {
				a3_VertexBuffer
					vbo_staticSceneObjectDrawBuffer[1];			// buffer to hold all data for static scene objects (e.g. grid)
			};
		};

		// vertex array objects
		union {
			a3_VertexArrayDescriptor vertexArray[demoStateMaxCount_vertexArray];
			struct {
				a3_VertexArrayDescriptor
					vao_position[1],							// VAO for vertex format with only position
					vao_position_color[1],						// VAO for vertex format with position and color
					vao_position_texcoords[1],					// VAO for vertex format with position and UVs
					vao_tangentBasis[1];						// VAO for vertex format with full tangent basis, inclusing position and UVs
			};
		};

		// drawables
		union {
			a3_VertexDrawable drawable[demoStateMaxCount_drawable];
			struct {
				a3_VertexDrawable
					draw_grid[1],								// wireframe ground plane to emphasize scaling
					draw_axes[1],								// coordinate axes at the center of the world
					draw_curve[1],								// single-vertex shape used for drawing curve segments
					draw_node[1],								// small round shape for a node or waypoint
					draw_gimbal[1],								// gimbal background
					draw_ring[1],								// gimbal ring (torus or ring)
					draw_bone[1],								// skeletal bone shape (spike)
					draw_joint[1],								// joint shape
					draw_sphere[1],								// procedural sphere
					draw_teapot[1];								// loaded teapot model
			};
		};


		// shader programs and uniforms
		union {
			a3_DemoStateShaderProgram shaderProgram[demoStateMaxCount_shaderProgram];
			struct {
				a3_DemoStateShaderProgram
					prog_drawColor[1],					// draw color attribute
					prog_drawColorInstanced[1],			// draw color attribute instanced
					prog_drawColorUnif[1],				// draw uniform color
					prog_drawColorUnifInstanced[1],		// draw uniform color instanced
					prog_drawTexture[1],				// draw texture sample
					prog_drawPhong_obj[1],				// draw object-space Phong shading
					prog_drawLine[1],					// draw line segment
					prog_drawCurve[1];					// draw curve segment
			};
		};


		//---------------------------------------------------------------------
	};

	
//-----------------------------------------------------------------------------

	// demo-related functions

	void a3demo_setDefaultGraphicsState();

	void a3demo_loadTextures(a3_DemoState *demoState);
	void a3demo_loadGeometry(a3_DemoState *demoState);
	void a3demo_loadShaders(a3_DemoState *demoState);

	void a3demo_unloadTextures(a3_DemoState *demoState);
	void a3demo_unloadGeometry(a3_DemoState *demoState);
	void a3demo_unloadShaders(a3_DemoState *demoState);

	void a3demo_loadAnimation(a3_DemoState *demoState);
	void a3demo_unloadAnimation(a3_DemoState *demoState);

	void a3demo_initScene(a3_DemoState *demoState);

	void a3demo_validateUnload(const a3_DemoState *demoState);

	void a3demo_refresh(a3_DemoState *demoState);
	void a3demo_input(a3_DemoState *demoState, double dt);
	void a3demo_update(a3_DemoState *demoState, double dt);
	void a3demo_render(const a3_DemoState *demoState);


//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif	// __cplusplus


#endif	// !__ANIMAL3D_DEMOSTATE_H