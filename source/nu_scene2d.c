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
typedef struct {
	NuRect2 rect;
	uint    color;
} SolidQuad;

typedef enum {
	CMD_SET_BLEND_STATE,
	CMD_DRAW,
} CommandType;

typedef enum {
	PRIMITIVE_SOLID_QUAD
} PrimitiveType;

typedef struct {
	PrimitiveType primitiveType;
	uint firstInstanceOffset;
	uint instanceCount;
} CommandDraw;

typedef struct {
	CommandType type;
	union {
		CommandDraw draw;
		NuBlendState blendState;
	};
} Command;

typedef struct NuScene2DImpl {
	NuAllocator        allocator;
	NuScene2DResetInfo beginInfo;
	Command*           commands;
	char*              instanceData;
} Scene2D;

typedef struct {
	NuTechnique technique;
	uint primitiveVertexOffset;
	uint instanceSize;
	NuPrimitiveType devicePrimitiveType;
} PrimitiveInfo;

typedef struct {
	float transform[16];
} Constants;

static struct {
	bool initialized;
	uint primitiveVertexBufferOffsetToQuad;
	NuBuffer constantBuffer;
	NuBuffer primitivesVertexBuffer;
	NuBuffer instancesVertexBuffer;
} gScene2D;

/*-------------------------------------------------------------------------------------------------
 * Static functions
 *-----------------------------------------------------------------------------------------------*/
#define EnforceInitialized() nEnforce(gScene2D.initialized, "Scene 2D module uninitialized.");

inline void PushVec2( float **v, NuAllocator* allocator, float x, float y)
{
	float *p = nArrayPushN(v, allocator, float, 2);
	p[0] = x;
	p[1] = y;
}

static inline PrimitiveInfo GetPrimitiveInfo(PrimitiveType type)
{
	switch (type) {
		case PRIMITIVE_SOLID_QUAD:
			return (PrimitiveInfo) { nGetBuiltins()->techniqueSolidQuad2D, gScene2D.primitiveVertexBufferOffsetToQuad, sizeof(SolidQuad), NU_PRIMITIVE_TRIANGLE_STRIP };
	}
	nAssert(false);
	return (PrimitiveInfo) { 0 };
}

/* State functions */
static Command* SetStateChangeDeviceState(NuScene2D scene, CommandType cmdType)
{
	/* fetch last command and check whether it matches command type */
	uint numCommands = nArrayLen(scene->commands);
	Command *command = NULL;

	if (numCommands > 0) {
		command = &scene->commands[numCommands - 1];

		/* check command state matches quad drawing command */
		if (command->type != cmdType) {
			command = NULL;
		}
	}

	/* if last command is not appropriate or there simply are no commands in scene yet make one */
	if (command == NULL) {
		command = nArrayPush(&scene->commands, &scene->allocator, Command);
		*command = (Command) {
			.type = cmdType,
		};
	}

	return command;
}

static Command* SetStateQuadSolid(NuScene2D scene)
{
	/* fetch last command and make sure it matches quads rendering */
	uint numCommands = nArrayLen(scene->commands);
	Command *command = NULL;

	if (numCommands > 0) {
		command = &scene->commands[numCommands - 1];

		/* check command state matches quad drawing command */
		if (command->type != CMD_DRAW || command->draw.primitiveType != PRIMITIVE_SOLID_QUAD) {
			command = NULL;
		}
	}

	/* if last command is not appropriate or there simply are no commands in scene yet make one */
	if (command == NULL) {
		nArrayAlignUp(&scene->instanceData, &scene->allocator, n_alignof(SolidQuad));
		
		command = nArrayPush(&scene->commands, &scene->allocator, Command);
		*command = (Command) {
			.type = CMD_DRAW,
			.draw = (CommandDraw) {
				.primitiveType = PRIMITIVE_SOLID_QUAD,
				.firstInstanceOffset = nArrayLen(scene->instanceData),
			}
		};
	}

	return command;
}

/*-------------------------------------------------------------------------------------------------
 * Internal API
 *-----------------------------------------------------------------------------------------------*/
NuResult nInitScene2D(NuAllocator* allocator)
{
	nAssert(!gScene2D.initialized);
	gScene2D.initialized = true;

	/* prepare the primitive template vertex buffer */
	float *data = NULL;

	/* quad */
	gScene2D.primitiveVertexBufferOffsetToQuad = 0;
	PushVec2(&data, allocator, 0, 0);
	PushVec2(&data, allocator, 1, 0);
	PushVec2(&data, allocator, 0, 1);
	PushVec2(&data, allocator, 1, 1);

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
	nuDestroyBuffer(gScene2D.primitivesVertexBuffer, allocator);
	nuDestroyBuffer(gScene2D.instancesVertexBuffer, allocator);
	nZero(&gScene2D);
}

/*-------------------------------------------------------------------------------------------------
 * Public API
 *-----------------------------------------------------------------------------------------------*/
NuResult nuCreateScene2D(NuAllocator* allocator, NuScene2D* scene)
{
	allocator = nGetAllocator(allocator);
	*scene = nNew(Scene2D, allocator);
	if (!*scene) return NU_ERROR_OUT_OF_MEMORY;
	(*scene)->allocator = *allocator;
	return NU_SUCCESS;
}

void nuDestroyScene2D(NuScene2D scene, NuAllocator* allocator)
{
	nArrayFree(scene->commands, allocator);
	nArrayFree(scene->instanceData, allocator);
	nFree(scene, nGetAllocator(allocator));
}

NuResult nu2dReset(NuScene2D scene, NuScene2DResetInfo const* info)
{
	scene->beginInfo = *info;
	nArrayClear(scene->commands);
	nArrayClear(scene->instanceData);
	return NU_SUCCESS;
}

void nu2dPresent(NuScene2D scene)
{
	uint numCommands = nArrayLen(scene->commands);
	if (numCommands == 0) return;

	NuContext context = scene->beginInfo.context;

	/* update the constant buffer */
	Constants constants;
	NuRect2i  viewport = scene->beginInfo.bounds;
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

	/* update the instance buffer */
	nuBufferUpdate(gScene2D.instancesVertexBuffer, 0, scene->instanceData, nArrayLen(scene->instanceData));

	for (uint i = 0; i < numCommands; ++i) {
		Command* command = &scene->commands[i];

		switch (command->type) {
			case CMD_SET_BLEND_STATE:
			{
				nuDeviceSetBlendState(context, &command->blendState);
				break;
			}

			case CMD_DRAW:
			{
				CommandDraw* draw = &command->draw;
				PrimitiveInfo primitiveInfo = GetPrimitiveInfo(draw->primitiveType);

				/* set device state */
				nuDeviceSetTechnique(context, primitiveInfo.technique);

				nuDeviceSetVertexBuffers(context, 0, 2, (NuBufferView[]) {
					gScene2D.primitivesVertexBuffer, primitiveInfo.primitiveVertexOffset, 0,
						gScene2D.instancesVertexBuffer, draw->firstInstanceOffset, 0,
				});

				/* issue draw */
				nuDeviceDrawArrays(context, primitiveInfo.devicePrimitiveType, 0, 4, draw->instanceCount);
				break;
			}
		}
	
	}
}

void nu2dSetBlendState(NuScene2D scene, NuBlendState const* blendState)
{
	Command* command = SetStateChangeDeviceState(scene, CMD_SET_BLEND_STATE);
	command->blendState = *blendState;
}

void nu2dDrawQuadSolidFlat(NuScene2D scene, NuRect2 rect, uint32_t color)
{
	/* #todo add culling here.  */

	Command* command = SetStateQuadSolid(scene);

	/* push the quad instance */
	++command->draw.instanceCount;
	SolidQuad *instance = nArrayPushEx(&scene->instanceData, &scene->allocator, 1, sizeof(SolidQuad));
	*instance = (SolidQuad) { rect, nSwizzleUInt(color) };
}
