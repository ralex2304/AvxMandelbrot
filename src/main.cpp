#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "utils/statuses.h"
#include "sfml/lib_sfml.h"
#include "mandelbrot.h"

static Status::Statuses parse_args_(int argc, char* argv[], Mandelbrot* mb);

static void print_help_();

int main(int argc, char* argv[]) {

    sfmlCanvas canvas = {};

    if (!canvas.ctor(WIDTH, HEIGHT))
        return Status::raise(Status::MEMORY_EXCEED);

    Mandelbrot mb = {};

    STATUS_CHECK_RAISE(parse_args_(argc, argv, &mb));

    if (mb.test_mode == Mandelbrot::TEST) {
        STATUS_CHECK_RAISE(measure_ticks(&mb, &canvas), canvas.dtor());

        canvas.dtor();

        return Status::NORMAL_WORK;
    }

    if (mb.test_mode != Mandelbrot::GRAPH)
        return Status::raise(Status::ARGS_ERROR);

    sfmlWindow window = {};
    if (!window.ctor(WIDTH, HEIGHT, HEADER, &canvas))
        return Status::raise(Status::MEMORY_EXCEED);

    STATUS_CHECK_RAISE(window_process(&mb, &window), canvas.dtor();
                                                     window.dtor());

    canvas.dtor();
    window.dtor();

    return Status::OK_EXIT;
}

static Status::Statuses parse_args_(int argc, char* argv[], Mandelbrot* mb) {
    assert(argv);
    assert(mb);

    if (argc == 1) {
        mb->test_mode = Mandelbrot::GRAPH;
        mb->impl = Mandelbrot::VECTOR_AVX;
        return Status::NORMAL_WORK;
    }

    if (strcmp(argv[1], "-h") == 0) {
        print_help_();
        return Status::OK_EXIT;
    }

    if (argc == 3) {
        sscanf(argv[1], "%d", (int*)&mb->test_mode);

        sscanf(argv[2], "%d", (int*)&mb->impl);

        return Status::NORMAL_WORK;
    }

    return Status::ARGS_ERROR;
}

static void print_help_() {
    printf("# Mandelbrot set benchmark\n"
           "# You must specify working mode and implementation in cmd args:\n"
           "#     0 - display mode\n"
           "#     1 - test mode\n"
           "#\n"
           "#     0 - Naive\n"
           "#     1 - Vectorized no avx\n"
           "#     2 - Vectorized avx\n"
           "# Example: ./main 1 0\n"
           "# Graphics controls:\n"
           "#     +/-    - scale\n"
           "#     arrows - movement\n");
}
