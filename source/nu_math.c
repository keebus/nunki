/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include "nu_math.h"
#include "nu_libs.h"

void nOrtho(float left, float top, float right, float bottom, float near, float far, float* m)
{
	nAssert(far != near);

	const float rightMinusLeft = right - left;
	const float topMinusBottom = top - bottom;
	const float farMinusNear = far - near;

	m[0] = 2 / rightMinusLeft;
	m[1] = m[2] = m[3] = m[4] = 0.0f;
	m[5] = 2 / topMinusBottom;
	m[6] = m[7] = m[8] = m[9] = 0.0f;
	m[10] = -2 / farMinusNear;
	m[11] = 0;
	m[12] = -(right + left) / rightMinusLeft;
	m[13] = -(top + bottom) / topMinusBottom;
	m[14] = -(far + near) / farMinusNear;
	m[15] = 1.0f;
}
