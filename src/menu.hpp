#ifndef MENU_HPP
#define MENU_HPP

#include <string>
#include <cstring>
#include <list>
#include <sstream>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <ncurses.h>
#include "window.hpp"

namespace cedit {

class Menu : public cedit::Window  {
public:
	std::string content;

	Menu();

	[[nodiscard]] bool type(const char* text, const char* content = "");

	void display(std::string text);
private:
	std::size_t currentIndex;

	int key;

	void display_type(const char* text);

	void event_insert();

	void event_left();

	void event_right();

	void event_backspace();

	void event_delete();

	void event_pos_1();

	void event_pos_end();
};

}
#endif
