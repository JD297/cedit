#ifndef CEDIT_HPP
#define CEDIT_HPP

#include <ncurses.h>
#include <fstream>
#include <list>
#include <map>
#include <iterator>
#include <algorithm>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include <regex>

#include "version.hpp"
#include "menu.hpp"

namespace cedit {

struct BrainData
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

	std::map<std::string, BrainData> brain;

	std::size_t currentIndex;
	std::size_t savedIndex;

	std::size_t entryLine;

	int key;

	bool refreshDisplay;
	bool refreshHeader;
	bool isrunning;

	std::string filename;

	bool showLineNumbers;
	bool quite;

	int cursorX = 0, cursorY = 0, cursorXReset = 0;

	WINDOW* wheader;
	WINDOW* wcontent;
	WINDOW* wmenu;
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

	void scrollup(size_t n = 1);

	void scrolldown(size_t n = 1);

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

	void event_quite();

	void display();

	void menu_reset();

	void menu_print(const std::string text, const size_t x, const size_t y);

	void menu_print_clear(const std::string text, const size_t x, const size_t y);

	void display_header();

	void display_current_line();

	void display_content();

	void display_syntax_content(std::string line);

	void window_print_color(WINDOW* window, short color, std::string line);

	void display_help(std::string key, std::string text);
};

}

#endif
