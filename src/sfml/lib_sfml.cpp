#include <SFML/Graphics.hpp>

#include "lib_sfml.h"

bool sfmlWindow::ctor(const unsigned int width, const unsigned int height, const char* header) {
    assert(width > 0);
    assert(height > 0);
    assert(header);

    if ((_window = new(std::nothrow) sf::RenderWindow(sf::VideoMode(
                                        (unsigned int)width, (unsigned int)height), header)) == nullptr)
        return false;

    _window->setPosition(sf::Vector2i(100, 100));

    if (!_canvas.ctor(width, height))
        return false;

#ifdef SHOW_FPS

    if (!_font.loadFromFile(FONT)) {
        fprintf(stderr, "lib_sfml: font not found! \n");
        return false;
    }

    _fps_text.setFont(_font);
    _fps_text.setFillColor(TEXT_COLOR);
    _fps_text.setCharacterSize(TEXT_SIZE);
#endif

    renew();

    return true;
}

void sfmlWindow::renew() {
    _window->clear();

    _canvas.renew();

    _window->draw(_canvas.sprite());

#ifdef SHOW_FPS
    _show_fps();
#endif

    _window->display();
}

void sfmlWindow::dtor() {
    _canvas.dtor();
    delete _window;
}

#ifdef SHOW_FPS

void sfmlWindow::_show_fps() {
    static size_t last_time = 0;

    size_t cur_time = time_microseconds();

    snprintf(_fps_text_buf, _FPS_TEXT_BUF_SIZE, "%ld", 1000'000 / (cur_time - last_time));

    last_time = cur_time;

    _fps_text.setString(_fps_text_buf);
    _window->draw(_fps_text);
}

#endif //< #ifdef SHOW_FPS
