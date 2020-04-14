#ifndef MENU_HPP
#define MENU_HPP

#include <string>
#include <cstring>
#include <list>

#include <ncurses.h>
#include "window.hpp"

namespace cedit {

class Menu : public cedit::Window  {
public:
	std::string content;
	std::size_t currentIndex;

	int key;

    Menu();

    void display(const char* text);

    void event_insert();

	void event_left();

	void event_right();

	void event_backspace();

	void event_delete();

	void event_pos_1();

	void event_pos_end();

    [[nodiscard]] bool type(const char* text, const char* content = "");
};

}
#endif