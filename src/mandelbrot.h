#ifndef MANDELBROT_H_
#define MANDELBROT_H_

#include <immintrin.h>

#include "config.h"
#include "utils/statuses.h"
#include "sfml/lib_sfml.h"

inline __m256 _mm256_set_ps1(float v) { // REVIEW
    return _mm256_set_ps(v, v, v, v, v, v, v, v);
}

Status::Statuses process(sfmlWindow* window);

struct Mandelbrot {

    float x_offs = X_DEFAULT_OFFS;
    float y_offs = Y_DEFAULT_OFFS;

    const float R2 = RADIUS * RADIUS;
    const __m256 R2_vect = _mm256_set_ps1(R2);

    float scale = DEFAULT_SCALE;

    inline void calculate_naive(sfmlWindow* window);

    inline void calculate_vector_no_sse(sfmlWindow* window);

    inline void calculate_vector_sse(sfmlWindow* window);

};

Status::Statuses measure_ticks(Mandelbrot* mb, sfmlWindow* window);

#endif //< #ifndef MANDELBROT_H_
