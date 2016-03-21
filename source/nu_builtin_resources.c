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

static void CreateVertexLayouts(NuAllocator* allocator)
{
	NuVertexLayoutDesc desc;
	desc.numStreams = 2;
	desc.streams = (NuVertexStreamDesc[]) { false, true };

#define LoadVertexLayout(name, alloc)\
	nEnforce(!nuCreateVertexLayout(&desc, alloc, &gBuiltins.vertexLayout##name), vertexLayoutErrorMessage, #name)

	desc.numAttributes = 3;
	desc.attributes = (NuVertexAttributeDesc[]) {
		0, NU_VAT_FLOAT, 2, /* quad normalized 2d pos */
		1, NU_VAT_FLOAT, 4, /* instance 2d bounds */
		1, NU_VAT_UNORM8, 4, /* instance color */
	};
	
	LoadVertexLayout(QuadInstance, allocator);
}

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

static void CreateTechniques(void)
{
	#define MSG_BUFFER_SIZE 2048
	char msgBuffer[MSG_BUFFER_SIZE];

	/* textured quad 2d */
	{
		NuTechniqueCreateInfo info = {
			.errorMessageBuffer = msgBuffer,
			.errorMessageBufferSize = MSG_BUFFER_SIZE,

			.layout = gBuiltins.vertexLayoutQuadInstance,
			.vertexShaderSource = N_SHADER_SRC_SOLID_QUAD_2D_VERT,
			.fragmentShaderSource = N_SHADER_SRC_SOLID_QUAD_2D_FRAG,
			.constantBuffers = (const char*[]) { "cbScene2D", NULL },
		};	

		CompileTechnique("TexturedQuad2D", &info, &gBuiltins.techniqueSolidQuad2D);
	}
}

void nInitBuiltinResources(NuAllocator* allocator)
{
	CreateVertexLayouts(allocator);
	CreateTechniques();
}

void nDeinitBuiltinResources(NuAllocator* allocator)
{
	nuDestroyVertexLayout(gBuiltins.vertexLayoutQuadInstance, allocator);
}

NBuiltinResources const* nGetBuiltins(void)
{
	return &gBuiltins;
}
