/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include "nunki/font.h"
#include "nu_device.h"
#include "nu_libs.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>
#include "nu_font.h"

typedef struct GlyphPage
{
	struct GlyphPage* next;
	uint fromCodePoint;
	uint toCodePoint;
	NuFaceGlyph glyphs[];
} GlyphPage;

typedef struct {
	NuFaceMeasures measures;
	GlyphPage*  firstPage;
} Face;

typedef struct NuFontImpl {
	NuTexture   texture;
	size_t      numFaces;
	Face        faces[];
} Font;

typedef struct {
	Face* face;
	NuFaceGlyph* glyph;
	uint  code;
	NuPoint2i advance;
	NuPoint2i offset;
	NuSize2i  size;
	FT_BitmapGlyph bitmap_glyph;
} TempGlyphData;

static struct {
	FT_Library ftLibrary;
} gFont;

/*-------------------------------------------------------------------------------------------------
 * helper functions
 *-----------------------------------------------------------------------------------------------*/
static inline CompareTempGlyphData(const void* vlhs, const void* vrhs)
{
	TempGlyphData const* lhs = vlhs;
	TempGlyphData const* rhs = vrhs;
	if (lhs->size.height == rhs->size.height)
		return rhs->size.width - lhs->size.width;
	else
		return rhs->size.height - lhs->size.height;
}

static inline NuFaceGlyph const* FetchGlyph(const Font* font, uint faceId, uint code)
{
	nAssert(faceId < font->numFaces);
	GlyphPage const* page = font->faces[faceId].firstPage;
	while (page) {
		if (code >= page->fromCodePoint && code <= page->toCodePoint) {
			return &page->glyphs[code - page->fromCodePoint];
		}
	}
	return NULL;
}

/*-------------------------------------------------------------------------------------------------
 * API
 *-----------------------------------------------------------------------------------------------*/
NuResult nInitFontModule(void)
{
	/* create And initialize the FreeType Font Library */
	if (FT_Init_FreeType(&gFont.ftLibrary)) {
		nDebugError("Cannot initialize the freetype library.");
		return NU_FAILURE;
	}

	return NU_SUCCESS;
}

void nDeinitFontModule(void)
{
	FT_Done_FreeType(gFont.ftLibrary);
}

NuTexture nFontGetTexture(NuFont font)
{
	nEnforce(font, "Invalid font provided.");
	return font->texture;
}

NuResult nuCreateFont(NuFontCreateInfo const * info, NuAllocator* allocator, NuTempAllocator _tempAllocator, NuFont * ppFont)
{
	typedef struct Section
	{
		struct Section* next;
		NuPoint2i p;
		NuSize2i s;
	} Section;

	/* preamble */
	NuResult result = NU_SUCCESS;
	allocator = nGetDefaultOrAllocator(allocator);
	NuAllocator* tempAllocator = nAllocatorFromTemp(_tempAllocator);
	*ppFont = NULL;

	/* create the font */
	Font* pFont = n_malloc(sizeof(Font) + sizeof(Face) * info->numFaces, allocator);
	if (!pFont) {
		return NU_ERROR_OUT_OF_MEMORY;
	}

	pFont->numFaces = info->numFaces;

	/* count total number of glyphs */
	uint totNumGlyphs = 0;
	for (uint i = 0; i < info->numFaces; ++i)
	for (uint j = 0; j < info->faces[i].numCharSets; ++j) {
		totNumGlyphs += info->faces[i].charSets[j].toCodePoint- info->faces[i].charSets[j].fromCodePoint + 1;
	}

	TempGlyphData* glyphs = n_newarray(TempGlyphData, totNumGlyphs, tempAllocator);
	uint glyphIndex = 0;

	size_t tempBufferSize = 1024 * 100;
	void* faceBytes = n_malloc(tempBufferSize, tempAllocator);
	nTempAllocatorLockMalloc();

	for (uint i = 0; i < info->numFaces; ++i) {
		NuFaceInfo const* faceInfo = &info->faces[i];
		Face* face = &pFont->faces[i];

		/* load face file bytes */
		size_t faceByteSize = info->faceLoader(faceBytes, tempBufferSize, faceInfo->userData, info->faceLoaderUserData);
		while (faceByteSize > tempBufferSize) {
			n_realloc(faceBytes, faceByteSize, tempAllocator);
			tempBufferSize = faceByteSize;
			faceByteSize = info->faceLoader(faceBytes, tempBufferSize, faceInfo->userData, info->faceLoaderUserData);
		}

		if (faceBytes == 0) {
			continue;
		}

		/* create a new font face. */
		FT_Face ftFace;
		if (FT_New_Memory_Face(gFont.ftLibrary, faceBytes, (FT_Long)faceByteSize, 0, &ftFace)) {
			nDebugError("Error while loading font face %d.", (face - pFont->faces) + 1);
			n_free(faceBytes, tempAllocator);
			continue;
		}

		/* setup the font size. For some reason, freetype takes char sizes in 1/64th of pixel */
		auto adjustedSize = faceInfo->size * 64;
		FT_Set_Char_Size(ftFace, adjustedSize, adjustedSize, info->resolution, info->resolution);

		/* get font data from freetype */
		face->measures.ascender = ftFace->size->metrics.ascender / 64;
		face->measures.descender = ftFace->size->metrics.descender / 64;
		face->measures.lineHeight = ftFace->size->metrics.height / 64;

		GlyphPage* lastPage = NULL;

		// Load each glyph in the charmap
		for (uint j = 0; j <faceInfo->numCharSets; ++j) {
			NuCharSet const* charSet = &faceInfo->charSets[j];

			/* create a glyph page for current face */
			GlyphPage* page = n_malloc(sizeof(GlyphPage) + sizeof(NuFaceGlyph) * (charSet->toCodePoint - charSet->fromCodePoint + 1), allocator);
			if (lastPage) {
				lastPage->next = page;
				lastPage = page;
			}
			else {
				face->firstPage = page;
			}

			page->next = NULL;
			page->fromCodePoint = charSet->fromCodePoint;
			page->toCodePoint = charSet->toCodePoint;

			for (uint ch = charSet->fromCodePoint; ch <= charSet->toCodePoint; ++ch) {
				if (FT_Load_Char(ftFace, ch, FT_LOAD_RENDER)) {
					nDebugWarning("Error loading freetype character with code %c.", ch);
					continue;
				}

				NuPoint2i advance = { ftFace->glyph->advance.x / 64, ftFace->glyph->advance.y / 64 };
				NuPoint2i bitmapPos = { ftFace->glyph->bitmap_left, ftFace->glyph->bitmap_top };

				// The +1 is for creating an empty border so that no leaks appear during rendering.
				NuSize2i size = { ftFace->glyph->bitmap.width + 1, ftFace->glyph->bitmap.rows + 1 };

				// Get the glyph face into a glyph
				FT_Glyph glyph;
				if (FT_Get_Glyph(ftFace->glyph, &glyph)) {
					nDebugError("Could not get glyph of character with code %c.", ch);
					continue;
				}

				// Push the temp glyph
				FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;
				nAssert(glyphIndex < totNumGlyphs);
				glyphs[glyphIndex++] = (TempGlyphData) {
					face,
					&page->glyphs[ch - charSet->fromCodePoint],
					ch,
					advance,
					bitmapPos,
					size,
					bitmapGlyph
				};
			}
		}

		/* destroy the face, we don't need it anymore */
		FT_Done_Face(ftFace);
	}

	n_free(faceBytes, tempAllocator);
	nTempAllocatorUnlockMalloc();

	/* all needed glyphs are stored into glyph. Sort this list by height and then by width */
	qsort(glyphs, totNumGlyphs, sizeof(TempGlyphData), CompareTempGlyphData);

	/* render each glyph in the atlas */
	uint8_t* imagedata = n_malloc(info->textureWidth * info->textureHeight, tempAllocator);
	
	Section* sections = n_new(Section, tempAllocator);
	Section* lastSection = sections;
	sections->s = (NuSize2i) { info->textureWidth, info->textureHeight };

	for (uint j = 0, n = totNumGlyphs; j < n; ++j) {
		TempGlyphData* glyph = &glyphs[j];

	  // Find a suitable section.
		Section* suitableSection = sections;
		Section* prevSection = NULL;
		while (suitableSection) {
			if (suitableSection->s.width >= glyph->size.width && suitableSection->s.height >= glyph->size.height)
				break;
			prevSection = suitableSection;
			suitableSection = suitableSection->next;
		}

		if (!suitableSection) {
			nDebugWarning("Could not place glyph during atlas generation. Not enough space.");
			continue;
		}

		// Create a new font glyph data instance
		NuFaceGlyph* savedGlyph = glyph->glyph;
		savedGlyph->size.width = glyph->bitmap_glyph->bitmap.width;
		savedGlyph->size.height = glyph->bitmap_glyph->bitmap.rows;
		savedGlyph->offset = glyph->offset;
		savedGlyph->advance = glyph->advance.x;
		savedGlyph->textureRect.position = (NuPoint2) { (float)suitableSection->p.x / info->textureWidth,  (float)suitableSection->p.y / info->textureHeight };
		savedGlyph->textureRect.size = (NuSize2) { (float)savedGlyph->size.width / info->textureWidth,  (float)savedGlyph->size.height / info->textureHeight };

		// Blit the bitmap in the atlas buffer
		for (int j = 0; j < glyph->bitmap_glyph->bitmap.rows; j++) 
		for (int i = 0; i < glyph->bitmap_glyph->bitmap.width; i++) {
			imagedata[i + suitableSection->p.x + (j + suitableSection->p.y) * info->textureWidth] = glyph->bitmap_glyph->bitmap.buffer[i + glyph->bitmap_glyph->bitmap.width * j];
		}

		  // In order to reduce unused space, if the remaining section has a higher height than this glyph,
		  // split the section into two horizontally.
		if (suitableSection->s.height >= glyph->size.height * 2) {
			Section* newSection = n_new(Section, tempAllocator);
			*newSection = *suitableSection;
			
			suitableSection->s.height = glyph->size.height;
			
			newSection->p.y += glyph->size.height;
			newSection->s.height -= glyph->size.height;

			lastSection->next = newSection;
			lastSection = newSection;
			lastSection->next = NULL;
		}

		// Remove the space occupied by this glyph from the current section. If no space left, delete it.
		suitableSection->s.width -= glyph->size.width;
		if (suitableSection->s.width <= 0) {
		  /* section is full, remove it */
			if (prevSection) {
				prevSection->next = suitableSection->next;
			}
			else {
				sections = suitableSection->next;
			}
		}
		else {
			suitableSection->p.x += glyph->size.width;
		}
	}

	/* finally create the atlas texture */
	nuCreateTexture(&(NuTextureCreateInfo) {
		NU_TEXTURE_TYPE_2D,
		{ info->textureWidth, info->textureHeight, 1 },
		NU_TEXTURE_FORMAT_R8_UNORM,
	}, allocator, &pFont->texture);

	nuTextureUpdateLevels(pFont->texture, 0, 1, &(NuImageView) {
		.format = NU_IMAGE_FORMAT_R8,
		.size = { info->textureWidth, info->textureHeight },
		.data = imagedata,
	});

	*ppFont = pFont;
	goto cleanup;

cleanup:
	// Clear the bitmap glyphs
	for (size_t i = 0, n = totNumGlyphs; i < n; ++i)
		FT_Done_Glyph((FT_Glyph)glyphs[i].bitmap_glyph);

	return result;
}

void nuDestroyFont(NuFont font, NuAllocator * allocator)
{
	if (!font) return;
	allocator = nGetDefaultOrAllocator(allocator);
	nuDestroyTexture(font->texture, allocator);
	
	/* destroy face glyph pages */
	for (size_t i = 0; i < font->numFaces; ++i)
	for (GlyphPage* page = font->faces[i].firstPage, *next = page->next; page; page = next) {
		n_free(page, allocator);
	}

	n_free(font, allocator);
}

NuTexture nuFontGetTexture(NuFont fontSet)
{
	return fontSet->texture;
}

NuSize2i nuFontProcessText(NuFont fontSet, const char* text, NuTextStyle const* styles, uint initialStyleId, NuFontProcessCharCallback callback, void* userdata)
{
	static const uint kMarkupStackSize = 16;
	NuTextStyle const* markups[16];
	markups[0] = &styles[initialStyleId];
	uint sp = 0; /* stack pointer */
	const Face* initialFace = &fontSet->faces[markups[0]->faceId];

	NuPoint2i pen = { 0 };
	pen.y += fontSet->faces[markups[0]->faceId].measures.ascender;

	/* #TODO use a utf-8 iterator instead! */
	for (; *text; ++text) {
		uint32_t code = *text;

		switch (code) {
			case '\n':
				pen.x = 0;
				pen.y += initialFace->measures.lineHeight;
				continue;

			case '<':
				switch (*++text) {
					case '/':
						++text;
						--sp;
						nEnforce(sp < kMarkupStackSize, "Markup stack pointer over markup stack size.");
						break;

					default:
					{
						char* end;
						uint styleIndex = strtoul(text, &end, 10);
						text = end;
						++sp;
						nEnforce(sp < kMarkupStackSize, "Markup stack pointer over markup stack size.");
						if (styleIndex >= 10) {
							nDebugWarning("Markup style index greater than 9 during text parsing.");
						}
						else {
							markups[sp] = &styles[styleIndex];
						}
						break;
					}
				}

				if (*text != '>') {
					nDebugWarning("Invalid font rendering string:\n%s\n expected enclosing >.", text);
					pen.y += initialFace->measures.lineHeight - initialFace->measures.ascender;
					return (NuSize2i){ pen.x, pen.y };
				}

				continue;

			default:
				break;
		}

		const NuFaceGlyph* glyph = FetchGlyph(fontSet, markups[sp]->faceId, code);

		if (glyph == NULL) {
			nDebugWarning("Glyph with char code %d not found in font with id %d.", code, markups[sp]->faceId);
			continue;
		}

		callback(
			markups[sp],
			(NuRect2i) { pen.x + glyph->offset.x, pen.y - glyph->offset.y, glyph->size.width, glyph->size.height },
			glyph->textureRect,
			userdata);

		// Move the pen forward.
		pen.x += (short)glyph->advance;
	}
	pen.y += initialFace->measures.lineHeight - initialFace->measures.ascender;
	return (NuSize2i){ pen.x, pen.y };
}

