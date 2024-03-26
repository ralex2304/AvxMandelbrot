#ifndef MANDELBROT_H_
#define MANDELBROT_H_

#include <immintrin.h>

#include "config.h"
#include "utils/statuses.h"
#include "sfml/lib_sfml.h"

inline __m256 _mm256_set_ps1(float v) { // REVIEW
    return _mm256_set_ps(v, v, v, v, v, v, v, v);
}

struct Mandelbrot {

    enum IMPL: int {
        I_DEFAULT     = -1,
        NAIVE         =  0,
        VECTOR_NO_AVX =  1,
        VECTOR_AVX    =  2,

    } impl = I_DEFAULT;

    enum MODE: int {
        M_DEFAULT = -1,
        GRAPH     =  0,
        TEST      =  1,

    } test_mode = M_DEFAULT;

    float x_offs = X_DEFAULT_OFFS;
    float y_offs = Y_DEFAULT_OFFS;

    const float R2 = RADIUS * RADIUS;
    const __m256 R2_vect = _mm256_set_ps1(R2);

    float scale = DEFAULT_SCALE;

    void calculate_naive(sfmlCanvas* canvas);

    void calculate_vector_no_avx(sfmlCanvas* canvas);

    void calculate_vector_avx(sfmlCanvas* canvas);

};

Status::Statuses window_process(Mandelbrot* mb, sfmlWindow* window);

Status::Statuses measure_ticks(Mandelbrot* mb, sfmlCanvas* canvas);

#endif //< #ifndef MANDELBROT_H_
