/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#pragma once

#include "base.h"

NU_HANDLE(NuWindow);

typedef enum {
	NU_WINDOW_STYLE_FIXED,
	NU_WINDOW_STYLE_RESIZABLE,
	NU_WINDOW_STYLE_FULLSCREEN,
} NuWindowStyle;

typedef enum {
	NU_WINDOW_EVENT_TYPE_NONE,
	NU_WINDOW_EVENT_TYPE_CLOSE,
	NU_WINDOW_EVENT_TYPE_KEY,
	NU_WINDOW_EVENT_TYPE_BUTTON,
	NU_WINDOW_EVENT_TYPE_RESIZE,
	NU_WINDOW_EVENT_TYPE_MOUSE_MOVE,
} NuWindowEventType;

typedef enum {
	NU_MOUSE_BUTTON_LEFT,
	NU_MOUSE_BUTTON_MIDDLE,
	NU_MOUSE_BUTTON_RIGHT,
	NU_MOUSE_BUTTON_BUTTON4,
	NU_MOUSE_BUTTON_BUTTON5,
} NuMouseButton;

typedef enum {
	NU_KEY_UNKNOWN = 0XFFFF,
	NU_KEY_SPACE = 32,
	NU_KEY_APOSTROPHE = 39,
	NU_KEY_COMMA = 44,
	NU_KEY_MINUS = 45,
	NU_KEY_PERIOD = 46,
	NU_KEY_SLASH = 47,

	/* PRINTABLE KEYS */
	NU_KEY_0 = 48,
	NU_KEY_1 = 49,
	NU_KEY_2 = 50,
	NU_KEY_3 = 51,
	NU_KEY_4 = 52,
	NU_KEY_5 = 53,
	NU_KEY_6 = 54,
	NU_KEY_7 = 55,
	NU_KEY_8 = 56,
	NU_KEY_9 = 57,
	NU_KEY_SEMICOLON = 59,
	NU_KEY_EQUAL = 61,
	NU_KEY_A = 65,
	NU_KEY_B = 66,
	NU_KEY_C = 67,
	NU_KEY_D = 68,
	NU_KEY_E = 69,
	NU_KEY_F = 70,
	NU_KEY_G = 71,
	NU_KEY_H = 72,
	NU_KEY_I = 73,
	NU_KEY_J = 74,
	NU_KEY_K = 75,
	NU_KEY_L = 76,
	NU_KEY_M = 77,
	NU_KEY_N = 78,
	NU_KEY_O = 79,
	NU_KEY_P = 80,
	NU_KEY_Q = 81,
	NU_KEY_R = 82,
	NU_KEY_S = 83,
	NU_KEY_T = 84,
	NU_KEY_U = 85,
	NU_KEY_V = 86,
	NU_KEY_W = 87,
	NU_KEY_X = 88,
	NU_KEY_Y = 89,
	NU_KEY_Z = 90,
	NU_KEY_LEFTBRACKET = 91,
	NU_KEY_BACKSLASH = 92,
	NU_KEY_RIGHTBRACKET = 93,
	NU_KEY_GRAVEACCENT = 96,
	NU_KEY_WORLD1 = 161,
	NU_KEY_WORLD2 = 162,

	/* FUNCTION KEYS */
	NU_KEY_ESCAPE = 256,
	NU_KEY_ENTER = 257,
	NU_KEY_TAB = 258,
	NU_KEY_BACKSPACE = 259,
	NU_KEY_INSERT = 260,
	NU_KEY_DELETE = 261,
	NU_KEY_RIGHT = 262,
	NU_KEY_LEFT = 263,
	NU_KEY_DOWN = 264,
	NU_KEY_UP = 265,
	NU_KEY_PAGEUP = 266,
	NU_KEY_PAGEDOWN = 267,
	NU_KEY_HOME = 268,
	NU_KEY_END = 269,
	NU_KEY_CAPSLOCK = 280,
	NU_KEY_SCROLLLOCK = 281,
	NU_KEY_NUMLOCK = 282,
	NU_KEY_PRINTSCREEN = 283,
	NU_KEY_PAUSE = 284,
	NU_KEY_F1 = 290,
	NU_KEY_F2 = 291,
	NU_KEY_F3 = 292,
	NU_KEY_F4 = 293,
	NU_KEY_F5 = 294,
	NU_KEY_F6 = 295,
	NU_KEY_F7 = 296,
	NU_KEY_F8 = 297,
	NU_KEY_F9 = 298,
	NU_KEY_F10 = 299,
	NU_KEY_F11 = 300,
	NU_KEY_F12 = 301,
	NU_KEY_F13 = 302,
	NU_KEY_F14 = 303,
	NU_KEY_F15 = 304,
	NU_KEY_F16 = 305,
	NU_KEY_F17 = 306,
	NU_KEY_F18 = 307,
	NU_KEY_F19 = 308,
	NU_KEY_F20 = 309,
	NU_KEY_F21 = 310,
	NU_KEY_F22 = 311,
	NU_KEY_F23 = 312,
	NU_KEY_F24 = 313,
	NU_KEY_NUMPAD0 = 320,
	NU_KEY_NUMPAD1 = 321,
	NU_KEY_NUMPAD2 = 322,
	NU_KEY_NUMPAD3 = 323,
	NU_KEY_NUMPAD4 = 324,
	NU_KEY_NUMPAD5 = 325,
	NU_KEY_NUMPAD6 = 326,
	NU_KEY_NUMPAD7 = 327,
	NU_KEY_NUMPAD8 = 328,
	NU_KEY_NUMPAD9 = 329,
	NU_KEY_NUMPADDOT = 330,
	NU_KEY_NUMPADSLASH = 331,
	NU_KEY_NUMPADASTERISK = 332,
	NU_KEY_NUMPADMINUS = 333,
	NU_KEY_NUMPADPLUS = 334,
	NU_KEY_NUMPADENTER = 335,
	NU_KEY_LEFTSHIFT = 340,
	NU_KEY_LEFTCTRL = 341,
	NU_KEY_LEFTALT = 342,
	NU_KEY_LEFTSUPER = 343,
	NU_KEY_RIGHTSHIFT = 344,
	NU_KEY_RIGHTCTRL = 345,
	NU_KEY_RIGHTALT = 346,
	NU_KEY_RIGHTSUPER = 347,
	NU_KEY_MENU = 348,
} NuKey;

typedef struct {
	const char *title;
	unsigned width;
	unsigned height;
	NuWindowStyle style;
} NuWindowCreateInfo;

typedef struct {
	NuWindowEventType type;

	union {
		struct {
			uint isDown : 1;
			uint repetitions;
			NuKey code;
			uint scancode;
			uint isAnyShiftDown : 1;
			uint isAnyCtrlDown : 1;
			uint isAnyAltDown : 1;
		} key;

		struct {
			uint isDown : 1;
			NuMouseButton button;
			int position[2];
		} mouseButton;

		struct {
			int relativePosition[2];
			int position[2];
		} mouseMotion;

		struct {
			uint width, height;
		} size;
	};
} NuWindowEvent;

/**
 * Write the #documentation.
 */
NUNKI_API NuResult nuCreateWindow(NuWindowCreateInfo const* info, NuAllocator* allocator, NuWindow* window);

/**
 * Write the #documentation.
 */
NUNKI_API void nuDestroyWindow(NuWindow window, NuAllocator* allocator);

/**
 * Write the #documentation.
 */
NUNKI_API void nuWindowSetTitle(NuWindow window, const char* title);

/**
 * Write the #documentation.
 */
NUNKI_API void nuWindowShowCursor(NuWindow window, bool show);

/**
 * Write the #documentation.
 */
NUNKI_API bool nuWindowPollEvents(NuWindow window, NuWindowEvent *e);

/**
 * Write the #documentation.
 */
NUNKI_API NuSize2i nuWindowGetSize(NuWindow window);

/**
 * Write the #documentation.
 */
NUNKI_API void* nuWindowGetNativeHandle(NuWindow window);
