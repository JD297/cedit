#ifndef CEDIT_HPP
#define CEDIT_HPP

#include <ncurses.h>
#include <fstream>
#include <list>
#include <iterator>
#include <algorithm>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <map>

#include "version.hpp"
#include "menu.hpp"

namespace cedit {

struct SessionData
{
	std::list<std::string> content;
	long int contentIndex;
	std::size_t currentIndex;
	std::size_t savedIndex;
	std::size_t entryLine;
	bool showLineNumbers;
};

class Cedit
{
public:

	std::list<std::string> content;
	std::list<std::string>::iterator contentIt;

	std::map<std::string, SessionData> session;

	std::size_t currentIndex;
	std::size_t savedIndex;

	std::size_t entryLine;

	int key;

	bool refreshDisplay;
	bool refreshHeader;
	bool isrunning;

	std::string filename;

	bool showLineNumbers;

	int cursorX = 0, cursorY = 0, cursorXReset = 0;

	WINDOW* wheader;
	WINDOW* wcontent;

	Menu menu;

	std::size_t width = getmaxx(stdscr);
	std::size_t height = getmaxy(stdscr) - 2;

	std::size_t rwidth = getmaxx(stdscr);
	std::size_t rheight = getmaxy(stdscr);

	Cedit();

	void reset();

	void run();

	void event_save();

	void event_load(const char* filename);

	void event_open();

	void event_write();

	void event_backspace();

	void event_delete();

	std::list<std::string>::iterator displayFirstIt();

	std::list<std::string>::iterator displayLastIt();

	void scrollup();

	void scrolldown();

	void event_up();

	void event_down();

	void event_left();

	void event_right();

	void event_pagedown();

	void event_pageup();

	void event_pos1();

	void event_end();

	void event_toggle_linenumbers();

	void event_goto();

	void event_change_tab();

	void display();

	void display_header();

	void display_current_line();

	void display_linenumbers(std::list<std::string>::iterator it);

	void display_content();

	void window_print_color(WINDOW* window, short color, std::string line);

	void display_help(std::string key, std::string text);
};

}

#endif
