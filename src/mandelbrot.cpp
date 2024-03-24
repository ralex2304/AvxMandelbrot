#include "mandelbrot.h"

inline uint32_t count_color_rgba_(unsigned char n) {
    n = 255 - n;

    uint32_t res = 0xFF << 24; //< A channel

    res += n << 16;
    res += ((n % 8) * 32) << 8;
    res += (n % 2) * 255;

    return res;
}

inline void Mandelbrot::calculate_naive(sfmlWindow* window) {
    assert(window);

    size_t n = 0;

    float y0 = y_offs * DEFAULT_SCALE - scale * HEIGHT / 2;

    for (size_t iy = 0; iy < window->height(); iy++, y0 += scale) {

        float x0 = x_offs * DEFAULT_SCALE - scale * WIDTH / 2;


        for (size_t jx = 0; jx < window->width(); jx++, x0 += scale) {

            float x = x0;
            float y = y0;

            uint32_t cnt = 0;

            while (cnt++ < MAX_ITER - 1) {
                float x2 = x * x;
                float y2 = y * y;

                if (x2 + y2 > R2)
                    break;

                y = 2 * x * y + y0;
                x = x2 - y2 + x0;
            }

            window->set_pixel_color_raw(n, count_color_rgba_((unsigned char)(cnt - 1)));

            n += 4;
        }
    }
}

inline void Mandelbrot::calculate_vector_no_sse(sfmlWindow* window) {
    assert(window);

    size_t n = 0;

    float y0 = y_offs * DEFAULT_SCALE - scale * HEIGHT / 2;

    for (size_t iy = 0; iy < window->height(); iy++, y0 += scale) {

        float x0_base = x_offs * DEFAULT_SCALE - scale * WIDTH / 2;

        for (size_t jx = 0; jx < window->width(); jx += 8, x0_base += 8 * scale) {

            float x0[8] = {x0_base + 0 * scale, x0_base + 1 * scale,
                           x0_base + 2 * scale, x0_base + 3 * scale,
                           x0_base + 4 * scale, x0_base + 5 * scale,
                           x0_base + 6 * scale, x0_base + 7 * scale};

            float x[8] = {}; for (int i = 0; i < 8; i++) x[i] = x0[i];
            float y[8] = {}; for (int i = 0; i < 8; i++) y[i] = y0;

            uint32_t cnt = 0;
            uint32_t cnts[8] = {};

            while (cnt++ < MAX_ITER - 1) {
                float x2[8] = {}; for (int i = 0; i < 8; i++) x2[i] = x[i] * x[i];
                float y2[8] = {}; for (int i = 0; i < 8; i++) y2[i] = y[i] * y[i];

                bool cmp = false;
                for (int i = 0; i < 8; i++) {
                    if (x2[i] + y2[i] <= R2) {
                        cnts[i] = cnt;
                        cmp = true;
                    }
                }
                if (!cmp)
                    break;

                for (int i = 0; i < 8; i++) {
                    y[i] = 2 * x[i] * y[i] + y0;
                    x[i] = x2[i] - y2[i] + x0[i];
                }
            }

            for (int i = 0; i < 8; i++)
                window->set_pixel_color_raw(n += 4, count_color_rgba_((unsigned char)(cnts[i])));
        }
    }
}

inline void Mandelbrot::calculate_vector_sse(sfmlWindow* window) {
    assert(window);

    size_t n = 0;

    __m256 y0 = _mm256_set_ps1(y_offs * DEFAULT_SCALE - scale * HEIGHT / 2);

    for (size_t iy = 0; iy < window->height(); iy++, y0 += scale) {

        float x0_base = x_offs * DEFAULT_SCALE - scale * WIDTH / 2;

        for (size_t jx = 0; jx < window->width(); jx += 8, x0_base += 8 * scale) {

            // x0[0:7] = x0_base + scale * {0 - 7}
            __m256 x0 = _mm256_add_ps(_mm256_set_ps1(x0_base),
                                      _mm256_mul_ps(_mm256_set_ps1(scale),
                                                    _mm256_set_ps(7, 6, 5, 4, 3, 2, 1, 0)));

            __m256 x = x0;
            __m256 y = y0;

            uint32_t cnt = 0;

            __m256i cnts = _mm256_setzero_si256();

            while (cnt++ < MAX_ITER - 1) {
                __m256 x2 = _mm256_mul_ps(x, x);
                __m256 y2 = _mm256_mul_ps(y, y);

                // x2[0:7] + y2[0:7] <= R2[0:7]
                __m256 cmp = _mm256_cmp_ps(_mm256_add_ps(x2, y2) , R2_vect, _CMP_LE_OQ);

                int mask = _mm256_movemask_ps(cmp);
                if (!mask)
                    break;

                cnts = _mm256_sub_epi32(cnts, _mm256_castps_si256(cmp));

                // y[0:7] = 2 * x[0:7] * y[0:7] + y0[0:7]
                __m256 xy = _mm256_mul_ps(x, y);
                y = _mm256_add_ps(_mm256_add_ps(xy, xy), y0);

                // x[0:7] = x2[0:7] - y2[0:7] + x0[0:7]
                x = _mm256_add_ps(_mm256_sub_ps(x2, y2), x0);
            }

            int* cnts_int = (int*)&cnts;

            for (int i = 0; i < 8; i++)
                window->set_pixel_color_raw(n += 4, count_color_rgba_((unsigned char)(cnts_int[i])));
        }
    }
}

Status::Statuses measure_ticks(Mandelbrot* mb, sfmlWindow* window) {
    assert(mb);
    assert(window);

    printf("Measuring...\n");
    fflush(stdout);

    unsigned long lo = 0;
    unsigned long hi = 0;
    asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    unsigned long begin = lo + (hi << 32);

    for (size_t cnt = 0; cnt < MEASURE_CNT; cnt++) {

#if defined NAIVE
        mb->calculate_naive(window);
#elif defined VECTORIZED_NO_SSE
        mb->calculate_vector_no_sse(window);
#elif defined VECTORIZED_SSE
        mb->calculate_vector_sse(window);
#endif
    }

    asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    unsigned long end = lo + (hi << 32);

    unsigned long result = (end - begin) / MEASURE_CNT;

    printf("%ld\n", result);

    return Status::NORMAL_WORK;
}

Status::Statuses process(sfmlWindow* window) {
    assert(window);

    Mandelbrot mb = {};

    measure_ticks(&mb, window);

    /*

    while (window->is_open()) {

#if defined NAIVE
        mb.calculate_naive(window);
#elif defined VECTORIZED_NO_SSE
        mb.calculate_vector_no_sse(window);
#elif defined VECTORIZED_SSE
        mb.calculate_vector_sse(window);
#endif

        window->renew();

        while (window->events_left()) {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
            switch (window->event.type) {

                case sf::Event::Closed:
                    window->close();
                    break;

                case sf::Event::KeyPressed:

                    switch (window->event.key.scancode) {

                        case sf::Keyboard::Scan::NumpadPlus:
                        case sf::Keyboard::Scan::Equal:
                            mb.scale /= SCALING_STEP;
                            break;
                        case sf::Keyboard::Scan::NumpadMinus:
                        case sf::Keyboard::Scan::Hyphen:
                                mb.scale *= SCALING_STEP;
                            break;

                        case sf::Keyboard::Scan::Right:
                            mb.x_offs += MOVE_STEP * mb.scale / DEFAULT_SCALE;
                            break;
                        case sf::Keyboard::Scan::Left:
                            mb.x_offs -= MOVE_STEP * mb.scale / DEFAULT_SCALE;
                            break;
                        case sf::Keyboard::Scan::Up:
                            mb.y_offs -= MOVE_STEP * mb.scale / DEFAULT_SCALE;
                            break;
                        case sf::Keyboard::Scan::Down:
                            mb.y_offs += MOVE_STEP * mb.scale / DEFAULT_SCALE;
                            break;

                        default:
                            break;
                    }

                    break;

                default:
                    break;
            }
#pragma GCC diagnostic pop
        }
    }
    */

    return Status::NORMAL_WORK;
}
