#pragma once
#include "common.h"
#include <assert.h>
#include <cairo.h>
#include "window.h"
#include "spectrum.h"

#define TICK_LEN			6
#define	BORDER_LINE_WIDTH	1.8

#define	TITLE_FONT_SIZE		20.0
#define	NORMAL_FONT_SIZE	12.0

#define	LEFT_BORDER			70.0
#define	TOP_BORDER			30.0
#define	RIGHT_BORDER		75.0
#define	BOTTOM_BORDER		40.0

#define	SPEC_FLOOR_DB		-180.0


typedef struct
{
	const char *pngfilepath;
	int width, height;
	bool border, log_freq, gray_scale;
	SAMPLE min_freq, max_freq, fft_freq;
	enum WINDOW_FUNCTION window_function;
	SAMPLE spec_floor_db;
  	paTestData data;
} RENDER;

typedef struct
{
	int left, top, width, height;
} RECT;

extern void render_spectogram (RENDER * render);
