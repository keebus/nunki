/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#pragma once

#include "nu_libs.h"
#include "thirdparty/gl3w.h"

static inline GLenum BufferTypeToGl(NuBufferType type)
{
	return (GLenum[]) { GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_UNIFORM_BUFFER }[type];
}

static inline GLenum BufferUsageToGl(NuBufferUsage usage)
{
	return (GLenum[]) { GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW }[usage];
}

static inline uint GetAttributeTypeSize(NuVertexAttributeType type)
{
	switch (type) {
		case NU_VAT_INT8: case NU_VAT_UINT8: case NU_VAT_SNORM8: case NU_VAT_UNORM8: return 1;
		case NU_VAT_INT16: case NU_VAT_UINT16: case NU_VAT_SNORM16: case NU_VAT_UNORM16: return 2;
		case NU_VAT_INT32: case NU_VAT_UINT32: case NU_VAT_SNORM32: case NU_VAT_UNORM32:  return 4;
		case NU_VAT_FLOAT: return 4;
	}
	nDebugBreak();
	return 0;
}

static inline void UnpackVertexLayoutAttributeTypeToGl(NuVertexAttributeType type, GLenum *gl_type, bool *normalized, bool *integer)
{
	switch (type) {
		case NU_VAT_INT8:  *gl_type = GL_BYTE; *normalized = false; *integer = true; return;
		case NU_VAT_UINT8: *gl_type = GL_UNSIGNED_BYTE; *normalized = false; *integer = true; return;
		case NU_VAT_SNORM8: *gl_type = GL_BYTE; *normalized = true; *integer = false; return;
		case NU_VAT_UNORM8: *gl_type = GL_UNSIGNED_BYTE; *normalized = true; *integer = false; return;

		case NU_VAT_INT16:  *gl_type = GL_SHORT; *normalized = false; *integer = true; return;
		case NU_VAT_UINT16: *gl_type = GL_UNSIGNED_SHORT; *normalized = false; *integer = true; return;
		case NU_VAT_SNORM16: *gl_type = GL_SHORT; *normalized = true; *integer = false; return;
		case NU_VAT_UNORM16: *gl_type = GL_UNSIGNED_SHORT; *normalized = true; *integer = false; return;

		case NU_VAT_INT32:  *gl_type = GL_INT; *normalized = false; *integer = true; return;
		case NU_VAT_UINT32: *gl_type = GL_UNSIGNED_INT; *normalized = false; *integer = true; return;
		case NU_VAT_SNORM32: *gl_type = GL_INT; *normalized = true; *integer = false; return;
		case NU_VAT_UNORM32: *gl_type = GL_UNSIGNED_INT; *normalized = true; *integer = false; return;

		case NU_VAT_FLOAT: *gl_type = GL_FLOAT; *normalized = false; *integer = false; return;
	}
}

static const GLenum kGlPrimitiveType[] = {
	GL_POINTS, GL_LINES, GL_LINE_LOOP, GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP, GL_TRIANGLES,
};

static const GLenum kGlTextureType[] = {
	GL_TEXTURE_1D,
	GL_TEXTURE_2D,
	GL_TEXTURE_3D,
	GL_TEXTURE_1D_ARRAY,
	GL_TEXTURE_2D_ARRAY
};

static const kGlTextureInternalFormat[] = {
	GL_R8,
	GL_RG8,
	GL_RGB8,
	GL_RGBA8,
};

static void ImageFormatToGl(NuImageFormat format, GLenum* pixelFormat, GLenum* pixelType)
{
	switch (format) {
		case NU_IMAGE_FORMAT_R8: *pixelFormat = GL_RED; *pixelType = GL_UNSIGNED_BYTE; return;
		case NU_IMAGE_FORMAT_R8G8: *pixelFormat = GL_RG; *pixelType = GL_UNSIGNED_BYTE; return;
		case NU_IMAGE_FORMAT_R8G8B8: *pixelFormat = GL_RGB; *pixelType = GL_UNSIGNED_BYTE; return;
		case NU_IMAGE_FORMAT_R8G8B8A8: *pixelFormat = GL_RGBA; *pixelType = GL_UNSIGNED_BYTE; return;
		default: nAssert(false);
	}
}

static const float kGlSamplerFilter[] = {
	GL_NEAREST,
	GL_LINEAR,
	GL_NEAREST_MIPMAP_NEAREST,
	GL_LINEAR_MIPMAP_LINEAR,
};

static const float kGlSamplerWrapMode[] = {
	GL_CLAMP_TO_EDGE,
	GL_REPEAT,
};
