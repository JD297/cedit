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
#include <bits/stdc++.h>

#include "version.hpp"

namespace cedit {

class Cedit
{
public:

	std::list<std::string> content;
	std::list<std::string>::iterator contentIt;

	std::size_t currentIndex;
	std::size_t savedIndex;

	std::size_t entryLine;

	short int key;

	bool refreshDisplay;
	bool refreshHeader;
	bool isrunning;

	std::string filename;

	WINDOW* wheader;
	WINDOW* wcontent;
	WINDOW* wmenu;
	WINDOW* wfooter;

	const std::size_t width = getmaxx(stdscr);
	const std::size_t height = getmaxy(stdscr) - 5;

	const std::size_t rwidth = width;
	const std::size_t rheight = height+5;

	Cedit();

	void reset();

	void run();

	void event_save();

	void event_load(const char* filename);

	void event_open();

	void event_write();

	void event_backspace();

	void event_delete();

	void event_up();

	void event_down();

	void event_left();

	void event_right();

	void event_pagedown();

	void event_pageup();

	void display();

	void menu_reset();

	void menu_print(const std::string text, const size_t x, const size_t y);

	void menu_print_clear(const std::string text, const size_t x, const size_t y);

	void display_header();

	void display_content();

	void display_help(std::string key, std::string text);

	void display_footer();
};

}

#endif
