#pragma once
#include "common.h"
#include "window.h"
typedef struct
{
	int speclen;
	int hop_size;
	enum WINDOW_FUNCTION wfunc;
	fftw_plan plan;

	fftw_complex *time_domain;
	SAMPLE *window;
	fftw_complex *freq_domain;
	SAMPLE *mag_spec;

	SAMPLE *data;
	int max_frame;
	float min_freq;
	float max_freq;
} spectrum;


extern int calc_spec_len(SAMPLE fft_freq);

extern spectrum * create_spectrum (SAMPLE * data, int max_frame, int speclen, int hop_size, enum WINDOW_FUNCTION window_function, float min_freq, float max_freq);

extern void destroy_spectrum (spectrum * spec);

extern SAMPLE calc_magnitude_spectrum (spectrum * spec, int type);
