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
	static const p3mat4 downscale20x = {
		+0.05f, 0.0f, 0.0f, 0.0f,
		0.0f, +0.05f, 0.0f, 0.0f,
		0.0f, 0.0f, +0.05f, 0.0f,
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
	a3_GeometryData overlayShapesData[4] = { 0 };
	a3_GeometryData proceduralShapesData[1] = { 0 };
	a3_GeometryData loadedModelsData[1] = { 0 };
	const unsigned int sceneShapesCount = sizeof(sceneShapesData) / sizeof(a3_GeometryData);
	const unsigned int overlayShapesCount = sizeof(overlayShapesData) / sizeof(a3_GeometryData);
	const unsigned int proceduralShapesCount = sizeof(proceduralShapesData) / sizeof(a3_GeometryData);
	const unsigned int loadedModelsCount = sizeof(loadedModelsData) / sizeof(a3_GeometryData);

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
		a3_ProceduralGeometryDescriptor overlayShapes[4] = { a3geomShape_none };
		a3_ProceduralGeometryDescriptor proceduralShapes[1] = { a3geomShape_none };

		// static scene procedural objects
		//	(axes and grid)
		a3proceduralCreateDescriptorAxes(sceneShapes + 0, a3geomFlag_wireframe, 0.0f, 1);
		a3proceduralCreateDescriptorPlane(sceneShapes + 1, a3geomFlag_wireframe, a3geomAxis_default, 32.0f, 32.0f, 32, 32);
		for (i = 0; i < sceneShapesCount; ++i)
		{
			a3proceduralGenerateGeometryData(sceneShapesData + i, sceneShapes + i);
			a3fileStreamWriteObject(fileStream, sceneShapesData + i, (a3_FileStreamWriteFunc)a3geometrySaveDataBinary);
		}


		// single-vertex shape for curve segment drawing
		{
			// create vertex format
			a3_GeometryData *pointData = overlayShapesData + 0;
			a3geometryCreateVertexFormat(pointData->vertexFormat, 0, 0);
			pointData->primType = a3prim_points;
			pointData->numVertices = 1;
			pointData->attribData[0] = pointData->data = malloc(3 * sizeof(float));
			memset(pointData->data, 0, 3 * sizeof(float));
			a3fileStreamWriteObject(fileStream, pointData, (a3_FileStreamWriteFunc)a3geometrySaveDataBinary);
		}

		// static overlay objects
		// first, "node" or "waypoint" object, just a low-resolution sphere
		// next, a background sphere and torus or ring for a gimbal
		demoState->arcballSphere->center = p3wVec4;
		demoState->arcballSphere->radius = 6.0f;
		a3proceduralCreateDescriptorSphere(overlayShapes + 1, a3geomFlag_vanilla, a3geomAxis_default, 0.1f, 8, 4);
		a3proceduralCreateDescriptorSphere(overlayShapes + 2, a3geomFlag_vanilla, a3geomAxis_default, demoState->arcballSphere->radius, 64, 48);
		a3proceduralCreateDescriptorCircle(overlayShapes + 3, a3geomFlag_wireframe, a3geomAxis_default, demoState->arcballSphere->radius, 64, 1);
	//	a3proceduralCreateDescriptorTorus(overlayShapes + 3, a3geomFlag_wireframe, a3geomAxis_default, demoState->arcballSphere->radius, 0.01f, 64, 4);
		for (i = 1; i < overlayShapesCount; ++i)
		{
			a3proceduralGenerateGeometryData(overlayShapesData + i, overlayShapes + i);
			a3fileStreamWriteObject(fileStream, overlayShapesData + i, (a3_FileStreamWriteFunc)a3geometrySaveDataBinary);
		}


		// procedural
		a3proceduralCreateDescriptorSphere(proceduralShapes + 0, a3geomFlag_tangents, a3geomAxis_default, 1.0f, 24, 16);
		for (i = 0; i < proceduralShapesCount; ++i)
		{
			a3proceduralGenerateGeometryData(proceduralShapesData + i, proceduralShapes + i);
			a3fileStreamWriteObject(fileStream, proceduralShapesData + i, (a3_FileStreamWriteFunc)a3geometrySaveDataBinary);
		}

		// loaded models
		a3modelLoadOBJ(loadedModelsData + 0, "../../../../resource/obj/teapot/teapot.obj", a3model_calculateVertexTangents, downscale20x.mm);
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
	currentDrawable = demoState->draw_curve;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, overlayShapesData + 0, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);
	currentDrawable = demoState->draw_node;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, overlayShapesData + 1, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);
	currentDrawable = demoState->draw_gimbal;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, overlayShapesData + 2, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);
	currentDrawable = demoState->draw_ring;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, overlayShapesData + 3, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);

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
		"uLineColor",
		"uCurveColor",

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
		if ((uLocation = currentDemoProg->uLineColor) >= 0)
			a3shaderUniformSendFloat(a3unif_vec4, uLocation, 1, defaultColor);
		if ((uLocation = currentDemoProg->uCurveColor) >= 0)
			a3shaderUniformSendFloat(a3unif_vec4, uLocation, 1, defaultColor);
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

//	demoState->sceneCamera->sceneObject->position.x = 8.0f;
//	demoState->sceneCamera->sceneObject->position.y = 4.0f;
//	demoState->sceneCamera->sceneObject->position.z = cameraAxisPos;

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


	// create a short "path" of orientations using quaternions
	//	(first and last are identity)
	a3quatCreateIdentity(demoState->slerpPathTargets[0]);
	a3quatCreateAxisAngle(demoState->slerpPathTargets[1], p3xVec3.v, realNinety);
	a3quatCreateAxisAngle(demoState->slerpPathTargets[2], p3yVec3.v, realNinety);
	a3quatCreateAxisAngle(demoState->slerpPathTargets[3], p3zVec3.v, realNinety);
	a3quatCreateIdentity(demoState->slerpPathTargets[4]);
	demoState->slerpPathDuration = 4.0f;

	// initialize quaternions
	a3quatCreateIdentity(demoState->arcballOriginalOrientation);
	a3quatCreateIdentity(demoState->arcballOrientation);
	a3quatCreateIdentity(demoState->arcballTargetOrientation);
	a3quatCreateIdentity(demoState->arcballAngularVelocity);
	a3quatCreateIdentity(demoState->arcballTargetAngularVelocity);

	// smoothing factor: low is very noticeable
	demoState->arcballTargetSmoothing = 0.125f;

	// set default joystick vectors
	demoState->joystickInitialVector = demoState->joystickActiveVector = p3zVec3;
	demoState->raypickInitialVector = demoState->raypickActiveVector = p3zVec3;
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

	// temporary quaternion for integrating angular velocity
	a3quat tmpVel;


	// keyframe controller for path update
	{
		const unsigned int slerpSegmentCount = 4;
		const p3real slerpSegmentDuration = demoState->slerpPathDuration / (float)slerpSegmentCount;
		demoState->slerpSegmentTime += (float)dt;
		if (demoState->slerpSegmentTime >= slerpSegmentDuration)
		{
			demoState->slerpSegmentTime -= slerpSegmentDuration;
			demoState->slerpSegmentIndex = (demoState->slerpSegmentIndex + 1) % slerpSegmentCount;
		}
		demoState->slerpSegmentParam = demoState->slerpSegmentTime / slerpSegmentDuration;
	}


	// controller/keyboard input
	{
		const float joystickMax = 0.7f;

		// ****TO-DO: 
		// read keyboard or joystick input, calculate arcball vector
		if (demoState->xcontrol->connected)
		{
			double joystick[2];
			a3XboxControlGetLeftJoystick(demoState->xcontrol, joystick);

		}
		else
		{

		}

		// ****TO-DO: 
		// calculate Z value using core arcball algorithm: 
		//	->	r^2 = x^2 + y^2 + z^2
		//		z^2 = r^2 - x^2 - y^2
		//		z^2 = 1 - (x^2 + y^2)	<- 2D length squared

	}


	// ****TO-DO: control orientation based on control mode
	switch (demoState->quatDemoMode)
	{
		// clamp to path
	case 0:
		a3quatUnitSLERP(demoState->arcballOrientation,
			demoState->slerpPathTargets[demoState->slerpSegmentIndex],
			demoState->slerpPathTargets[demoState->slerpSegmentIndex + 1], demoState->slerpSegmentParam);
		break;

		// target path
	case 1:
		// have target follow path
		a3quatUnitSLERP(demoState->arcballTargetOrientation,
			demoState->slerpPathTargets[demoState->slerpSegmentIndex],
			demoState->slerpPathTargets[demoState->slerpSegmentIndex + 1], demoState->slerpSegmentParam);

		// interpolate actual orientation towards target
		a3quatUnitSLERP(demoState->arcballOrientation,
			demoState->arcballOrientation,
			demoState->arcballTargetOrientation, demoState->arcballTargetSmoothing);
		break;


		// set arcball orientation
	case 2:
		// quat between joystick inital and current
		break;

		// target arcball orientation
	case 3:
		break;

		// set arcball angular velocity
	case 4:
		break;

		// target arcball angular velocity
	case 5:
		break;


		// set position using ray pick
	case 6:
		break;

		// set target position using ray pick
	case 7:
		break;

		// set angular velocity using ray pick
	case 8:
		break;

		// set target angular velocity using ray pick
	case 9:
		break;


		// set delta on arcball
	case 10:
		break;

		// set target delta on arcball
	case 11:
		break;
	}


	// in any case normalize orientation
	p3real4Normalize(demoState->arcballOrientation);


	// update waypoints for drawing controls
	if (demoState->quatDemoMode >= 2 && demoState->quatDemoMode < 6)
	{
		p3real3ProductS(demoState->lineEnds[1].v, demoState->joystickInitialVector.v, demoState->arcballSphere->radius);
		p3real3ProductS(demoState->lineEnds[3].v, demoState->joystickActiveVector.v, demoState->arcballSphere->radius);
	}
	else if (demoState->quatDemoMode >= 6)
	{
		p3real3ProductS(demoState->lineEnds[1].v, demoState->raypickInitialVector.v, demoState->arcballSphere->radius);
		p3real3ProductS(demoState->lineEnds[3].v, demoState->raypickActiveVector.v, demoState->arcballSphere->radius);
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
		const p3real azimuth = -(p3real)a3mouseGetDeltaX(demoState->mouse);
		const p3real elevation = -(p3real)a3mouseGetDeltaY(demoState->mouse);

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


	// update model matrix of quaternion objects
	a3quatConvertToMat4(demoState->rotateObject->modelMat.m, 
		demoState->arcballOrientation, demoState->rotateObject->position.v);


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


	// RGB
	const float rgba4[] = {
		1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 0.5f, 
	};

	// curve handle color: orange
	const float handleColor[] = {
		1.0f, 0.5f, 0.0f, 1.0f
	};

	// waypoint color: blue
	const float waypointColor[] = {
		0.0f, 0.5f, 1.0f, 1.0f
	};

	// target color: pink
	const float targetColor[] = {
		1.0f, 0.0f, 0.5f, 1.0f
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
	const p3mat4 convertZ2X = {
		0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, +1.0f, 0.0f, 0.0f,
		+1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, +1.0f,
	};

	// transformed vectors
	p3vec4 lightPos_obj, eyePos_obj;

	// final model matrix and full matrix stack
	p3mat4 modelMatOrig, modelMat = p3identityMat4, modelMatInv = p3identityMat4, modelViewProjectionMat = p3identityMat4;

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

	
	currentDrawable = demoState->draw_teapot;
	currentSceneObject = demoState->rotateObject;
	if (!useVerticalY)	// teapot's axis is Y
	{
		modelMatOrig = currentSceneObject->modelMat;
		p3real4x4ProductTransform(modelMat.m, modelMatOrig.m, convertY2Z.m);
		p3real4x4TransformInverseIgnoreScale(modelMatInv.m, modelMat.m);
	}
	else
	{
		modelMatOrig = modelMat = currentSceneObject->modelMat;
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
	
	// draw gimbal
	currentDemoProgram = demoState->prog_drawColorUnif;
	a3shaderProgramActivate(currentDemoProgram->program);

	glCullFace(GL_FRONT);
	currentDrawable = demoState->draw_gimbal;
	a3vertexActivateDrawable(currentDrawable);
	p3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMatOrig.m);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, rgba4 + 12);
	a3vertexRenderActiveDrawable();
	glCullFace(GL_BACK);

	currentDrawable = demoState->draw_ring;
	a3vertexActivateDrawable(currentDrawable);
	p3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMatOrig.m);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, rgba4 + 8);
	a3vertexRenderActiveDrawable();
	p3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMatOrig.m);
	p3real4x4ConcatL(modelViewProjectionMat.m, convertZ2Y.m);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, rgba4 + 4);
	a3vertexRenderActiveDrawable();
	p3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMatOrig.m);
	p3real4x4ConcatL(modelViewProjectionMat.m, convertZ2X.m);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, rgba4 + 0);
	a3vertexRenderActiveDrawable();


	// overlays
	if (demoState->quatDemoMode >= 2)
	{
		const int rmbDown = a3mouseGetState(demoState->mouse, a3mouse_right);

		glDisable(GL_DEPTH_TEST);

		// draw line segments
		currentDemoProgram = demoState->prog_drawLine;
		modelViewProjectionMat = demoState->camera->viewProjectionMat;
		a3shaderProgramActivate(currentDemoProgram->program);
		a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);

		// render segments if there are enough waypoints
		if (demoState->quatDemoMode >= 6 && rmbDown || demoState->quatDemoMode < 6)
		{
			currentDrawable = demoState->draw_curve;
			a3vertexActivateDrawable(currentDrawable);
			a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uLineColor, 1, handleColor);
			a3shaderUniformSendFloat(a3unif_vec3, currentDemoProgram->uWaypoints, 2, demoState->lineEnds[0].v);
			a3vertexRenderActiveDrawable();
			a3shaderUniformSendFloat(a3unif_vec3, currentDemoProgram->uWaypoints, 2, demoState->lineEnds[2].v);
			a3vertexRenderActiveDrawable();
		}

		// render raypicking path
		if (demoState->quatDemoMode >= 6 && rmbDown && demoState->rotatePathSampleCount)
		{
			a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uLineColor, 1, targetColor);
			a3shaderUniformSendFloat(a3unif_vec3, currentDemoProgram->uWaypoints, demoState->rotatePathSampleCount, demoState->rotatePathSamples->v);
			a3vertexRenderActiveDrawableInstanced(demoState->rotatePathSampleCount - 1);
		}

		// draw waypoints
		currentDemoProgram = demoState->prog_drawColorUnif;
		a3shaderProgramActivate(currentDemoProgram->program);
		a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, waypointColor);

		modelMat = p3identityMat4;
		currentDrawable = demoState->draw_node;
		a3vertexActivateDrawable(currentDrawable);
		for (i = 0; i < 4; ++i)
		{
			modelMat.v3.xyz = demoState->lineEnds[i];
			p3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMat.m);
			a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
			a3vertexRenderActiveDrawable();
		}

		glEnable(GL_DEPTH_TEST);


		// draw original shape in trackball mode
		if (demoState->quatDemoMode >= 10 && rmbDown)
		{
			// redraw object
			currentDemoProgram = demoState->prog_drawColorUnif;
			a3shaderProgramActivate(currentDemoProgram->program);
			a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, rgba4 + 12);
	
			currentDrawable = demoState->draw_teapot;
			currentSceneObject = demoState->rotateObject;
			if (!useVerticalY)	// teapot's axis is Y
			{
				a3quatConvertToMat4(modelMatOrig.m,
					demoState->arcballOriginalOrientation, demoState->rotateObject->position.v);
				p3real4x4ProductTransform(modelMat.m, modelMatOrig.m, convertY2Z.m);
			}
			else
			{
				a3quatConvertToMat4(modelMatOrig.m,
					demoState->arcballOriginalOrientation, demoState->rotateObject->position.v);
				modelMat = modelMatOrig;
			}

			glCullFace(GL_FRONT);
			p3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMat.m);
			a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
			a3vertexActivateAndRenderDrawable(currentDrawable);
			glCullFace(GL_BACK);


			// redraw gimbal rings
			currentDrawable = demoState->draw_ring;
			a3vertexActivateDrawable(currentDrawable);
			p3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMatOrig.m);
			a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
			a3vertexRenderActiveDrawable();
			p3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMatOrig.m);
			p3real4x4ConcatL(modelViewProjectionMat.m, convertZ2Y.m);
			a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
			a3vertexRenderActiveDrawable();
			p3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMatOrig.m);
			p3real4x4ConcatL(modelViewProjectionMat.m, convertZ2X.m);
			a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
			a3vertexRenderActiveDrawable();
		}
	}


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
		const char *quatDemoModeText[] = {
			"Set position    (orientation path)",
			"Target position (orientation path)",
			"Set position    (controller/keyboard input; left joystick / IJKL)",
			"Target position (controller/keyboard input; left joystick / IJKL)",
			"Set velocity    (controller/keyboard input; left joystick / IJKL)",
			"Target velocity (controller/keyboard input; left joystick / IJKL)",
			"Set position    (arcball mouse input; right click & drag)",
			"Target position (arcball mouse input; right click & drag)",
			"Set velocity    (arcball mouse input; right click & drag)",
			"Target velocity (arcball mouse input; right click & drag)",
			"Set position    (trackball mouse input; right click & drag)",
			"Target position (trackball mouse input; right click & drag)",
		};

		p3vec3 axis;
		p3real angle;

		glDisable(GL_DEPTH_TEST);

		// print mode
		a3textDraw(demoState->text,
			-0.98f, +0.95f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 
			"Quaternion demo control mode %u / 12 (toggles = ','/'.'): ", (demoState->quatDemoMode + 1));
		a3textDraw(demoState->text,
			-0.98f, +0.90f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"    %s", quatDemoModeText[demoState->quatDemoMode]);

		a3textDraw(demoState->text,
			-0.98f, +0.85f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"Target smoothing: %f", demoState->arcballTargetSmoothing);

		// print quaternion values
		a3quatGetAxisAngle(axis.v, &angle, demoState->arcballOrientation);
		a3textDraw(demoState->text,
			-0.98f, -0.30f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"Arcball orientation (xyzw): ");
		a3textDraw(demoState->text,
			-0.98f, -0.35f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"    q = (%f, %f, %f, %f)", demoState->arcballOrientation[0], demoState->arcballOrientation[1], demoState->arcballOrientation[2], demoState->arcballOrientation[3]);
		a3textDraw(demoState->text,
			-0.98f, -0.40f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"    axis = (%f, %f, %f), angle = %f degrees", axis.x, axis.y, axis.z, angle);

		a3textDraw(demoState->text,
			-0.98f, -0.50f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"Arcball angular velocity: ");
		a3textDraw(demoState->text,
			-0.98f, -0.55f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"    q' = (%f, %f, %f, %f)", demoState->arcballAngularVelocity[0], demoState->arcballAngularVelocity[1], demoState->arcballAngularVelocity[2], demoState->arcballAngularVelocity[3]);

		a3quatGetAxisAngle(axis.v, &angle, demoState->arcballTargetOrientation);
		a3textDraw(demoState->text,
			-0.98f, -0.70f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"Arcball target orientation: ");
		a3textDraw(demoState->text,
			-0.98f, -0.75f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"    Q = (%f, %f, %f, %f)", demoState->arcballTargetOrientation[0], demoState->arcballTargetOrientation[1], demoState->arcballTargetOrientation[2], demoState->arcballTargetOrientation[3]);
		a3textDraw(demoState->text,
			-0.98f, -0.80f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"    axis = (%f, %f, %f), angle = %f degrees", axis.x, axis.y, axis.z, angle);

		a3textDraw(demoState->text,
			-0.98f, -0.90f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"Arcball target angular velocity: ");
		a3textDraw(demoState->text,
			-0.98f, -0.95f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"    Q' = (%f, %f, %f, %f)", demoState->arcballTargetAngularVelocity[0], demoState->arcballTargetAngularVelocity[1], demoState->arcballTargetAngularVelocity[2], demoState->arcballTargetAngularVelocity[3]);

		glEnable(GL_DEPTH_TEST);
	}
}


//-----------------------------------------------------------------------------
