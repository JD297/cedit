#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <cstddef>
#include <ncurses.h>

namespace cedit {

class Window {
public:
    WINDOW* window;

    std::size_t width;
    std::size_t height;

    Window(std::size_t sizeX, std::size_t sizeY, std::size_t posX, std::size_t posY);

    void window_clear_refresh();
};

}
#endif
