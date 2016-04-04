/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include "nunki/image.h"
#include "nu_libs.h"

typedef struct NuImageImpl {
	NuImageFormat format;
	NuSize2i      size;
	uint          byteSize;
	char          data[];
} Image;

static const kPixelSize[NU_IMAGE_FORMAT_COUNT_] = {
	/* NU_IMAGE_FORMAT_RGBA8 */ 4,
};

NuResult nuCreateImage(NuImageCreateInfo const* info, NuAllocator* allocator, NuImage* ppImage)
{
	allocator = nGetDefaultOrAllocator(allocator);
	*ppImage = NULL;

	/* compute total image size */
	uint byteSize = kPixelSize[info->format] * info->size.width * info->size.height;

	Image* image = n_malloc(sizeof(Image) + byteSize, allocator);
	if (!image) { return NU_ERROR_OUT_OF_MEMORY; }

	image->format = info->format;
	image->size = info->size;
	image->byteSize = byteSize;

	if (info->initializeMemory) {
		memset(image->data, 0, byteSize);
	}

	*ppImage = image;
	return NU_SUCCESS;
}

void nuDestroyImage(NuImage image, NuAllocator* allocator)
{
	if (!image) return;
	n_free(image, allocator);
}

NuImageView nuImageGetView(NuImage const image)
{
	return (NuImageView) {
		image->format,
		image->size,
		image->data,
	};
}

void* nuImageGetWritableDataPtr(NuImage image)
{
	return image->data;
}
