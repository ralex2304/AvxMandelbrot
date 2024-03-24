
#include "config.h"
#include "utils/statuses.h"
#include "sfml/lib_sfml.h"
#include "mandelbrot.h"

int main() {

    sfmlWindow window;
    if (!window.ctor(WIDTH, HEIGHT, HEADER))
        return Status::raise(Status::MEMORY_EXCEED);

    STATUS_CHECK_RAISE(process(&window), window.dtor());

    window.dtor();

    return Status::OK_EXIT;
}
