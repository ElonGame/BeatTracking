#pragma once
#include "common.h"
enum WINDOW_FUNCTION { RECTANGULAR = 0, KAISER = 1, NUTTALL = 2, HANN = 3 };

extern void calc_kaiser_window (SAMPLE * data, int datalen, SAMPLE beta);

extern void calc_nuttall_window (SAMPLE * data, int datalen);

extern void calc_hann_window (SAMPLE * data, int datalen);
