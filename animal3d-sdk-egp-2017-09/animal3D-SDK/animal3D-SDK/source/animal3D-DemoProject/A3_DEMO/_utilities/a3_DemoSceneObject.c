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
	
	a3_DemoSceneObject.c
	Example of demo utility source file.
*/

#include "a3_DemoSceneObject.h"


//-----------------------------------------------------------------------------

// scene object setup and manipulation
extern inline void a3demo_initSceneObject(a3_DemoSceneObject *sceneObject)
{
	p3real4x4SetIdentity(sceneObject->modelMat.m);
	p3real4x4SetIdentity(sceneObject->modelMatInv.m);
	p3real3Set(sceneObject->euler.v, realZero, realZero, realZero);
	p3real3Set(sceneObject->position.v, realZero, realZero, realZero);
}

extern inline void a3demo_updateSceneObject(a3_DemoSceneObject *sceneObject)
{
	p3real4x4SetRotateZYX(sceneObject->modelMat.m, sceneObject->euler.x, sceneObject->euler.y, sceneObject->euler.z);
	sceneObject->modelMat.v3.xyz = sceneObject->position;
	p3real4x4TransformInverseIgnoreScale(sceneObject->modelMatInv.m, sceneObject->modelMat.m);
}

extern inline void a3demo_rotateSceneObject(a3_DemoSceneObject *sceneObject, const p3real speed, const p3real deltaX, const p3real deltaY, const p3real deltaZ)
{
	// validate angles so they don't get zero'd out (trig functions have a limit)
	sceneObject->euler.x = p3trigValid_sind(sceneObject->euler.x + speed * deltaX);
	sceneObject->euler.y = p3trigValid_sind(sceneObject->euler.y + speed * deltaY);
	sceneObject->euler.z = p3trigValid_sind(sceneObject->euler.z + speed * deltaZ);
}

extern inline void a3demo_moveSceneObject(a3_DemoSceneObject *sceneObject, const p3real speed, const p3real deltaX, const p3real deltaY, const p3real deltaZ)
{
	if (deltaX || deltaY || deltaZ)
	{
		p3real3 delta[3];
		p3real3ProductS(delta[0], sceneObject->modelMat.m[0], deltaX);	// account for orientation of object
		p3real3ProductS(delta[1], sceneObject->modelMat.m[1], deltaY);
		p3real3ProductS(delta[2], sceneObject->modelMat.m[2], deltaZ);
		p3real3Add(delta[0], delta[1]);									// add the 3 deltas together
		p3real3Add(delta[0], delta[2]);
		p3real3MulS(delta[0], speed * p3real3LengthInverse(delta[0]));	// normalize and scale by speed
		p3real3Add(sceneObject->position.v, delta[0]);					// add delta to current
	}
}


extern inline void a3demo_setCameraSceneObject(a3_DemoCamera *camera, a3_DemoSceneObject *sceneObject)
{
	camera->sceneObject = sceneObject;
}

extern inline void a3demo_initCamera(a3_DemoCamera *camera)
{
	p3real4x4SetIdentity(camera->projectionMat.m);
	p3real4x4SetIdentity(camera->projectionMatInv.m);
	p3real4x4SetReal4x4(camera->viewProjectionMat.m, camera->sceneObject->modelMatInv.m);
	camera->fovy = realNinety;
	camera->aspect = realOne;
	camera->znear = -realOne;
	camera->zfar = realOne;
	camera->ctrlMoveSpeed = realOne;
	camera->ctrlRotateSpeed = realOne;
	camera->ctrlZoomSpeed = realZero;
}

extern inline void a3demo_updateCameraViewProjection(a3_DemoCamera *camera)
{
	p3real4x4Product(camera->viewProjectionMat.m, camera->projectionMat.m, camera->sceneObject->modelMatInv.m);
}


//-----------------------------------------------------------------------------
