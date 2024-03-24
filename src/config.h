#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdlib.h>

#define NAIVE
//#define VECTORIZED_NO_SSE
//#define VECTORIZED_SSE

const size_t MEASURE_CNT = 100;

const size_t WIDTH  = 1600;
const size_t HEIGHT = 900;
const char* const HEADER = "Mandelbrot";

const size_t MAX_ITER = 256;

const float DEFAULT_SCALE = 1.0f / (float)WIDTH;
const float SCALING_STEP = 1.1f;
const float MOVE_STEP = 100.0f;

const float X_DEFAULT_OFFS = -1.5f * (float)WIDTH;
const float Y_DEFAULT_OFFS = 0.0f;

const float RADIUS = 100.0f;


#endif //< CONFIG_H_
