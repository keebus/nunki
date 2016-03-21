/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include "nu_shaders.h"

const char* N_SHADER_SRC_SOLID_QUAD_2D_FRAG = 
		"/*\n"
		" * Yume (simple rendering engine)\n"
		" * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.\n"
		" * For licensing info see LICENSE.\n"
		" */\n"
		"\n"
		"#version 330\n"
		"\n"
		"flat in vec4 vColor;\n"
		"\n"
		"out vec4 fFragColor;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	fFragColor = vColor;\n"
		"}\n";

const char* N_SHADER_SRC_SOLID_QUAD_2D_VERT = 
		"/*\n"
		" * Yume (simple rendering engine)\n"
		" * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.\n"
		" * For licensing info see LICENSE.\n"
		" */\n"
		"\n"
		"#version 330\n"
		"\n"
		"uniform cbScene2D {\n"
		"	mat4 transform; // #todo we probably don't need a full matrix here\n"
		"} scene2d;\n"
		"\n"
		"layout(location = 0) in vec2 avPosition;\n"
		"layout(location = 1) in vec4 aiBounds;\n"
		"layout(location = 2) in vec4 aiColor;\n"
		"\n"
		"flat out vec4 vColor;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	vColor = aiColor;\n"
		"	vec2 position = aiBounds.xy + aiBounds.zw * avPosition;\n"
		"	gl_Position = scene2d.transform * vec4(position, 0, 1);\n"
		"}\n";

