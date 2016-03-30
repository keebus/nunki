/*
 * Yume (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#version 330

flat in vec4 vColor;

out vec4 fFragColor;

void main()
{
	fFragColor = vColor;
}
