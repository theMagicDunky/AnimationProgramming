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
	
	a3_DemoSceneObject.h
	Example of demo utility header file.
*/

#ifndef __ANIMAL3D_DEMOSCENEOBJECT_H
#define __ANIMAL3D_DEMOSCENEOBJECT_H


// math library
#include "P3DM/P3DM.h"


//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#else	// !__cplusplus
	typedef struct a3_DemoSceneObject	a3_DemoSceneObject;
	typedef struct a3_DemoCamera		a3_DemoCamera;
#endif	// __cplusplus

	
//-----------------------------------------------------------------------------


	// general scene objects
	struct a3_DemoSceneObject
	{
		p3mat4 modelMat;	// model matrix: transform relative to scene
		p3mat4 modelMatInv;	// inverse model matrix: scene relative to this
		p3vec3 euler;		// euler angles for direct rotation control
		p3vec3 position;	// scene position for direct control
	};

	// camera/viewer
	struct a3_DemoCamera
	{
		a3_DemoSceneObject *sceneObject;	// pointer to scene object
		p3mat4 projectionMat;				// projection matrix
		p3mat4 projectionMatInv;			// inverse projection matrix
		p3mat4 viewProjectionMat;			// concatenation of view-projection
		p3real fovy;						// vertical field of view for zoom
		p3real aspect;						// aspect ratio
		p3real znear, zfar;					// near and far clipping planes
		p3real ctrlMoveSpeed;				// how fast controlled camera moves
		p3real ctrlRotateSpeed;				// control rotate speed (degrees)
		p3real ctrlZoomSpeed;				// control zoom speed (degrees)
	};


//-----------------------------------------------------------------------------

	// scene object initializers and updates
	inline void a3demo_initSceneObject(a3_DemoSceneObject *sceneObject);
	inline void a3demo_updateSceneObject(a3_DemoSceneObject *sceneObject);
	inline void a3demo_rotateSceneObject(a3_DemoSceneObject *sceneObject, const p3real speed, const p3real deltaX, const p3real deltaY, const p3real deltaZ);
	inline void a3demo_moveSceneObject(a3_DemoSceneObject *sceneObject, const p3real speed, const p3real deltaX, const p3real deltaY, const p3real deltaZ);
	inline void a3demo_setCameraSceneObject(a3_DemoCamera *camera, a3_DemoSceneObject *sceneObject);
	inline void a3demo_initCamera(a3_DemoCamera *camera);
	inline void a3demo_updateCameraViewProjection(a3_DemoCamera *camera);


//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif	// __cplusplus


#endif	// !__ANIMAL3D_DEMOSCENEOBJECT_H