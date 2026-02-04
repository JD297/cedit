#include "window.hpp"

namespace cedit {

Window::Window(size_t sizeX, std::size_t sizeY, std::size_t posX, std::size_t posY) {
    this->width = sizeX;
    this->height = sizeY;

    this->window = newwin(this->height, this->width, posY, posX);
    keypad(this->window, true);
}

void Window::window_clear_refresh() {
    werase(this->window);
    wrefresh(this->window);
}

}
