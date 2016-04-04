newaction {
	trigger = "embedglsl",
	description = "Generates files nu_shader.h and nu_shader.c containing shader files in source/shaders.",

	onStart = function()
		shader_h = io.open("source/nu_shaders.h", "w")
		shader_c = io.open("source/nu_shaders.c", "w")

		shader_h:write([[
/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#pragma once

]])

		shader_c:write([[
/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include "nu_shaders.h"

]])
		local shaders = os.matchfiles("source/shaders/*.glsl")
		for i, shader in pairs(shaders) do
			local shader_id = "N_SHADER_SRC_"..string.upper(path.getbasename(shader))
			shader_h:write(string.format("extern const char* %s;\n", shader_id))
			shader_c:write(string.format("const char* %s = ", shader_id))
			shader_file = io.open(shader)
			local anyline = false
			for line in shader_file:lines() do
				anyline = true
				if not first then
					first = false
					shader_c:write("\n")	
				end
				shader_c:write('\t\t"' .. line .. '\\n"')
			end
			if anyline then
				shader_c:write(";\n\n")
			else
				shader_c:write('"";\n')
			end
			shader_file:close()
		end
	end,

	onEnd = function()
		shader_h:close()
		shader_c:close()
	end
}

solution "Nunki"
	language "C"
	location "build"
	startproject "NunkiSample"
	configurations { "Debug", "Optimized", "Release" }
	platforms { "x64" }
	characterset "MBCS"

	configuration { "Debug", "x64" };		targetdir("bin/debug64"); debugdir("bin/debug64")
	configuration { "Optimized", "x64"}; 	targetdir("bin/optimized64"); debugdir("bin/optimized64")
	configuration { "Release", "x64" };		targetdir("bin/release64"); debugdir("bin/release64")
	configuration { "Debug", "x32" };		targetdir("bin/debug32"); debugdir("bin/debug32")
	configuration { "Optimized", "x32"};	targetdir("bin/optimized32"); debugdir("bin/optimized32")
	configuration { "Release", "x32" };		targetdir("bin/release32"); debugdir("bin/release32")

	filter { "configurations:Debug" }
		defines { "DEBUG" }
		flags { "Symbols" }

	filter { "configurations:Optimized" }
		defines { "DEBUG" }
		flags { "Symbols" }
		optimize "Speed"

	filter { "configurations:Release" }
		optimize "Speed"

	configuration "vs*"
		buildoptions {
			"/wd4201", -- nameless struct/union"
			"/wd4204", -- non-constant aggregate initializer
			"/we4013", -- undefined; assuming extern returning int (treat as error)
			"/we4133", -- incompatible types
			"/we4028", -- formal parameter different from declaration
			"/we4047", -- indirection level
		}
	
	project "Nunki"
		kind "SharedLib"
		defines "NUNKI_EXPORT"
		targetname "nunki"
		includedirs "include"
		files { "include/**.h", "source/**.h", "source/**.inl", "source/**.c", "source/**.glsl" }
		links { "freetype261" }
		configuration "windows"; links { "opengl32", "glu32" }
		configuration {}

	project "NunkiSample"
		kind "WindowedApp"
		links "Nunki"
		files { "sample/source/**.c", "sample/source/**.h" }
		includedirs "include"
