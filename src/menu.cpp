#include "menu.hpp"

namespace cedit {

Menu::Menu() : Window(getmaxx(stdscr), 1, 0, getmaxy(stdscr) - 1) {}

void Menu::display(std::string message, short color) {
	wclear(this->window);

	message = "[ " + message + " ]";

	wattron(this->window, COLOR_PAIR(color));
	wattron(this->window, A_REVERSE);

	mvwprintw(this->window, 0, (size_t)(getmaxx(stdscr) / 2 - message.length() / 2), message.c_str());

	wattroff(this->window, A_REVERSE);
	wattroff(this->window, COLOR_PAIR(color));

	wrefresh(this->window);
}

void Menu::display_type(const char* text) {
    wbkgd(this->window, A_REVERSE);

    wclear(this->window);
    wprintw(this->window, text);

    mvwprintw(this->window, 0, std::strlen(text), "%s", this->content.c_str());

    wmove(this->window, 0, std::strlen(text) + this->currentIndex);
}

void Menu::event_insert() {
    if(this->key >= 32 && this->key <= 126) {
        this->content.insert(this->currentIndex, 1, (char)this->key);

        this->currentIndex++;
    }
}

void Menu::event_left() {
    if(this->currentIndex <= 0) {
        return;
    }

    this->currentIndex--;
}

void Menu::event_right() {
    if(this->content.length() <= this->currentIndex ) {
        return;
    }

    this->currentIndex++;
}

void Menu::event_backspace() {
    if(this->currentIndex <= 0) {
        return;
    }

    this->content.erase(this->currentIndex - 1, 1);
    this->currentIndex--;
}

void Menu::event_delete() {
    if(this->content.length() <= this->currentIndex ) {
        return;
    }

    this->content.erase(this->currentIndex, 1);
}

void Menu::event_pos_1() {
    this->currentIndex = 0;
}

void Menu::event_pos_end() {
    this->currentIndex = this->content.length();
}

[[nodiscard]] bool Menu::type(const char* text, const char* content) {
    this->content = content;
    this->currentIndex = 0;

    while(true) {

        this->display_type(text);

        this->key = wgetch(this->window);

        switch(this->key) {
            case 10:
                wbkgd(this->window, A_NORMAL);
				this->window_clear_refresh();

                return this->content.length() > 0;
            break;
            case 3: // abort strg+c
                wbkgd(this->window, A_NORMAL);
                this->window_clear_refresh();

                return false;
            break;
            case KEY_LEFT:
                this->event_left();
            break;
            case KEY_RIGHT:
                this->event_right();
            break;
            case KEY_BACKSPACE:
                this->event_backspace();
            break;
            case KEY_DC:
                this->event_delete();
            break;
            case KEY_HOME:
                this->event_pos_1();
            break;
            case KEY_END:
                this->event_pos_end();
            break;
            default:
                this->event_insert();
            break;
        }
    }

    return false;
}

}
