/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#pragma once

#include "./base.h"
#include "./device.h"

NU_HANDLE(NuScene2D);

typedef struct {
	uint isImmediate : 1;
} NuScene2DCreateInfo;

typedef struct {
	NuContext context;
	NuRect2i  bounds;
	uint      enableScissors : 1;
} NuScene2DResetInfo;

/**
 * Write the #documentation.
 */
NUNKI_API NuResult nuCreateScene2D(NuScene2DCreateInfo const* info, NuAllocator* allocator, NuScene2D* scene);

/**
 * Write the #documentation.
 */
NUNKI_API void nuDestroyScene2D(NuScene2D scene, NuAllocator* allocator);

/**
 * Write the #documentation.
 */
NUNKI_API NuResult nu2dReset(NuScene2D scene, NuScene2DResetInfo const* info);

/**
 * Write the #documentation.
 */
NUNKI_API void nu2dPresent(NuScene2D scene);

/**
 * Write the #documentation.
 */
NUNKI_API void nu2dSetBlendState(NuScene2D scene, NuBlendState const* blendState);

/**
 * Write the #documentation.
 */
NUNKI_API void nu2dDrawQuadSolidFlat(NuScene2D scene, NuRect2 rect, uint32_t color);

