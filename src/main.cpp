#include <iostream>

#include "cedit.hpp"

void NCURSES_INIT()
{
	initscr();
	noecho();
	cbreak();
	raw();
	curs_set(0);
	keypad(stdscr, true);
	setlocale(LC_ALL, "");
}

namespace cedit
{
	void arguments(const int argc, const char** argv, cedit::Cedit* cedit)
	{
		if(argc >= 3)
		{
			endwin();

			std::cerr << "cedit: invalid argument -- \'" << argv[2] << "\'\n";
			std::cerr << "usage: cedit [path/to/file]\n";

			exit(1);
		}
		else if(argc == 2) {
			cedit->event_load(argv[1]);
		}
	}
}

int main(const int argc, const char** argv)
{
	NCURSES_INIT();

	cedit::Cedit editor;

	cedit::arguments(argc, argv, &editor);

	editor.run();

	return 0;
}
