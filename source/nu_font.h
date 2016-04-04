/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#pragma once

#include "nunki/font.h"

/**
 * Write the #documentation.
 */
NuResult nInitFontModule(void);

/**
 * Write the #documentation.
 */
void nDeinitFontModule(void);

/**
 * @returns the font atlas texture.
 */
NuTexture nFontGetTexture(NuFont font);