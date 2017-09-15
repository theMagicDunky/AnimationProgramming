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
	
	a3_DemoState.c/.cpp
	Demo state function implementations.

	********************************************
	*** THIS IS YOUR DEMO'S MAIN SOURCE FILE ***
	*** Implement your demo logic here.      ***
	********************************************
*/


#include "a3_DemoState.h"


//-----------------------------------------------------------------------------

// OpenGL
#ifdef _WIN32
#include <Windows.h>
#include <GL/GL.h>
#else	// !_WIN32
#include <OpenGL/gl3.h>
#endif	// _WIN32


#include <stdio.h>
#include <stdlib.h>


//-----------------------------------------------------------------------------
// SETUP AND TERMINATION UTILITIES

// set default GL state
void a3demo_setDefaultGraphicsState()
{
	const float lineWidth = 2.0f;
	const float pointSize = 4.0f;

	// lines and points
	glLineWidth(lineWidth);
	glPointSize(pointSize);

	// backface culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// alpha blending
	// result = ( new*[new alpha] ) + ( old*[1 - new alpha] )
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// textures
	glEnable(GL_TEXTURE_2D);

	// background
	glClearColor(0.0f, 0.0f, 0.0, 0.0f);
}


//-----------------------------------------------------------------------------

// utility to load textures
void a3demo_loadTextures(a3_DemoState *demoState)
{
	// pointer to texture
	a3_Texture *tex;
	unsigned int i;

	// list of texture files to load
	const char *texFiles[] = {
		"../../../../resource/tex/sprite/checker.png",
		"../../../../resource/tex/earth/2k/earth_dm_2k.png",
		"../../../../resource/tex/earth/2k/earth_sm_2k.png",
	};
	const unsigned int numTextures = sizeof(texFiles) / sizeof(const char *);

	for (i = 0; i < numTextures; ++i)
	{
		tex = demoState->texture + i;
		a3textureCreateFromFile(tex, texFiles[i]);
	}

	// change settings on a per-texture basis
	a3textureActivate(demoState->tex_checker, a3tex_unit00);
	a3textureDefaultSettings();	// nearest filtering, repeat on both axes

	a3textureActivate(demoState->tex_earth_dm, a3tex_unit00);
	a3textureChangeRepeatMode(a3tex_repeatNormal, a3tex_repeatClamp);	// repeat on U, clamp V
	a3textureChangeFilterMode(a3tex_filterLinear);	// linear pixel blending

	a3textureActivate(demoState->tex_earth_sm, a3tex_unit00);
	a3textureChangeRepeatMode(a3tex_repeatNormal, a3tex_repeatClamp);
	a3textureChangeFilterMode(a3tex_filterLinear);


	// done
	a3textureDeactivate(a3tex_unit00);
}

// utility to load geometry
void a3demo_loadGeometry(a3_DemoState *demoState)
{
	// static model transformations
	static const p3mat4 downscale100x = {
		+0.01f, 0.0f, 0.0f, 0.0f,
		0.0f, +0.01f, 0.0f, 0.0f,
		0.0f, 0.0f, +0.01f, 0.0f,
		0.0f, 0.0f, 0.0f, +1.0f,
	};

	// pointer to shared vbo/ibo
	a3_VertexBuffer *vbo_ibo;
	a3_VertexArrayDescriptor *vao;
	a3_VertexDrawable *currentDrawable;
	unsigned int sharedVertexStorage = 0, sharedIndexStorage = 0;
	unsigned int numVerts = 0;
	unsigned int i;


	// file streaming (if requested)
	a3_FileStream fileStream[1] = { 0 };
	const char *const geometryStream = "./data/geom_waypoints.dat";

	// geometry data
	a3_GeometryData sceneShapesData[2] = { 0 };
	a3_GeometryData overlayShapesData[2] = { 0 };
	a3_GeometryData proceduralShapesData[1] = { 0 };
	a3_GeometryData loadedModelsData[1] = { 0 };
	const unsigned int sceneShapesCount = 2;
	const unsigned int overlayShapesCount = 2;
	const unsigned int proceduralShapesCount = 1;
	const unsigned int loadedModelsCount = 1;

	// common index format
	a3_IndexFormatDescriptor sceneCommonIndexFormat[1] = { 0 };


	// procedural scene objects
	// attempt to load stream if requested
	if (demoState->streaming && a3fileStreamOpenRead(fileStream, geometryStream))
	{
		// read from stream

		// static scene objects
		for (i = 0; i < sceneShapesCount; ++i)
			a3fileStreamReadObject(fileStream, sceneShapesData + i, (a3_FileStreamReadFunc)a3geometryLoadDataBinary);

		// overlay scene objects
		for (i = 0; i < overlayShapesCount; ++i)
			a3fileStreamReadObject(fileStream, overlayShapesData + i, (a3_FileStreamReadFunc)a3geometryLoadDataBinary);

		// procedural models
		for (i = 0; i < proceduralShapesCount; ++i)
			a3fileStreamReadObject(fileStream, proceduralShapesData + i, (a3_FileStreamReadFunc)a3geometryLoadDataBinary);

		// loaded models
		for (i = 0; i < loadedModelsCount; ++i)
			a3fileStreamReadObject(fileStream, loadedModelsData + i, (a3_FileStreamReadFunc)a3geometryLoadDataBinary);

		// done
		a3fileStreamClose(fileStream);
	}
	// not streaming or stream doesn't exist
	else if (!demoState->streaming || a3fileStreamOpenWrite(fileStream, geometryStream))
	{
		// create new data
		a3_ProceduralGeometryDescriptor sceneShapes[2] = { a3geomShape_none };
		a3_ProceduralGeometryDescriptor overlayShapes[1] = { a3geomShape_none };
		a3_ProceduralGeometryDescriptor proceduralShapes[1] = { a3geomShape_none };

		// static scene procedural objects
		a3proceduralCreateDescriptorAxes(sceneShapes + 0, a3geomFlag_wireframe, 0.0f, 1);
		a3proceduralCreateDescriptorPlane(sceneShapes + 1, a3geomFlag_wireframe, a3geomAxis_default, 32.0f, 32.0f, 32, 32);
		for (i = 0; i < sceneShapesCount; ++i)
		{
			a3proceduralGenerateGeometryData(sceneShapesData + i, sceneShapes + i);
			a3fileStreamWriteObject(fileStream, sceneShapesData + i, (a3_FileStreamWriteFunc)a3geometrySaveDataBinary);
		}

		// static overlay objects
		// first, "node" or "waypoint" object, just a low-resolution sphere
		a3proceduralCreateDescriptorSphere(overlayShapes + 0, a3geomFlag_vanilla, a3geomAxis_default, 0.1f, 8, 4);
		a3proceduralGenerateGeometryData(overlayShapesData + 0, overlayShapes + 0);
		a3fileStreamWriteObject(fileStream, overlayShapesData + 0, (a3_FileStreamWriteFunc)a3geometrySaveDataBinary);

		// single-vertex shape for curve segment drawing
		{
			// create vertex format
			a3_GeometryData *pointData = overlayShapesData + 1;
			a3geometryCreateVertexFormat(pointData->vertexFormat, 0, 0);
			pointData->primType = a3prim_points;
			pointData->numVertices = 1;
			pointData->attribData[0] = pointData->data = malloc(3 * sizeof(float));
			memset(pointData->data, 0, 3 * sizeof(float));
			a3fileStreamWriteObject(fileStream, pointData, (a3_FileStreamWriteFunc)a3geometrySaveDataBinary);
		}


		// procedural
		a3proceduralCreateDescriptorSphere(proceduralShapes + 0, a3geomFlag_tangents, a3geomAxis_default, 1.0f, 24, 16);
		for (i = 0; i < proceduralShapesCount; ++i)
		{
			a3proceduralGenerateGeometryData(proceduralShapesData + i, proceduralShapes + i);
			a3fileStreamWriteObject(fileStream, proceduralShapesData + i, (a3_FileStreamWriteFunc)a3geometrySaveDataBinary);
		}

		// loaded models
		a3modelLoadOBJ(loadedModelsData + 0, "../../../../resource/obj/teapot/teapot.obj", a3model_calculateVertexTangents, downscale100x.mm);
		for (i = 0; i < loadedModelsCount; ++i)
			a3fileStreamWriteObject(fileStream, loadedModelsData + i, (a3_FileStreamWriteFunc)a3geometrySaveDataBinary);

		// done
		a3fileStreamClose(fileStream);
	}


	// GPU data upload process: 
	//	- determine storage requirements
	//	- allocate buffer
	//	- create vertex arrays using unique formats
	//	- create drawable and upload data

	// get storage size
	sharedVertexStorage = numVerts = 0;
	for (i = 0; i < sceneShapesCount; ++i)
	{
		sharedVertexStorage += a3geometryGetVertexBufferSize(sceneShapesData + i);
		numVerts += sceneShapesData[i].numVertices;
	}
	for (i = 0; i < overlayShapesCount; ++i)
	{
		sharedVertexStorage += a3geometryGetVertexBufferSize(overlayShapesData + i);
		numVerts += overlayShapesData[i].numVertices;
	}
	for (i = 0; i < proceduralShapesCount; ++i)
	{
		sharedVertexStorage += a3geometryGetVertexBufferSize(proceduralShapesData + i);
		numVerts += proceduralShapesData[i].numVertices;
	}
	for (i = 0; i < loadedModelsCount; ++i)
	{
		sharedVertexStorage += a3geometryGetVertexBufferSize(loadedModelsData + i);
		numVerts += loadedModelsData[i].numVertices;
	}

	// common index format required for shapes that share vertex formats
	a3geometryCreateIndexFormat(sceneCommonIndexFormat, numVerts);
	sharedIndexStorage = 0;
	for (i = 0; i < sceneShapesCount; ++i)
		sharedIndexStorage += a3indexStorageSpaceRequired(sceneCommonIndexFormat, sceneShapesData[i].numIndices);
	for (i = 0; i < overlayShapesCount; ++i)
		sharedIndexStorage += a3indexStorageSpaceRequired(sceneCommonIndexFormat, overlayShapesData[i].numIndices);
	for (i = 0; i < proceduralShapesCount; ++i)
		sharedIndexStorage += a3indexStorageSpaceRequired(sceneCommonIndexFormat, proceduralShapesData[i].numIndices);
	for (i = 0; i < loadedModelsCount; ++i)
		sharedIndexStorage += a3indexStorageSpaceRequired(sceneCommonIndexFormat, loadedModelsData[i].numIndices);


	// create shared buffer
	vbo_ibo = demoState->vbo_staticSceneObjectDrawBuffer;
	a3bufferCreateSplit(vbo_ibo, a3buffer_vertex, sharedVertexStorage, sharedIndexStorage, 0, 0);
	sharedVertexStorage = 0;


	// create vertex formats and drawables
	// axes
	vao = demoState->vao_position_color;
	a3geometryGenerateVertexArray(vao, sceneShapesData + 0, vbo_ibo, sharedVertexStorage);
	currentDrawable = demoState->draw_axes;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, sceneShapesData + 0, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);

	// grid: position attribute only
	// overlay objects are also just position
	vao = demoState->vao_position;
	a3geometryGenerateVertexArray(vao, sceneShapesData + 1, vbo_ibo, sharedVertexStorage);
	currentDrawable = demoState->draw_grid;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, sceneShapesData + 1, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);
	currentDrawable = demoState->draw_node;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, overlayShapesData + 0, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);
	currentDrawable = demoState->draw_curve;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, overlayShapesData + 1, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);

	// models: procedural and loaded have the same format, so they can share 
	//	a vertex format
	vao = demoState->vao_tangentBasis;
	a3geometryGenerateVertexArray(vao, proceduralShapesData + 0, vbo_ibo, sharedVertexStorage);
	currentDrawable = demoState->draw_sphere;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, proceduralShapesData + 0, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);
	currentDrawable = demoState->draw_teapot;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, loadedModelsData + 0, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);

	// release data when done
	for (i = 0; i < sceneShapesCount; ++i)
		a3geometryReleaseData(sceneShapesData + i);
	for (i = 0; i < overlayShapesCount; ++i)
		a3geometryReleaseData(overlayShapesData + i);
	for (i = 0; i < proceduralShapesCount; ++i)
		a3geometryReleaseData(proceduralShapesData + i);
	for (i = 0; i < loadedModelsCount; ++i)
		a3geometryReleaseData(loadedModelsData + i);
}


// utility to load shaders
void a3demo_loadShaders(a3_DemoState *demoState)
{
	// direct to demo programs
	a3_DemoStateShaderProgram *currentDemoProg;
	int *currentUnif, uLocation;
	unsigned int i, j;

	// list of uniform names: align with uniform list in demo struct!
	const char *uniformNames[demoStateMaxCount_shaderProgramUniform] = {
		// common vertex
		"uMVP",
		"uLightPos_obj",
		"uEyePos_obj",

		// common geometry
		"uWaypoints",
		"uWaypointHandles",
		"uWaypointCount",
		"uWaypointIndex",

		// common fragment
		"uColor",
		"uTex_dm",
		"uTex_sm",
	};


	// some default uniform values
	const float defaultColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const int defaultTexUnits[] = { 0, 1 };


	// list of all unique shaders
	// this is a good idea to avoid multi-loading 
	//	those that are shared between programs
	union {
		struct {
			// vertex shaders
			a3_Shader passInstanceID_vs[1];
			a3_Shader passthru_transform_vs[1];
			a3_Shader passColor_transform_vs[1];
			a3_Shader passTexcoord_transform_vs[1];
			a3_Shader passPhong_obj_transform_vs[1];

			// geometry shaders
			a3_Shader drawLine_gs[1];
			a3_Shader drawHermite_gs[1];

			// fragment shaders
			a3_Shader drawColorUnif_fs[1];
			a3_Shader drawColorAttrib_fs[1];
			a3_Shader drawTexture_fs[1];
			a3_Shader drawPhong_obj_fs[1];
		};
	} shaderList = { 0 };
	a3_Shader *const shaderListPtr = (a3_Shader *)(&shaderList);
	const unsigned int numUniqueShaders = sizeof(shaderList) / sizeof(a3_Shader);

	// descriptors to help load shaders; aligned with above list
	struct {
		a3_ShaderType shaderType;
		const char *filePath;
	} shaderDescriptor[] = {
		{ a3shader_vertex,		"../../../../resource/glsl/4x/vs/passInstanceID_vs4x.glsl" },
		{ a3shader_vertex,		"../../../../resource/glsl/4x/vs/passthru_transform_vs4x.glsl" },
		{ a3shader_vertex,		"../../../../resource/glsl/4x/vs/passColor_transform_vs4x.glsl" },
		{ a3shader_vertex,		"../../../../resource/glsl/4x/vs/passTexcoord_transform_vs4x.glsl" },
		{ a3shader_vertex,		"../../../../resource/glsl/4x/vs/passPhong_obj_transform_vs4x.glsl" },

		{ a3shader_geometry,	"../../../../resource/glsl/4x/gs/drawLine_gs4x.glsl" },
		{ a3shader_geometry,	"../../../../resource/glsl/4x/gs/drawHermite_gs4x.glsl" },

		{ a3shader_fragment,	"../../../../resource/glsl/4x/fs/drawColorUnif_fs4x.glsl" },
		{ a3shader_fragment,	"../../../../resource/glsl/4x/fs/drawColorAttrib_fs4x.glsl" },
		{ a3shader_fragment,	"../../../../resource/glsl/4x/fs/drawTexture_fs4x.glsl" },
		{ a3shader_fragment,	"../../../../resource/glsl/4x/fs/drawPhong_obj_fs4x.glsl" },
	};

	// loading files and shaders
	a3_Stream shaderFile[1] = { 0 };

	// load unique shaders: 
	//	- load file contents
	//	- create and compile shader object
	//	- release file contents
	for (i = 0; i < numUniqueShaders; ++i)
	{
		a3streamLoadContents(shaderFile, shaderDescriptor[i].filePath);
		a3shaderCreateFromSource(shaderListPtr + i, shaderDescriptor[i].shaderType, shaderFile->contents);
		a3streamReleaseContents(shaderFile);
	}


	// setup programs: 
	//	- create program object
	//	- attach shader objects

	// uniform color program
	currentDemoProg = demoState->prog_drawColorUnif;
	a3shaderProgramCreate(currentDemoProg->program);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.passthru_transform_vs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawColorUnif_fs);

	// color attrib program
	currentDemoProg = demoState->prog_drawColor;
	a3shaderProgramCreate(currentDemoProg->program);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.passColor_transform_vs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawColorAttrib_fs);

	// texturing program
	currentDemoProg = demoState->prog_drawTexture;
	a3shaderProgramCreate(currentDemoProg->program);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.passTexcoord_transform_vs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawTexture_fs);

	// lighting program
	currentDemoProg = demoState->prog_drawPhong_obj;
	a3shaderProgramCreate(currentDemoProg->program);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.passPhong_obj_transform_vs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawPhong_obj_fs);

	// draw line program
	currentDemoProg = demoState->prog_drawLine;
	a3shaderProgramCreate(currentDemoProg->program);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.passInstanceID_vs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawLine_gs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawColorAttrib_fs);

	// draw curve program
	currentDemoProg = demoState->prog_drawCurve;
	a3shaderProgramCreate(currentDemoProg->program);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.passInstanceID_vs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawHermite_gs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawColorAttrib_fs);


	// activate a primitive for validation
	// makes sure the specified geometry can draw using programs
	// good idea to activate the drawable with the most attributes
	a3vertexActivateDrawable(demoState->draw_axes);

	// link and validate all programs
	for (i = 0; i < demoStateMaxCount_shaderProgram; ++i)
	{
		currentDemoProg = demoState->shaderProgram + i;
		a3shaderProgramLink(currentDemoProg->program);
		a3shaderProgramValidate(currentDemoProg->program);
	}

	// if linking fails, contingency plan goes here
	// otherwise, release shaders
	for (i = 0; i < numUniqueShaders; ++i)
		a3shaderRelease(shaderListPtr + i);


	// prepare uniforms algorithmically instead of manually for all programs
	for (i = 0; i < demoStateMaxCount_shaderProgram; ++i)
	{
		// activate program
		currentDemoProg = demoState->shaderProgram + i;
		a3shaderProgramActivate(currentDemoProg->program);

		// get uniform locations
		currentUnif = currentDemoProg->uniformLocation;
		for (j = 0; j < demoStateMaxCount_shaderProgramUniform; ++j)
			currentUnif[j] = a3shaderUniformGetLocation(currentDemoProg->program, uniformNames[j]);

		// set default values for all programs that have a uniform that will 
		//	either never change or is consistent for all programs
		if ((uLocation = currentDemoProg->uMVP) >= 0)
			a3shaderUniformSendFloatMat(a3unif_mat4, 0, uLocation, 1, p3identityMat4.mm);
		if ((uLocation = currentDemoProg->uLightPos_obj) >= 0)
			a3shaderUniformSendFloat(a3unif_vec4, uLocation, 1, p3wVec4.v);
		if ((uLocation = currentDemoProg->uEyePos_obj) >= 0)
			a3shaderUniformSendFloat(a3unif_vec4, uLocation, 1, p3wVec4.v);
		if ((uLocation = currentDemoProg->uColor) >= 0)
			a3shaderUniformSendFloat(a3unif_vec4, uLocation, 1, defaultColor);
		if ((uLocation = currentDemoProg->uTex_dm) >= 0)
			a3shaderUniformSendInt(a3unif_single, uLocation, 1, defaultTexUnits + 0);
		if ((uLocation = currentDemoProg->uTex_sm) >= 0)
			a3shaderUniformSendInt(a3unif_single, uLocation, 1, defaultTexUnits + 1);
	}

	//done
	a3shaderProgramDeactivate();
	a3vertexDeactivateDrawable();
}


//-----------------------------------------------------------------------------
// release objects
// this is where the union array style comes in handy; don't need a single 
//	release statement for each and every object... just iterate and release!

// utility to unload textures
void a3demo_unloadTextures(a3_DemoState *demoState)
{
	a3_Texture *currentTex = demoState->texture,
		*const endTex = currentTex + demoStateMaxCount_texture;

	while (currentTex < endTex)
		a3textureRelease(currentTex++);
}

// utility to unload geometry
void a3demo_unloadGeometry(a3_DemoState *demoState)
{
	a3_BufferObject *currentBuff = demoState->drawDataBuffer,
		*const endBuff = currentBuff + demoStateMaxCount_drawDataBuffer;
	a3_VertexArrayDescriptor *currentVAO = demoState->vertexArray,
		*const endVAO = currentVAO + demoStateMaxCount_vertexArray;
	a3_VertexDrawable *currentDraw = demoState->drawable,
		*const endDraw = currentDraw + demoStateMaxCount_drawable;

	while (currentBuff < endBuff)
		a3bufferRelease(currentBuff++);
	while (currentVAO < endVAO)
		a3vertexArrayReleaseDescriptor(currentVAO++);
	while (currentDraw < endDraw)
		a3vertexReleaseDrawable(currentDraw++);
}


// utility to unload shaders
void a3demo_unloadShaders(a3_DemoState *demoState)
{
	a3_DemoStateShaderProgram *currentProg = demoState->shaderProgram,
		*const endProg = currentProg + demoStateMaxCount_shaderProgram;

	while (currentProg < endProg)
		a3shaderProgramRelease((currentProg++)->program);
}


//-----------------------------------------------------------------------------

// initialize non-asset objects
void a3demo_initScene(a3_DemoState *demoState)
{
	unsigned int i;
	const float cameraAxisPos = 20.0f;

	// all objects
	for (i = 0; i < demoStateMaxCount_sceneObject; ++i)
		a3demo_initSceneObject(demoState->sceneObject + i);

	a3demo_setCameraSceneObject(demoState->sceneCamera, demoState->cameraObject);
	for (i = 0; i < demoStateMaxCount_camera; ++i)
		a3demo_initCamera(demoState->sceneCamera + i);

	// cameras
	demoState->sceneCamera->znear = 1.00f;
	demoState->sceneCamera->zfar = 100.0f;
	demoState->sceneCamera->ctrlMoveSpeed = 10.0f;
	demoState->sceneCamera->ctrlRotateSpeed = 5.0f;
	demoState->sceneCamera->ctrlZoomSpeed = 5.0f;

/*
	// camera's starting orientation depends on "vertical" axis
	// we want the exact same view in either case
	if (demoState->verticalAxis)
	{
		// vertical axis is Y
		demoState->sceneCamera->sceneObject->position.x = +cameraAxisPos;
		demoState->sceneCamera->sceneObject->position.y = +cameraAxisPos;
		demoState->sceneCamera->sceneObject->position.z = -cameraAxisPos;
		demoState->sceneCamera->sceneObject->euler.x = -30.0f;
		demoState->sceneCamera->sceneObject->euler.y = 135.0f;
		demoState->sceneCamera->sceneObject->euler.z = 0.0f;
	}
	else
	{
		// vertical axis is Z
		demoState->sceneCamera->sceneObject->position.x = +cameraAxisPos;
		demoState->sceneCamera->sceneObject->position.y = +cameraAxisPos;
		demoState->sceneCamera->sceneObject->position.z = +cameraAxisPos;
		demoState->sceneCamera->sceneObject->euler.x = 60.0f;
		demoState->sceneCamera->sceneObject->euler.y = 0.0f;
		demoState->sceneCamera->sceneObject->euler.z = 135.0f;
	}
*/
	demoState->sceneCamera->sceneObject->position.x = 8.0f;
	demoState->sceneCamera->sceneObject->position.y = 4.0f;
	demoState->sceneCamera->sceneObject->position.z = cameraAxisPos;

	// same fovy to start
	demoState->sceneCamera->fovy = realSixty;


	// light
	if (demoState->verticalAxis)
	{
		demoState->lightObject->position.x = +cameraAxisPos;
		demoState->lightObject->position.y = +cameraAxisPos;
		demoState->lightObject->position.z = -cameraAxisPos;
	}
	else
	{
		demoState->lightObject->position.x = +cameraAxisPos;
		demoState->lightObject->position.y = +cameraAxisPos;
		demoState->lightObject->position.z = +cameraAxisPos;
	}


	// animation: preset a few path waypoints
	memset(demoState->waypoints, 0, sizeof(demoState->waypoints));
	demoState->waypointCountMax = sizeof(demoState->waypoints) / sizeof(p3vec3);
	demoState->currentWaypointIndex = 0;
	demoState->currentSegmentParam = 0.0f;

	demoState->waypointCount = 4;
	demoState->useHermiteCurveSegments = 0;

	demoState->waypoints[0].x = 0.0f;
	demoState->waypoints[1].x = 10.0f;
	demoState->waypoints[2].x = 15.0f;
	demoState->waypoints[3].x = 17.5f;

	demoState->waypointHandles[0].x = demoState->waypoints[0].x + 8.0f;
	demoState->waypointHandles[1].x = demoState->waypoints[1].x + 4.0f;
	demoState->waypointHandles[2].x = demoState->waypoints[2].x + 2.0f;
	demoState->waypointHandles[3].x = demoState->waypoints[3].x + 1.0f;

	demoState->waypointHandles[0].y = 12.0f;
	demoState->waypointHandles[1].y = 6.0f;
	demoState->waypointHandles[2].y = 3.0f;
	demoState->waypointHandles[3].y = 1.5f;


	// ****TO-DO: 
	//	- load keyframe data if file-based (replace hard-coded data above)

	demoState->pathTime = 0.0f;

	demoState->waypointTimes[0] = 0.0f;
	demoState->waypointTimes[1] = 4.20f;
	demoState->waypointTimes[2] = 6.0f;
	demoState->waypointTimes[3] = 8.0f;

	for (i = 1; i < demoState->waypointCount; ++i)
	{
		demoState->segmentDurations[i - 1] = demoState->waypointTimes[i] - demoState->waypointTimes[i - 1];
	}

	// ****TO-DO: 
	//	- generate speed control tables
	demoState->useSpeedControl = 1;
	demoState->samplesPerSegment = 32;
	{
		//algorithm:
		//foreach segment in path
		//	calculate samples and arc length
		//	accumulate arc length
		//normalize all arc length (/ total)
		const unsigned int maxTableSamples = 256;
		const unsigned int samplesPerSegment = demoState->samplesPerSegment;
		const unsigned int numSegments = demoState->waypointCount - 1;
		
		//s is sampling index
		unsigned int s, segmentEndIndex;

		float totalArcLength;
		float sampleDistance;
		float t, dt = 1.0f / (float)samplesPerSegment;

		p3vec3* sampleTablePtr, p0, p1, s0, s1;
		float* arcLengthTablePtr;

		// lerp tables
		totalArcLength = 0.0f;

		for (i = segmentEndIndex = 0; i < numSegments; ++i, segmentEndIndex += samplesPerSegment)
		{
			sampleTablePtr = demoState->pathSampleTableLerp + segmentEndIndex;
			arcLengthTablePtr = demoState->arcLengthTableLerp + segmentEndIndex;

			// sampling
			p0 = demoState->waypoints[i];
			p1 = demoState->waypoints[i + 1];

			s1 = s0 = p0;

			// first table entry
			*sampleTablePtr = s1;
			*arcLengthTablePtr = totalArcLength; // not param

			for (s = 1, t = dt; s <= samplesPerSegment; ++s, t += dt, s0 = s1)
			{
				//calc next sample and distance from prev
				//next sample is s1
				p3real3Lerp(s1.v, p0.v, p1.v, t);
				sampleDistance = p3real3Distance(s0.v, s1.v);

				//accumulate
				totalArcLength += sampleDistance;

				*(++sampleTablePtr) = s1;
				*(++arcLengthTablePtr) = totalArcLength;
			}
		}

		//after all segments normalize arc length
		for (s = 0; s < maxTableSamples; ++s)
		{
			demoState->arcLengthTableLerp[s] /= totalArcLength;
		}
	}
}


//-----------------------------------------------------------------------------

// confirm that all graphics objects were unloaded
void a3demo_validateUnload(const a3_DemoState *demoState)
{
	unsigned int handle;
	const a3_Texture *currentTex = demoState->texture,
		*const endTex = currentTex + demoStateMaxCount_texture;
	const a3_BufferObject *currentBuff = demoState->drawDataBuffer,
		*const endBuff = currentBuff + demoStateMaxCount_drawDataBuffer;
	const a3_VertexArrayDescriptor *currentVAO = demoState->vertexArray,
		*const endVAO = currentVAO + demoStateMaxCount_vertexArray;
	const a3_DemoStateShaderProgram *currentProg = demoState->shaderProgram,
		*const endProg = currentProg + demoStateMaxCount_shaderProgram;

	handle = 0;
	currentTex = demoState->texture;
	while (currentTex < endTex)
		handle += (currentTex++)->handle->handle;
	if (handle)
		printf("\n A3 Warning: One or more texture not released.");

	handle = 0;
	currentBuff = demoState->drawDataBuffer;
	while (currentBuff < endBuff)
		handle += (currentBuff++)->handle->handle;
	if (handle)
		printf("\n A3 Warning: One or more draw data buffers not released.");

	handle = 0;
	currentVAO = demoState->vertexArray;
	while (currentVAO < endVAO)
		handle += (currentVAO++)->handle->handle;
	if (handle)
		printf("\n A3 Warning: One or more vertex arrays not released.");

	handle = 0;
	currentProg = demoState->shaderProgram;
	while (currentProg < endProg)
		handle += (currentProg++)->program->handle->handle;
	if (handle)
		printf("\n A3 Warning: One or more shader programs not released.");
}


//-----------------------------------------------------------------------------

void a3demo_refresh(a3_DemoState *demoState)
{
	// the handle release callbacks are no longer valid; since the library was 
	//	reloaded, old function pointers are out of scope!
	// could reload everything, but that would mean rebuilding GPU data...
	//	...or just set new function pointers!

	a3_Texture *currentTex = demoState->texture,
		*const endTex = currentTex + demoStateMaxCount_texture;
	a3_BufferObject *currentBuff = demoState->drawDataBuffer,
		*const endBuff = currentBuff + demoStateMaxCount_drawDataBuffer;
	a3_VertexArrayDescriptor *currentVAO = demoState->vertexArray,
		*const endVAO = currentVAO + demoStateMaxCount_vertexArray;
	a3_DemoStateShaderProgram *currentProg = demoState->shaderProgram,
		*const endProg = currentProg + demoStateMaxCount_shaderProgram;

	while (currentTex < endTex)
		a3textureHandleUpdateReleaseCallback(currentTex++);
	while (currentBuff < endBuff)
		a3bufferHandleUpdateReleaseCallback(currentBuff++);
	while (currentVAO < endVAO)
		a3vertexArrayHandleUpdateReleaseCallback(currentVAO++);
	while (currentProg < endProg)
		a3shaderProgramHandleUpdateReleaseCallback((currentProg++)->program);
}

void a3demo_update(a3_DemoState *demoState, double dt)
{
	unsigned int i;


	// ****TO-DO: 
	//	- animate path-following object
	//		1. add time change to total path time
	//		2. if path time > segment end time move onto the next segment
	//		3. calc normalized parameter for time



	// ****TO-DO	slides 7 and 29
	//	- implement speed control sampling

	if (demoState->useSpeedControl)
	{
		const unsigned int segmentCount = demoState->waypointCount - 1;
		const float pathDuration = demoState->waypointTimes[segmentCount];
		float remapParam;		

		// table pointers
		p3vec3* sampleTablePtr;
		float* arcLengthTablePtr;

		// start and end
		float remapStart, remapEnd;

		demoState->pathTime += (float)dt;

		// looping stuff here
		if (demoState->pathTime >= pathDuration)
			demoState->pathTime -= pathDuration;

		// algorithm: remap time to distance
		remapParam = demoState->pathTime / pathDuration;

		// which 2 samples we are in between
		// how far in between them are we
		// lerp between samples using param ^
		sampleTablePtr = demoState->pathSampleTableLerp;
		arcLengthTablePtr = demoState->arcLengthTableLerp;

		remapStart = arcLengthTablePtr[0];
		remapEnd = arcLengthTablePtr[1];
		i = 1;

		// check if param is in between the two remap arch lengths (start/end)
		while (!(remapParam >= remapStart && remapParam <= remapEnd))
		{
			remapStart = remapEnd;
			remapEnd = arcLengthTablePtr[++i];
		}

		demoState->currentSegmentParam = unlerp(remapStart, remapEnd, remapParam);
		
		p3real3Lerp(demoState->pathObject->position.v,
			sampleTablePtr[i - 1].v, sampleTablePtr[i].v, demoState->currentSegmentParam);
	}

	else
	{
		const unsigned segmentCount = demoState->waypointCount - 1;
		float segmentStart = demoState->waypointTimes[demoState->currentWaypointIndex];
		float segmentEnd = demoState->waypointTimes[demoState->currentWaypointIndex + 1];

		//1
		demoState->pathTime += (float)dt;

		//2
		if (demoState->pathTime >= demoState->waypointTimes[demoState->currentWaypointIndex + 1])
		{
			//looping behaviors
			if (demoState->currentWaypointIndex == segmentCount)
			{
				demoState->pathTime -= demoState->waypointTimes[segmentCount];
				demoState->currentWaypointIndex = 0;
			}
			else
			{
				++demoState->currentWaypointIndex;
			}

			//fix segment start and end
			segmentStart = demoState->waypointTimes[demoState->currentWaypointIndex];
		}	segmentEnd = demoState->waypointTimes[demoState->currentWaypointIndex + 1];

		//3
		/*
			unlerp formula

			(n - n0) / (n1 - n0)
		*/
		demoState->currentSegmentParam = unlerp(segmentStart, segmentEnd, demoState->pathTime);


		// control teapot
		{
			const p3vec3 p0 = demoState->waypoints[demoState->currentWaypointIndex];
			const p3vec3 p1 = demoState->waypoints[demoState->currentWaypointIndex + 1];
			const p3vec3 h0 = demoState->waypointHandles[demoState->currentWaypointIndex];
			const p3vec3 h1 = demoState->waypointHandles[demoState->currentWaypointIndex + 1];

			if (demoState->useHermiteCurveSegments)
				p3real3HermiteControl(demoState->pathObject->position.v, p0.v, p1.v, h0.v, h1.v, demoState->currentSegmentParam);
			else
				p3real3Lerp(demoState->pathObject->position.v, p0.v, p1.v, demoState->currentSegmentParam);
		}
	}

	// DO THIS ***
	// given current velocity (in demostate)
	// calc target velocty (from keys or controller)
	{
		//oops
		p3vec3 tmpCurrentVelocity = p3zeroVec3;

		p3vec3 tmpTargetVelocity = p3xVec3;

		//lerp velocity towards target												// param is smoothness, acceleration kinda i think?
		p3real3Lerp(tmpCurrentVelocity.v, tmpCurrentVelocity.v, tmpTargetVelocity.v, .35f);

		// calc position by integrating velocity
		p3vec3 tmp;
		p3real3ProductS(tmp.v, tmpTargetVelocity.v, (float)dt);
		p3real3Add(tmpTargetVelocity.v, tmp.v);
	}


	// controls
	
	// move and rotate camera
	a3demo_moveSceneObject(demoState->camera->sceneObject, (float)dt * demoState->camera->ctrlMoveSpeed,
		(p3real)a3keyboardGetDifference(demoState->keyboard, a3key_D, a3key_A),
		(p3real)a3keyboardGetDifference(demoState->keyboard, a3key_E, a3key_Q),
		(p3real)a3keyboardGetDifference(demoState->keyboard, a3key_S, a3key_W)
	);
	if (a3mouseIsHeld(demoState->mouse, a3mouse_left))
	{
		const p3real azimuth = 0.0f;	// -(p3real)a3mouseGetDeltaX(demoState->mouse);
		const p3real elevation = 0.0f;	// -(p3real)a3mouseGetDeltaY(demoState->mouse);

		// this really defines which way is "up"
		// mouse's Y motion controls pitch, but X can control yaw or roll
		// controlling yaw makes Y axis seem "up", roll makes Z seem "up"
		a3demo_rotateSceneObject(demoState->camera->sceneObject, (float)dt * demoState->camera->ctrlRotateSpeed,
			// pitch: vertical tilt
			elevation,
			// yaw/roll depends on "vertical" axis: if y, yaw; if z, roll
			demoState->verticalAxis ? azimuth : realZero,
			demoState->verticalAxis ? realZero : azimuth);
	}


	// update scene objects
	for (i = 0; i < demoStateMaxCount_sceneObject; ++i)
		a3demo_updateSceneObject(demoState->sceneObject + i);

	// update cameras
	for (i = 0; i < demoStateMaxCount_camera; ++i)
		a3demo_updateCameraViewProjection(demoState->camera + i);


	// update input
	a3mouseUpdate(demoState->mouse);
	a3keyboardUpdate(demoState->keyboard);
	a3XboxControlUpdate(demoState->xcontrol);
}

void a3demo_render(a3_DemoState *demoState)
{
	const a3_VertexDrawable *currentDrawable;
	const a3_DemoStateShaderProgram *currentDemoProgram;

	const int useVerticalY = demoState->verticalAxis;

	unsigned int i;


	// grid lines highlight
	// if Y axis is up, give it a greenish hue
	// if Z axis is up, a bit of blue
	const float gridColor[] = {
		0.15f,
		useVerticalY ? 0.25f : 0.20f,
		useVerticalY ? 0.20f : 0.25f,
		1.0f
	};


	// curve handle color: orange
	const float handleColor[] = {
		1.0f, 0.5f, 0.0f, 1.0f
	};

	// waypoint color: blue
	const float waypointColor[] = {
		0.0f, 0.5f, 1.0f, 1.0f
	};


	// model transformations (if needed)
	const p3mat4 convertY2Z = {
		+1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, +1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, +1.0f,
	};
	const p3mat4 convertZ2Y = {
		+1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, +1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, +1.0f,
	};

	// transformed vectors
	p3vec4 lightPos_obj, eyePos_obj;

	// final model matrix and full matrix stack
	p3mat4 modelMat = p3identityMat4, modelMatInv = p3identityMat4, modelViewProjectionMat = p3identityMat4;

	// current scene object being rendered, for convenience
	a3_DemoSceneObject *currentSceneObject;


	// reset viewport and clear buffers
	a3framebufferDeactivateSetViewport(a3fbo_depth24, -demoState->frameBorder, -demoState->frameBorder, demoState->frameWidth, demoState->frameHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// draw objects: 
	//	- correct "up" axis if needed
	//	- calculate full MVP matrix
	//	- move lighting objects' positions into object space
	//	- send uniforms
	//	- draw


	// draw teapot & earth
	currentDemoProgram = demoState->prog_drawPhong_obj;
	a3shaderProgramActivate(currentDemoProgram->program);

/*	
	currentDrawable = demoState->draw_teapot;
	currentSceneObject = demoState->teapotObject;
	if (!useVerticalY)	// teapot's axis is Y
	{
		p3real4x4ProductTransform(modelMat.m, convertY2Z.m, currentSceneObject->modelMat.m);
		p3real4x4TransformInverseIgnoreScale(modelMatInv.m, modelMat.m);
	}
	else
	{
		modelMat = currentSceneObject->modelMat;
		modelMatInv = currentSceneObject->modelMatInv;
	}
	p3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMat.m);
	p3real4TransformProduct(lightPos_obj.v, modelMatInv.m, demoState->lightObject->modelMat.v3.v);
	p3real4TransformProduct(eyePos_obj.v, modelMatInv.m, demoState->cameraObject->modelMat.v3.v);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uLightPos_obj, 1, lightPos_obj.v);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uEyePos_obj, 1, eyePos_obj.v);
	a3textureActivate(demoState->tex_checker, a3tex_unit00);
	a3textureActivate(demoState->tex_checker, a3tex_unit01);
	a3vertexActivateAndRenderDrawable(currentDrawable);
*/

//	currentDrawable = demoState->draw_sphere;
//	a3textureActivate(demoState->tex_earth_dm, a3tex_unit00);
//	a3textureActivate(demoState->tex_earth_sm, a3tex_unit01);

	currentDrawable = demoState->draw_teapot;
	a3textureActivate(demoState->tex_checker, a3tex_unit00);
	a3textureActivate(demoState->tex_checker, a3tex_unit01);

	currentSceneObject = demoState->pathObject;
	if (useVerticalY)	// sphere's axis is Z
	{
		p3real4x4ProductTransform(modelMat.m, convertZ2Y.m, currentSceneObject->modelMat.m);
		p3real4x4TransformInverseIgnoreScale(modelMatInv.m, modelMat.m);
	}
	else
	{
		modelMat = currentSceneObject->modelMat;
		modelMatInv = currentSceneObject->modelMatInv;
	}
	p3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMat.m);
	p3real4TransformProduct(lightPos_obj.v, modelMatInv.m, demoState->lightObject->modelMat.v3.v);
	p3real4TransformProduct(eyePos_obj.v, modelMatInv.m, demoState->cameraObject->modelMat.v3.v);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uLightPos_obj, 1, lightPos_obj.v);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uEyePos_obj, 1, eyePos_obj.v);
	a3vertexActivateAndRenderDrawable(currentDrawable);


	// draw grid aligned to world
	currentDemoProgram = demoState->prog_drawColorUnif;
	a3shaderProgramActivate(currentDemoProgram->program);
	currentDrawable = demoState->draw_grid;
	modelViewProjectionMat = demoState->camera->viewProjectionMat;
	if (useVerticalY)
		p3real4x4ConcatL(modelViewProjectionMat.m, convertZ2Y.m);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, gridColor);
	a3vertexActivateAndRenderDrawable(currentDrawable);


	// overlay items
	glDisable(GL_DEPTH_TEST);

	// draw curve segments
	if (demoState->useHermiteCurveSegments)
		currentDemoProgram = demoState->prog_drawCurve;
	else
		currentDemoProgram = demoState->prog_drawLine;
	a3shaderProgramActivate(currentDemoProgram->program);
	modelViewProjectionMat = demoState->camera->viewProjectionMat;
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
	a3shaderUniformSendFloat(a3unif_vec3, currentDemoProgram->uWaypoints, demoState->waypointCount, demoState->waypoints->v);
	a3shaderUniformSendFloat(a3unif_vec3, currentDemoProgram->uWaypointHandles, demoState->waypointCount, demoState->waypointHandles->v);
//	a3shaderUniformSendInt(a3unif_single, currentDemoProgram->uWaypointCount, 1, &demoState->waypointCount);
//	a3shaderUniformSendInt(a3unif_single, currentDemoProgram->uWaypointIndex, 1, &demoState->currentWaypointIndex);

	// render segments if there are enough waypoints
	currentDrawable = demoState->draw_curve;
	if (demoState->waypointCount > 1)
		a3vertexActivateAndRenderDrawableInstanced(currentDrawable, demoState->waypointCount - 1);

	// draw waypoints
	currentDemoProgram = demoState->prog_drawColorUnif;
	a3shaderProgramActivate(currentDemoProgram->program);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, waypointColor);

	modelMat = p3identityMat4;
	currentDrawable = demoState->draw_node;
	a3vertexActivateDrawable(currentDrawable);
	for (i = 0; i < demoState->waypointCount; ++i)
	{
		modelMat.v3.xyz = demoState->waypoints[i];
		p3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMat.m);
		a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
		a3vertexRenderActiveDrawable();
	}

	// draw handles
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, handleColor);

	modelMat = p3identityMat4;
	currentDrawable = demoState->draw_node;
	a3vertexActivateDrawable(currentDrawable);
	for (i = 0; i < demoState->waypointCount; ++i)
	{
		modelMat.v3.xyz = demoState->waypointHandles[i];
		p3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMat.m);
		a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
		a3vertexRenderActiveDrawable();
	}

	glEnable(GL_DEPTH_TEST);


	// draw coordinate axes in front of everything
	glDisable(GL_DEPTH_TEST);
	currentDemoProgram = demoState->prog_drawColor;
	a3shaderProgramActivate(currentDemoProgram->program);
	currentDrawable = demoState->draw_axes;
	modelViewProjectionMat = demoState->camera->viewProjectionMat;
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
	a3vertexActivateAndRenderDrawable(currentDrawable);
	glEnable(GL_DEPTH_TEST);


	// deactivate things
	a3vertexDeactivateDrawable();
	a3shaderProgramDeactivate();


	// HUD
	if (demoState->textInit && demoState->showText)
	{
		const char *curveTypeText = demoState->useHermiteCurveSegments ? "Hermite" : "Lines";
		glDisable(GL_DEPTH_TEST);
		a3textDraw(demoState->text,
			-0.9f, -0.7f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"Path type (toggle = \'h\'): %s", curveTypeText);
		a3textDraw(demoState->text,
			-0.9f, -0.8f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"Path segment: %u	Duration: %f", demoState->currentWaypointIndex, demoState->segmentDurations[demoState->currentWaypointIndex]);
		a3textDraw(demoState->text,
			-0.9f, -0.9f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"Path time: %f	Segment param: %f", demoState->pathTime, demoState->currentSegmentParam);
		glEnable(GL_DEPTH_TEST);
	}
}


//-----------------------------------------------------------------------------
