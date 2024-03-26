#include "mandelbrot.h"

#ifdef VOLATILE
static volatile uint32_t measure_result_global = 0;
#endif

inline uint32_t count_color_rgba_(unsigned int n) {
    n = 255 - n;

    uint32_t res = 0xFFu << 24u; //< A channel

    res += n << 16u;
    res += ((n % 8u) * 32u) << 8u;
    res += (n % 2u) * 255u;

    return res;
}

void Mandelbrot::calculate_naive(sfmlCanvas* canvas) {
    assert(canvas);

    size_t n = 0;

    float y0 = y_offs * DEFAULT_SCALE - scale * HEIGHT / 2;

    for (size_t iy = 0; iy < canvas->height(); iy++, y0 += scale) {

        float x0 = x_offs * DEFAULT_SCALE - scale * WIDTH / 2;


        for (size_t jx = 0; jx < canvas->width(); jx++, x0 += scale) {

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

#ifdef VOLATILE
            measure_result_global = count_color_rgba_((unsigned char)(cnt - 1));
#else
            canvas->set_pixel_color_raw(n, count_color_rgba_((unsigned char)(cnt - 1)));
#endif
            n += 4;
        }
    }
}

void Mandelbrot::calculate_vector_no_avx(sfmlCanvas* canvas) {
    assert(canvas);

    size_t n = 0;

    float y0 = y_offs * DEFAULT_SCALE - scale * HEIGHT / 2;

    for (size_t iy = 0; iy < canvas->height(); iy++, y0 += scale) {

        float x0_base = x_offs * DEFAULT_SCALE - scale * WIDTH / 2;

        for (size_t jx = 0; jx < canvas->width(); jx += 8, x0_base += 8 * scale) {

            float x0[8] = {x0_base + 0 * scale, x0_base + 1 * scale,
                           x0_base + 2 * scale, x0_base + 3 * scale,
                           x0_base + 4 * scale, x0_base + 5 * scale,
                           x0_base + 6 * scale, x0_base + 7 * scale};

            float x[8] = {}; for (int i = 0; i < 8; i++) x[i] = x0[i];
            float y[8] = {}; for (int i = 0; i < 8; i++) y[i] = y0;

            unsigned int cnt = 0;
            unsigned int cnts[8] = {};

            while (cnt++ < MAX_ITER - 1) {
                float x2[8] = {}; for (int i = 0; i < 8; i++) x2[i] = x[i] * x[i];
                float y2[8] = {}; for (int i = 0; i < 8; i++) y2[i] = y[i] * y[i];

                float dist2[8] = {}; for (int i = 0; i < 8; i++) dist2[i] = x2[i] + y2[i];

                unsigned int cmp[8] = {};
                unsigned int mask = 0;
                for (int i = 0; i < 8; i++) cmp[i] = (dist2[i] <= R2);
                for (int i = 0; i < 8; i++) mask |= cmp[i];

                if (!mask)
                    break;

                for (int i = 0; i < 8; i++) cnts[i] += cmp[i];

                for (int i = 0; i < 8; i++) y[i] = 2 * x[i] * y[i] + y0;
                for (int i = 0; i < 8; i++) x[i] = x2[i] - y2[i] + x0[i];
            }

            for (int i = 0; i < 8; i++) {
#ifdef VOLATILE
                measure_result_global = count_color_rgba_((unsigned char)(cnts[i]));
#else
                canvas->set_pixel_color_raw(n, count_color_rgba_((unsigned char)(cnts[i])));
#endif
                n += 4;
            }
        }
    }
}

void Mandelbrot::calculate_vector_avx(sfmlCanvas* canvas) {
    assert(canvas);

    size_t n = 0;

    __m256 y0 = _mm256_set_ps1(y_offs * DEFAULT_SCALE - scale * HEIGHT / 2);

    for (size_t iy = 0; iy < canvas->height(); iy++, y0 += scale) {

        float x0_base = x_offs * DEFAULT_SCALE - scale * WIDTH / 2;

        for (size_t jx = 0; jx < canvas->width(); jx += 8, x0_base += 8 * scale) {

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

            alignas(32) int cnts_int[8] = {};
            _mm256_store_si256((__m256i*)&cnts_int, cnts); // REVIEW
            //int* cnts_int = (int*)&cnts;

            for (int i = 0; i < 8; i++) {
#ifdef VOLATILE
                measure_result_global = count_color_rgba_((unsigned char)(cnts_int[i]));
#else
                canvas->set_pixel_color_raw(n, count_color_rgba_((unsigned char)(cnts_int[i])));
#endif
                n += 4;
            }
        }
    }
}

inline unsigned long measure_ticks_naive_(Mandelbrot* mb, sfmlCanvas* canvas) {
    assert(mb);
    assert(canvas);

    unsigned long lo = 0;
    unsigned long hi = 0;
    asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    unsigned long begin = lo + (hi << 32);

    for (size_t cnt = 0; cnt < MEASURE_CNT; cnt++)
        mb->calculate_naive(canvas);

    asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    unsigned long end = lo + (hi << 32);

    return (end - begin) / MEASURE_CNT;
}

inline unsigned long measure_ticks_vector_no_avx_(Mandelbrot* mb, sfmlCanvas* canvas) {
    assert(mb);
    assert(canvas);

    unsigned long lo = 0;
    unsigned long hi = 0;
    asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    unsigned long begin = lo + (hi << 32);

    for (size_t cnt = 0; cnt < MEASURE_CNT; cnt++)
        mb->calculate_vector_no_avx(canvas);

    asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    unsigned long end = lo + (hi << 32);

    return (end - begin) / MEASURE_CNT;
}

inline unsigned long measure_ticks_vector_avx_(Mandelbrot* mb, sfmlCanvas* canvas) {
    assert(mb);
    assert(canvas);

    unsigned long lo = 0;
    unsigned long hi = 0;
    asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    unsigned long begin = lo + (hi << 32);

    for (size_t cnt = 0; cnt < MEASURE_CNT; cnt++)
        mb->calculate_vector_avx(canvas);

    asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    unsigned long end = lo + (hi << 32);

    return (end - begin) / MEASURE_CNT;
}

Status::Statuses measure_ticks(Mandelbrot* mb, sfmlCanvas* canvas) {
    assert(mb);
    assert(canvas);

    unsigned long res = 0;

    switch (mb->impl) {
        case Mandelbrot::NAIVE:
            res = measure_ticks_naive_(mb, canvas);
            break;

        case Mandelbrot::VECTOR_NO_AVX:
            res = measure_ticks_vector_no_avx_(mb, canvas);
            break;

        case Mandelbrot::VECTOR_AVX:
            res = measure_ticks_vector_avx_(mb, canvas);
            break;

        case Mandelbrot::I_DEFAULT:
        default:
            return Status::ARGS_ERROR;
    }

    printf("%ld\n", res);

    return Status::NORMAL_WORK;
}

Status::Statuses window_process(Mandelbrot* mb, sfmlWindow* window) {
    assert(mb);
    assert(window);

    while (window->is_open()) {

        switch (mb->impl) {
            case Mandelbrot::NAIVE:
                mb->calculate_naive(window->canvas());
                break;

            case Mandelbrot::VECTOR_NO_AVX:
                mb->calculate_vector_no_avx(window->canvas());
                break;

            case Mandelbrot::VECTOR_AVX:
                mb->calculate_vector_avx(window->canvas());
                break;

            case Mandelbrot::I_DEFAULT:
            default:
                fprintf(stderr, "Wrong or no mode specified");
                return Status::ARGS_ERROR;
        }

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
                            mb->scale /= SCALING_STEP;
                            break;
                        case sf::Keyboard::Scan::NumpadMinus:
                        case sf::Keyboard::Scan::Hyphen:
                                mb->scale *= SCALING_STEP;
                            break;

                        case sf::Keyboard::Scan::Right:
                            mb->x_offs += MOVE_STEP * mb->scale / DEFAULT_SCALE;
                            break;
                        case sf::Keyboard::Scan::Left:
                            mb->x_offs -= MOVE_STEP * mb->scale / DEFAULT_SCALE;
                            break;
                        case sf::Keyboard::Scan::Up:
                            mb->y_offs -= MOVE_STEP * mb->scale / DEFAULT_SCALE;
                            break;
                        case sf::Keyboard::Scan::Down:
                            mb->y_offs += MOVE_STEP * mb->scale / DEFAULT_SCALE;
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

    return Status::NORMAL_WORK;
}
