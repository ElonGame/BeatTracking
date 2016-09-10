#pragma once
#include "common.h"
typedef struct
{	int speclen ;
	enum WINDOW_FUNCTION wfunc ;
	fftwf_plan plan ;

	SAMPLE *time_domain ;
	SAMPLE *window ;
	SAMPLE *freq_domain ;
	SAMPLE *mag_spec ;

	SAMPLE data [] ;
} spectrum ;


extern spectrum * create_spectrum (int speclen, enum WINDOW_FUNCTION window_function) ;

extern void destroy_spectrum (spectrum * spec) ;

extern SAMPLE calc_magnitude_spectrum (spectrum * spec) ;
