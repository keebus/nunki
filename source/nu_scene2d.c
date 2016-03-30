/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include "nu_scene2d.h"
#include "nu_builtin_resources.h"
#include "nu_math.h"
#include "nu_libs.h"

/*-------------------------------------------------------------------------------------------------
 * Types
 *-----------------------------------------------------------------------------------------------*/

/* Instance types */
typedef struct {
	NuRect2 rect;
	uint    color;
} QuadSolid;

typedef struct {
	NuRect2 rect;
	uint    color;
	NuRect2 uvRect;
	uint    textureIndex;
} QuadTextured;

typedef enum {
	MESH_TYPE_QUAD_SOLID,
	MESH_TYPE_QUAD_TEXTURED,
} MeshType;

static const uint kMeshInstanceSize[] = {
	sizeof(QuadSolid),
	sizeof(QuadTextured)
};

static const NuPrimitiveType kMeshPrimitiveType[] = {
	NU_PRIMITIVE_TRIANGLE_STRIP,
	NU_PRIMITIVE_TRIANGLE_STRIP,
};

static const char* kMeshTypeStr[] = {
	"solid quad",
	"textured quad",
};

typedef struct {
	MeshType     meshType;
	NuTechnique  technique;
	NuBlendState blendState;
	NuTexture    texture;
} DeviceState;

typedef struct {
	DeviceState deviceState;
	uint firstInstanceOffset;
	uint instanceCount;
} Command;

typedef struct {
	float transform[16];
} Constants;

typedef struct NuScene2DImpl {
	NuAllocator        allocator;
	NuRect2i           viewport;
	Command*           commands;
	char*              instanceData;
} Scene2D;

static struct {
	bool initialized;
	NuAllocator allocator;
	uint quadMeshVertexBufferOffset;
	NuBuffer constantBuffer;
	NuBuffer primitivesVertexBuffer;
	NuBuffer instancesVertexBuffer;

	/* immediate context */
	NuContext immediateContext;
	NuRect2i  immediateViewport;
	bool      immediateHasCommand;
	Command   immediateCommand;
	char*     immediateInstanceData;
} gScene2D;

/*-------------------------------------------------------------------------------------------------
 * Static functions
 *-----------------------------------------------------------------------------------------------*/
#define EnforceInitialized() nEnforce(gScene2D.initialized, "Scene 2D module uninitialized.");

/**
 * Sets the device state so that draw commands can be issued.
 */
static void SetDeviceViewport(NuContext context, NuRect2i viewport)
{
	/* update the constant buffer */
	Constants constants;
	nOrtho((float)viewport.position.x,
		(float)viewport.position.y,
		(float)viewport.position.x + viewport.size.width,
		(float)viewport.position.y + viewport.size.height,
		0.f, 1.f, constants.transform);

	nuBufferUpdate(gScene2D.constantBuffer, 0, &constants, sizeof constants);

	nuDeviceSetConstantBuffers(context, 0, 1, (NuBufferView[]) {
		gScene2D.constantBuffer, 0, 0
	});

	/* set device viewport */
	nuDeviceSetViewport(context, viewport);
}

/**
 * Executes a single draw command.
 */
static void ExecuteCommand(Command* command, NuContext context)
{
	DeviceState* state = &command->deviceState;
	
	nuDeviceSetBlendState(context, &state->blendState);

	/* set device state */
	nuDeviceSetTechnique(context, state->technique);

	uint primitiveVertexOffset = (uint[]) {
		gScene2D.quadMeshVertexBufferOffset,
	}[state->meshType];

	nuDeviceSetVertexBuffers(context, 0, 2, (NuBufferView[]) {
		gScene2D.primitivesVertexBuffer, primitiveVertexOffset, 0,
		gScene2D.instancesVertexBuffer, command->firstInstanceOffset, 0,
	});

	/* issue draw */
	nuDeviceDrawArrays(context, kMeshPrimitiveType[state->meshType], 0, 4, command->instanceCount);
}

static void ExecuteImmediateCommand(Command* command, NuContext context)
{
	nuBufferUpdate(gScene2D.instancesVertexBuffer, 0, gScene2D.immediateInstanceData, nArrayLen(gScene2D.immediateInstanceData));
	ExecuteCommand(command, context);
	nArrayClear(&gScene2D.immediateInstanceData);
	gScene2D.immediateHasCommand = false;
}

static bool CompatibleDeviceStates(const DeviceState* s1, const DeviceState* s2)
{
	return s1->meshType == s2->meshType &&
		s1->technique == s2->technique &&
		memcmp(&s1->blendState, &s2->blendState, sizeof(NuBlendState)) == 0 &&
		s1->texture == s2->texture;
}

/**
 * Executes a single draw command.
 */
static Command* NewCommand(Scene2D* scene, DeviceState const* deviceState)
{
	EnforceInitialized();
	Command* command = NULL;
	char** instanceData;
	NuAllocator* allocator;

	if (scene) {
		uint n = nArrayLen(scene->commands);
		if (n > 0 && CompatibleDeviceStates(&scene->commands[n - 1].deviceState, deviceState)) {
			return &scene->commands[n - 1];
		}
		command = nArrayPush(&scene->commands, &scene->allocator, Command);
		if (!command) return NULL;

		allocator = &scene->allocator;
		instanceData = &scene->instanceData;
	}
	/* if null, user asks for immediate mode scene */
	else {
		/* if different commands, we need to flush the current one before continuing */
		if (gScene2D.immediateHasCommand && !CompatibleDeviceStates(&gScene2D.immediateCommand.deviceState, deviceState)) {
			ExecuteImmediateCommand(&gScene2D.immediateCommand, gScene2D.immediateContext);
		}
		gScene2D.immediateHasCommand = true;
		command = &gScene2D.immediateCommand;
		allocator = &gScene2D.allocator;
		instanceData = &gScene2D.immediateInstanceData;
	}

	nAssert(command);
	command->deviceState = *deviceState;

	if (!nArrayAlignUp(instanceData, allocator, n_alignof(float))) {
		return NULL;
	}

	command->firstInstanceOffset = nArrayLen(*instanceData);
	command->instanceCount = 0;

	return command;
}

static inline void* NewInstance(NuScene2D scene, MeshType checkMeshType)
{
	EnforceInitialized();
	Command* command = NULL;
	void* instance = NULL;
	if (scene) {
		uint n = nArrayLen(scene->commands);
		nEnforce(n > 0, "No command given for specified Scene2D, you possibly forgot a nu2dBegin*() call?");
		command = &scene->commands[n - 1];
		instance = nArrayPushEx(&scene->instanceData, &scene->allocator, 1, kMeshInstanceSize[checkMeshType]);
	}
	else {
		nEnforce(gScene2D.immediateHasCommand, "No command given for immediate Scene2D, you possibly forgot a nu2dBegin*() call?");
		command = &gScene2D.immediateCommand;
		instance = nArrayPushEx(&gScene2D.immediateInstanceData, &gScene2D.allocator, 1, kMeshInstanceSize[checkMeshType]);
	}
	nEnforce(command->deviceState.meshType == checkMeshType, "Instance 2D pushed on scene not in the correct draw state, current is '%s', instance pushed for draw state '%s'.",
		kMeshTypeStr[command->deviceState.meshType], kMeshTypeStr[checkMeshType]);
	++command->instanceCount;
	return instance;
}

/*-------------------------------------------------------------------------------------------------
 * Internal API
 *-----------------------------------------------------------------------------------------------*/
NuResult nInitScene2D(NuAllocator* allocator)
{
	#define PushVec2(v, allocator, x, y)\
	{\
		float *p = nArrayPushN(v, allocator, float, 2);\
		p[0] = x;\
		p[1] = y;\
	}

	nAssert(!gScene2D.initialized);
	gScene2D.initialized = true;
	gScene2D.allocator = *allocator;

	/* prepare the primitive template vertex buffer */
	float *data = NULL;

	/* quad */
	gScene2D.quadMeshVertexBufferOffset = 0;
	PushVec2(&data, allocator, 0, 0);
	PushVec2(&data, allocator, 1, 0);
	PushVec2(&data, allocator, 0, 1);
	PushVec2(&data, allocator, 1, 1);

	#undef PushVec2

	/* create the primitives vertex buffer */
	NuBufferCreateInfo bufferInfo = {
		.type = NU_BUFFER_TYPE_VERTEX,
		.usage = NU_BUFFER_USAGE_IMMUTABLE,
		.initialSize = nArrayLen(data) * sizeof(float),
		.initialData = data,
	};
	
	NuResult result = nuCreateBuffer(&bufferInfo, allocator, &gScene2D.primitivesVertexBuffer);
	if (result) {
		nDebugError("Could not create scene 2d device primitives vertex buffer.");
		goto error;
	}

	/* create the device instance buffer */
	bufferInfo = (NuBufferCreateInfo) {
		.type = NU_BUFFER_TYPE_VERTEX,
		.usage = NU_BUFFER_USAGE_STREAM,
	};

	result = nuCreateBuffer(&bufferInfo, allocator, &gScene2D.instancesVertexBuffer);
	if (result) {
		nDebugError("Could not create scene 2d device instance vertex buffer.");
		goto error;
	}

	/* create the device constant buffer */
	bufferInfo = (NuBufferCreateInfo) {
		.type = NU_BUFFER_TYPE_CONSTANT,
		.usage = NU_BUFFER_USAGE_DYNAMIC,
	};

	result = nuCreateBuffer(&bufferInfo, allocator, &gScene2D.constantBuffer);
	if (result) {
		nDebugError("Could not create scene 2d device constant buffer.");
		goto error;
	}

	return NU_SUCCESS;

error:
	nArrayFree(data, allocator);
	nDeinitScene2D(allocator);
	return result;
}

void nDeinitScene2D(NuAllocator* allocator)
{
	if (!gScene2D.initialized) return;
	nArrayFree(gScene2D.immediateInstanceData, &gScene2D.allocator);
	nuDestroyBuffer(gScene2D.primitivesVertexBuffer, allocator);
	nuDestroyBuffer(gScene2D.instancesVertexBuffer, allocator);
	nZero(&gScene2D);
}

/*-------------------------------------------------------------------------------------------------
 * Public API
 *-----------------------------------------------------------------------------------------------*/
void nu2dImmediateBegin(Nu2dBeginImmediateInfo const* info)
{
	EnforceInitialized();
	nEnforce(!gScene2D.immediateHasCommand, "Immediate 2D rendering already begun.");
	gScene2D.immediateContext  = info->context;
	gScene2D.immediateViewport = info->viewport;
	SetDeviceViewport(info->context, info->viewport);
}

void nu2dImmediateEnd(void)
{
	EnforceInitialized();
	if (gScene2D.immediateHasCommand) {
		ExecuteImmediateCommand(&gScene2D.immediateCommand, gScene2D.immediateContext);
	}
	nArrayClear(gScene2D.immediateInstanceData);
}

NuResult nuCreateScene2D(NuAllocator* allocator, NuScene2D* ppScene)
{
	EnforceInitialized();
	allocator = nGetDefaultOrAllocator(allocator);
	*ppScene = nNew(Scene2D, allocator);
	Scene2D* scene = *ppScene;
	if (!scene) return NU_ERROR_OUT_OF_MEMORY;
	scene->allocator = *allocator;
	nArrayReserve(&scene->commands, allocator, Command, 10);
	return NU_SUCCESS;
}

void nuDestroyScene2D(NuScene2D scene, NuAllocator* allocator)
{
	EnforceInitialized();
	allocator = nGetDefaultOrAllocator(allocator);
	nArrayFree(scene->commands, allocator);
	nArrayFree(scene->instanceData, allocator);
	nFree(scene, nGetDefaultOrAllocator(allocator));
}

NuResult nu2dReset(NuScene2D scene, NuRect2i viewport)
{
	EnforceInitialized();
	scene->viewport = viewport;
	nArrayClear(scene->commands);
	nArrayClear(scene->instanceData);
	return NU_SUCCESS;
}

void nu2dPresent(NuScene2D scene, NuContext context)
{
	EnforceInitialized();
	uint numCommands = nArrayLen(scene->commands);
	if (numCommands == 0) return;

	SetDeviceViewport(context, scene->viewport);

	/* update the instance buffer */
	nuBufferUpdate(gScene2D.instancesVertexBuffer, 0, scene->instanceData, nArrayLen(scene->instanceData));

	for (uint i = 0; i < numCommands; ++i) {
		ExecuteCommand(scene->commands + i, context);
	}
}

NuResult nu2dBeginQuadsSolid(NuScene2D scene, NuBlendState const* blendState)
{
	DeviceState state = {
		.meshType = MESH_TYPE_QUAD_SOLID,
		.technique = nGetBuiltins()->technique2dQuadSolid,
		.blendState = *blendState,
	};

	Command* command = NewCommand(scene, &state);
	return command ? NU_SUCCESS : NU_ERROR_OUT_OF_MEMORY;
}

NuResult nu2dBeginQuadsTextured(NuScene2D scene, NuBlendState const* blendState, NuTexture texture)
{
	EnforceInitialized();
	DeviceState state = {
		.meshType   = MESH_TYPE_QUAD_TEXTURED,
		.technique  = nGetBuiltins()->technique2dQuadTextured,
		.blendState = *blendState,
		.texture    = texture,
	};

	Command* command = NewCommand(scene, &state);
	return command ? NU_SUCCESS : NU_ERROR_OUT_OF_MEMORY;
}

NuResult nu2dQuadSolid(NuScene2D scene, NuRect2 rect, uint32_t color)
{
	QuadSolid* quad = NewInstance(scene, MESH_TYPE_QUAD_SOLID);
	if (!quad) {
		return NU_ERROR_OUT_OF_MEMORY;
	}
	quad->rect = rect;
	quad->color = nSwizzleUInt(color);
	return NU_SUCCESS;
}

NuResult nu2dQuadSolidEx(NuScene2D scene, NuRect2 rect, uint32_t topLeftColor, uint32_t topRightColor, uint32_t bottomLeftColor, uint32_t bottomRightColor)
{
	EnforceInitialized();
	nEnforce(false, "Unimplemented.");
	return 0;
}

NuResult nu2dQuadTextured(NuScene2D scene, NuRect2 rect, uint32_t color, NuRect2 uvRect, uint textureIndex)
{
	EnforceInitialized();
	QuadTextured* quad = NewInstance(scene, MESH_TYPE_QUAD_TEXTURED);
	if (!quad) {
		return NU_ERROR_OUT_OF_MEMORY;
	}
	quad->rect = rect;
	quad->color = color;
	quad->uvRect = uvRect;
	quad->textureIndex = textureIndex;
	return NU_SUCCESS;
}

NuResult nu2dQuadTexturedEx(NuScene2D scene, NuRect2 rect, uint32_t topLeftColor, uint32_t topRightColor, uint32_t bottomLeftColor, uint32_t bottomRightColor, NuRect2 uvRect, uint textureIndex)
{
	EnforceInitialized();
	nEnforce(false, "Unimplemented.");
	return 0;
}
