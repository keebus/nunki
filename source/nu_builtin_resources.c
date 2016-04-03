/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include "nu_builtin_resources.h"
#include "nu_libs.h"
#include "nu_shaders.h"

static const char *vertexLayoutErrorMessage = "Could not create builtin vertex layout %s.";
static const char *techniqueErrorMessage = "Could not create builtin vertex layout %s.";

static NBuiltinResources gBuiltins;

static void CompileTechnique(const char* name, NuTechniqueCreateInfo* info, NuTechnique* pTechnique)
{
	NuTechniqueCreateResult result = nuCreateTechnique(info, pTechnique);

	static const char* errorMsgs[] = {
		"Error compiling builtin vertex shader",
		"Error compiling builtin geometry shader",
		"Error compiling builtin fragment shader",
		"Error linking builtin shader",
	};

	nEnforce(result == NU_TECHNIQUE_CREATE_SUCCESS, "%s %s:\n%s", errorMsgs[result - 1], name, info->errorMessageBuffer);
}

#define LoadVertexLayout(name, alloc)\
		nEnforce(!nuCreateVertexLayout(&desc, alloc, &gBuiltins.vertexLayout##name), vertexLayoutErrorMessage, #name)

/*-------------------------------------------------------------------------------------------------
 * Loading functions
 *-----------------------------------------------------------------------------------------------*/
static void CreateVertexLayouts(NuAllocator* allocator)
{
	NuVertexLayoutDesc desc;
	desc.numStreams = 2;
	desc.streams = (NuVertexStreamDesc[]) { false, true };

	desc.numAttributes = 3;
	desc.attributes = (NuVertexAttributeDesc[]) {
		0, NU_VAT_FLOAT, 2,		/* quad normalized 2d pos */
		1, NU_VAT_FLOAT, 4,		/* instance 2d bounds */
		1, NU_VAT_UNORM8, 4,	/* instance color */
	};
	LoadVertexLayout(2dQuadSolid, allocator);

	desc.numAttributes = 5;
	desc.attributes = (NuVertexAttributeDesc[]) {
		0, NU_VAT_FLOAT,	2,		/* quad normalized 2d pos */
		1, NU_VAT_FLOAT,	4,		/* instance 2d bounds */
		1, NU_VAT_UNORM8,	4,		/* instance color */
		1, NU_VAT_FLOAT,	4,		/* instance 2d uv rect */
		1, NU_VAT_UINT32,	1,		/* instance texture index */
	};
	LoadVertexLayout(2dQuadTextured, allocator);
}

static void CreateTechniques(void)
{
	#define MSG_BUFFER_SIZE 2048
	char msgBuffer[MSG_BUFFER_SIZE];

	NuTechniqueCreateInfo info2d = {
		.errorMessageBuffer = msgBuffer,
		.errorMessageBufferSize = MSG_BUFFER_SIZE,
		.constantBuffers = (const char*[]) { "cbScene2D", NULL },
	};

	/* textured quad 2d solid */
	info2d.layout = gBuiltins.vertexLayout2dQuadSolid;
	info2d.vertexShaderSource = N_SHADER_SRC_2D_QUAD_SOLID_VERT;
	info2d.fragmentShaderSource = N_SHADER_SRC_2D_QUAD_SOLID_FRAG;
	CompileTechnique("SolidQuad2D", &info2d, &gBuiltins.technique2dQuadSolid);

	/* textured quad 2d tetured */
	info2d.layout = gBuiltins.vertexLayout2dQuadTextured;
	info2d.vertexShaderSource = N_SHADER_SRC_2D_QUAD_TEXTURED_VERT;
	info2d.fragmentShaderSource = N_SHADER_SRC_2D_QUAD_TEXTURED_FRAG;
	info2d.samplers = (const char*[]) { "sTexture", NULL };
	CompileTechnique("TexturedQuad2D", &info2d, &gBuiltins.technique2dQuadTextured);
}


void nInitBuiltinResources(NuAllocator* allocator)
{
	CreateVertexLayouts(allocator);
	CreateTechniques();
}

void nDeinitBuiltinResources(NuAllocator* allocator)
{
	nuDestroyVertexLayout(gBuiltins.vertexLayout2dQuadSolid, allocator);
}

NBuiltinResources const* nGetBuiltins(void)
{
	return &gBuiltins;
}
