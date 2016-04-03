/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#pragma once

#include "nunki/device.h"

typedef struct NBuiltinResources {

	/* vertex layouts */
	NuVertexLayout vertexLayout2dQuadSolid;
	NuVertexLayout vertexLayout2dQuadTextured;

	/* techniques */
	NuTechnique technique2dQuadSolid;
	NuTechnique technique2dQuadTextured;

} NBuiltinResources;

/**
 * Write the #documentation.
 */
void nInitBuiltinResources(NuAllocator* allocator);

/**
 * Write the #documentation.
 */
void nDeinitBuiltinResources(NuAllocator* allocator);

/**
 * Write the #documentation.
 */
NBuiltinResources const* nGetBuiltins(void);
