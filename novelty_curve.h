#pragma once
#include "common.h"
#include "moving_average.h"


extern void calculate_novelty_curve(SAMPLE * mag_spec,int speclen, int max_frame, int hop_size, SAMPLE* novelty);
