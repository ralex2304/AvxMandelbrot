#ifndef LIB_SFML_H_
#define LIB_SFML_H_

#include <SFML/Graphics.hpp>
#include <time.h>

#include "../utils/macros.h"
#include "../utils/statuses.h"

#define SHOW_FPS
const char* const FONT = "/usr/share/fonts/noto/NotoSans-Regular.ttf";
const sf::Color TEXT_COLOR = sf::Color::White;
const unsigned int TEXT_SIZE = 24;

struct sfmlCanvas {
    public:
        bool ctor(const size_t width, const size_t height) {
            _width  = width;
            _height = height;

            if ((_pixels = (sf::Uint8*)calloc(width * height, 4)) == nullptr)
                return false;

            if (!_texture.create((unsigned int)width, (unsigned int)height))
                return false;

            renew();

            _sprite.setTexture(_texture);

            return true;
        }

        void dtor() { FREE(_pixels); }

        inline void set_pixel_color(const int x, const int y, const uint32_t color) {
            memcpy(_pixels + (size_t)y * _width + (size_t)x, &color, sizeof(uint32_t));
        }

        inline void set_pixel_color_by_number(const size_t i, const uint32_t color) {
            memcpy(_pixels + i * sizeof(uint32_t), &color, sizeof(uint32_t));
        }

        inline void set_pixel_color_raw(const size_t i, const uint32_t color) {
            memcpy(_pixels + i, &color, sizeof(uint32_t));
        }

        void renew() { _texture.update(_pixels); }

        inline size_t width()  { return _width; }
        inline size_t height() { return _height; }
        inline sf::Sprite sprite() { return _sprite; }

    private:
        sf::Uint8* _pixels = nullptr;
        sf::Texture _texture = {};
        sf::Sprite _sprite = {};

        size_t _width  = 0;
        size_t _height = 0;
};

/**
 * @brief sfml library wrapper
 */
struct sfmlWindow {
    public:

        bool ctor(const unsigned int window_width, const unsigned int window_height, const char* header,
                  sfmlCanvas* canvas);
        void dtor();

        void renew();

        bool is_closed();

        inline sfmlCanvas* canvas() { return _canvas; }

        inline size_t width()  { return _canvas->width(); }
        inline size_t height() { return _canvas->height(); }

        inline bool is_open() { return _window->isOpen(); }

        sf::Event event = {};

        inline bool events_left() { return _window->pollEvent(event); }

        inline void close() { _window->close(); }

    private:
        sf::RenderWindow* _window = nullptr;     //< sfml window class

        sfmlCanvas* _canvas = nullptr;

#ifdef SHOW_FPS
        sf::Font _font = {};
        sf::Text _fps_text = {};

        static const size_t _FPS_TEXT_BUF_SIZE = 32;
        char _fps_text_buf[_FPS_TEXT_BUF_SIZE] = "";

        void _show_fps();
#endif
};

#endif //< #ifndef LIB_SFML_H_
