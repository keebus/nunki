/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#version 330

uniform cbScene2D {
	mat4 transform; // #todo we probably don't need a full matrix here
} scene2d;

layout(location = 0) in vec2 avPosition;
layout(location = 1) in vec4 aiBounds;
layout(location = 2) in vec4 aiColor;
layout(location = 1) in vec4 aiUvRect;
layout(location = 2) in vec4 aiTextureIndex;

flat out vec4 vColor;
out vec3 vUV;

void main()
{
	vColor = aiColor;
	vUV = vec3(aiRect.xy + aiRect.zw * avPosition, aiTextureIndex);
	vec2 position = aiBounds.xy + aiBounds.zw * avPosition;
	gl_Position = scene2d.transform * vec4(position, 0, 1);
}
