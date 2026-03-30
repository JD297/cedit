#include <curses.h>
#include <stdlib.h>
#include <unistd.h>

#include "cedit.hpp"

void NCURSES_INIT()
{
	initscr();
	noecho();
	cbreak();
	raw();
	keypad(stdscr, true);
	set_tabsize(4);

	start_color();

	use_default_colors();
}

void usage(void)
{
	fprintf(stderr, "usage: cedit [file]\n");
}

int main(int argc, char** argv)
{
	int ch;

	while ((ch = getopt(argc, argv, "")) != -1) {
		switch (ch) {
			default: {
				usage();

				exit(EXIT_FAILURE);
			}
		}
	}

	if (argc > optind + 1) {
		usage();

		exit(EXIT_FAILURE);
	}

	NCURSES_INIT();

	cedit::Cedit editor;

	editor.event_load(argv[optind]);

	editor.run();

	return 0;
}
