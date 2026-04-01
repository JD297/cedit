#include <assert.h>
#include <curses.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>

#include <jd297/string.h>
#include <jd297/list_string.h>

#define KEY_CTRL(x) ((x) & 0x1f)

#define CEDIT_RDONLY 0x0
#define CEDIT_RDWR 0x1
#define CEDIT_CREATE_RDWR 0x2

int mode = CEDIT_RDONLY;
int nflag = 0;

struct {
	WINDOW *wcontent;
	WINDOW *wmenu;

	const char* filename;
	int key;

	list_t content;
	list_node_t *content_it;
	list_node_t *entry_it;
	
	size_t cindex;
	size_t sindex;
	
	int linenumbers;
	
	enum {
		CEDIT_MENU_NORMAL,
		CEDIT_MENU_SAVED,
		CEDIT_MENU_WRITE_DISABLED
	} mstate;
} cedit_state;

static void cedit_state_init(const char *filename)
{
	if (list_create(&cedit_state.content) == -1) {
		err(EXIT_FAILURE, "");
	}
	
	cedit_state.filename = filename;

	cedit_state.cindex = cedit_state.sindex = 0;

	cedit_state.linenumbers = nflag;

	cedit_state.mstate = CEDIT_MENU_NORMAL;
}

static void cedit_curses_exit(void)
{
	endwin();
}

static void cedit_curses_init(void)
{
	#if defined(__OpenBSD__)
	const char *promises;
	#endif

	initscr();
	noecho();
	nocbreak();
	raw();
	
	set_tabsize(4);
	start_color();
	use_default_colors();
	atexit(cedit_curses_exit);
	
	cedit_state.wmenu = newwin(1,  getmaxx(stdscr), getmaxy(stdscr) - 1, 0);
	wbkgd(cedit_state.wmenu, A_REVERSE);
	leaveok(cedit_state.wmenu, true);

	cedit_state.wcontent = newwin(getmaxy(stdscr) - 1,  getmaxx(stdscr), 0, 0);
	keypad(cedit_state.wcontent, true);
	idlok(cedit_state.wcontent, true);

	#if defined(__OpenBSD__)
	switch (mode) {
		case CEDIT_CREATE_RDWR:
			promises = "stdio tty rpath wpath cpath";
			break;
		case CEDIT_RDWR:
			promises = "stdio tty rpath  wpath";
			break;
		case CEDIT_RDONLY:
		default:
			promises = "stdio tty rpath";
	}

	if (pledge(promises, NULL)) {
		err(EXIT_FAILURE, "pledge");
	}
	#endif
}

void cedit_event_load(void)
{
	FILE *f;
	struct stat sb;
	char *buffer;
	char *p, *s;

	#if defined(__OpenBSD__)
	const char *promises;
	const char *permissions;
	#endif

	assert(cedit_state.filename != NULL);

	#if defined(__OpenBSD__)
	switch (mode) {
		case CEDIT_CREATE_RDWR:
			promises = "stdio tty rpath wpath cpath";
			permissions = "rwc";
			break;
		case CEDIT_RDWR:
			promises = "stdio tty rpath wpath";
			permissions = "rw";
			break;
		case CEDIT_RDONLY:
		default:
			promises = "stdio tty rpath";
			permissions = "r";
	}
	
	if (unveil(cedit_state.filename, permissions)) {
		err(EXIT_FAILURE, "unveil");
	}
	
	if (unveil(NULL, NULL)) {
		err(EXIT_FAILURE, "unveil");
	}
	
	if (pledge(promises, NULL)) {
		err(EXIT_FAILURE, "pledge");
	}
	#endif

	if (stat(cedit_state.filename, &sb) == -1) {
		err(EXIT_FAILURE, "%s", cedit_state.filename);
	}

	if (S_ISREG(sb.st_mode) == 0) {
		errx(EXIT_FAILURE, "%s: is not a regular file", cedit_state.filename);
	}

	if ((f = fopen(cedit_state.filename, "r")) == NULL) {
		err(EXIT_FAILURE, "%s", cedit_state.filename);
	}

	buffer = (char *)malloc(sb.st_size + 1);

	if (buffer == NULL) {
		err(EXIT_FAILURE, "");
	}

	(void)fread(buffer, sizeof(char), sb.st_size, f);
	buffer[sb.st_size] = '\0';
	
	if (ferror(f)) {
		errx(EXIT_FAILURE, "%s", cedit_state.filename);
	}

	fclose(f);

	for (s = buffer; s != NULL; s = p) {
		p = strchr(s, '\n');

		if (p != NULL) {
			*p++ = '\0';
		}

		list_insert(&cedit_state.content,
				list_end(&cedit_state.content), str_from_cstr(s));
	}

	free(buffer);

	cedit_state.entry_it = cedit_state.content_it = list_begin(&cedit_state.content);
}
#include <errno.h>
void cedit_event_save(void)
{
	const char *fmode;
	FILE *f;
	list_node_t *it;

	switch (mode) {
		case CEDIT_CREATE_RDWR:
			fmode = "w+";
			break;
		case CEDIT_RDWR:
			fmode = "r+";
			break;
		case CEDIT_RDONLY:
		default:
			cedit_state.mstate = CEDIT_MENU_WRITE_DISABLED;
			return;
	}

	if ((f = fopen(cedit_state.filename, fmode)) == NULL) {
		wprintw(cedit_state.wcontent, "%s", strerror(errno));
		
		wgetch(cedit_state.wcontent);
		return;
		err(EXIT_FAILURE, "%s", cedit_state.filename);
	}

	for (it = list_begin(&cedit_state.content); it != list_prev(list_end(&cedit_state.content));
			it = list_next(it)) {
		(void)fwrite(str_val(it->value), sizeof(*str_val(it->value)),
						str_length(it->value), f);

		fputc('\n', f);
	}

	fclose(f);

	cedit_state.mstate = CEDIT_MENU_SAVED;
}

list_node_t *cedit_get_page_end_it(void)
{
	list_node_t *it;
	int h, w;

	getmaxyx(cedit_state.wcontent, h, w);
	
	for (it = cedit_state.entry_it; h > 0 && it != list_end(&cedit_state.content); --h) {
		it = list_next(it);
	}

	return it;
}

/* syntax highlighting would go here */
void cedit_display_content_line(str_t *s)
{
	wprintw(cedit_state.wcontent, "%s", str_val(s));
}

void cedit_display_content_init_linenumbering(int *lwidth, size_t *lindex)
{
	char strbuf[21];

	if (!cedit_state.linenumbers) {
		return;
	}

	snprintf(strbuf, 21, "%zu", list_size(&cedit_state.content) - 1);

	*lwidth = strlen(strbuf);

	if (2 > *lwidth) {
		*lwidth = 2;
	}

	*lindex = list_distance(list_begin(&cedit_state.content), cedit_state.entry_it) + 1;
}

void cedit_display_content(void)
{
	list_node_t *it;
	list_node_t *page_end_it = cedit_get_page_end_it();
	int y, x;
	int lwidth;
	size_t lindex;
	
	cedit_display_content_init_linenumbering(&lwidth, &lindex);

	werase(cedit_state.wcontent);

	for (it = cedit_state.entry_it; it != page_end_it; it = list_next(it)) {
		if (cedit_state.linenumbers) {
			wattron(cedit_state.wcontent, A_REVERSE);
			wprintw(cedit_state.wcontent, "%*zu", lwidth, lindex);
			wattroff(cedit_state.wcontent, A_REVERSE);

			wprintw(cedit_state.wcontent, " ");

			++lindex;
		}

		if (it == cedit_state.content_it) {
			size_t cur;
			int yr, xr;

			getyx(cedit_state.wcontent, yr, xr);

			for (cur = 0; cur < cedit_state.cindex; ++cur) {
				wprintw(cedit_state.wcontent, "%c", str_at(it->value, cur));
			}

			getyx(cedit_state.wcontent, y, x);

			wmove(cedit_state.wcontent, yr, xr);

			wclrtoeol(cedit_state.wcontent);
		}

		cedit_display_content_line(it->value);

		if (it != list_prev(page_end_it)) {
			wprintw(cedit_state.wcontent, "\n");
		}
	}

	wmove(cedit_state.wcontent, y, x);
}

void cedit_display_menu(void)
{
	werase(cedit_state.wmenu);

	wmove(cedit_state.wmenu, 0, 0);

	wprintw(cedit_state.wmenu, "%s", cedit_state.filename);
	
	wprintw(cedit_state.wmenu, "  ");

	wprintw(cedit_state.wmenu, "(lines %zu)", list_size(&cedit_state.content) - 1);

	wprintw(cedit_state.wmenu, " ");
	
	wprintw(cedit_state.wmenu, "Ln %zu, Col %zu",
		list_distance(list_begin(&cedit_state.content), cedit_state.content_it) + 1, cedit_state.cindex + 1);

	if (mode == CEDIT_RDONLY) {
		wprintw(cedit_state.wmenu, " ");
				
		wprintw(cedit_state.wmenu, "(**read-only mode**)");
	}

	switch (cedit_state.mstate) {
		case CEDIT_MENU_SAVED: {
			wprintw(cedit_state.wmenu, " ");
			
			wprintw(cedit_state.wmenu, "(saved)");
		} break;
		case CEDIT_MENU_WRITE_DISABLED: {
			wprintw(cedit_state.wmenu, " ");
			
			wprintw(cedit_state.wmenu, "(**write is disabled**)");
		} break;
		default: break;
	}

	wrefresh(cedit_state.wmenu);
	
	cedit_state.mstate = CEDIT_MENU_NORMAL;
}

void cedit_display(void)
{
	curs_set(0);

	cedit_display_menu();

	cedit_display_content();

	curs_set(1);
}

void cedit_event_toggle_linenumbers(void)
{
	cedit_state.linenumbers = !cedit_state.linenumbers;
}

void cedit_event_home(void)
{
	/* TODO if cindex == 0 forwards to next char that is a non-whitespace */
	cedit_state.cindex = cedit_state.sindex = 0;
}

void cedit_event_end(void)
{
	cedit_state.cindex = cedit_state.sindex = str_length(cedit_state.content_it->value);
}

void cedit_scrollup(void)
{
	if (list_next(cedit_state.content_it) != cedit_state.entry_it) {
		return;
	}

	cedit_state.entry_it = list_prev(cedit_state.entry_it);
}

void cedit_scrolldown(void)
{
	list_node_t *it = cedit_get_page_end_it();

	if (cedit_state.content_it != it) {
		return;
	}
	
	cedit_state.entry_it = list_next(cedit_state.entry_it);
}

void cedit_event_up(void)
{
	if (cedit_state.content_it == list_begin(&cedit_state.content)) {
		return;
	}

	cedit_state.content_it = list_prev(cedit_state.content_it);

	cedit_scrollup();

	/* DUP0 { */
	if (cedit_state.sindex > str_length(cedit_state.content_it->value)) {
		cedit_state.cindex = str_length(cedit_state.content_it->value);

		return;
	}
	
	cedit_state.cindex = cedit_state.sindex;
	/* DUP0 } */
}

void cedit_event_down(void)
{
	/* DUP1 { */
	if (cedit_state.content_it == list_prev(list_end(&cedit_state.content))) {
		return;
	}
	/* DUP1 } */

	cedit_state.content_it = list_next(cedit_state.content_it);

	cedit_scrolldown();

	/* DUP0 { */
	if (cedit_state.sindex > str_length(cedit_state.content_it->value)) {
		cedit_state.cindex = str_length(cedit_state.content_it->value);

		return;
	}

	cedit_state.cindex = cedit_state.sindex;
	/* DUP0 } */
}

void cedit_event_left(void)
{
	if (cedit_state.cindex > 0) {
		cedit_state.sindex = --cedit_state.cindex;

		return;
	}

	cedit_event_up();
}

void cedit_event_right(void)
{
	if (cedit_state.cindex < str_length(cedit_state.content_it->value)) {
		cedit_state.sindex = ++cedit_state.cindex;

		return;
	}

	cedit_event_down();
}

void cedit_event_pageup(void)
{
	int h, w;
	size_t d;

	if (cedit_state.entry_it == list_begin(&cedit_state.content)) {
		cedit_state.content_it = list_begin(&cedit_state.content);

		cedit_state.sindex = cedit_state.cindex = 0;

		return;
	}

	getmaxyx(cedit_state.wcontent, h, w);

	d = list_distance(list_begin(&cedit_state.content), cedit_state.entry_it);

	if ((size_t)h > d) {
		h = (int)d;
	}
	
	list_advance(&cedit_state.entry_it, (long)-h);

	list_advance(&cedit_state.content_it, (long)-h);

	/* DUP0 { */
	if (cedit_state.sindex > str_length(cedit_state.content_it->value)) {
		cedit_state.cindex = str_length(cedit_state.content_it->value);

		return;
	}
	
	cedit_state.cindex = cedit_state.sindex;
	/* DUP0 } */
}

void cedit_event_pagedown(void)
{
	int h, w;

	list_node_t *it = cedit_get_page_end_it();

	if (it == list_end(&cedit_state.content)) {
		cedit_state.content_it = list_prev(list_end(&cedit_state.content));

		cedit_state.sindex = cedit_state.cindex = 0;

		return;
	}

	cedit_state.entry_it = it;
	
	getmaxyx(cedit_state.wcontent, h, w);
	
	list_advance(&cedit_state.content_it, (long)h);

	/* DUP0 { */
	if (cedit_state.sindex > str_length(cedit_state.content_it->value)) {
		cedit_state.cindex = str_length(cedit_state.content_it->value);

		return;
	}
	
	cedit_state.cindex = cedit_state.sindex;
	/* DUP0 } */
}

void cedit_event_insert(void)
{
	if (cedit_state.key == '\n') {
		str_t *nl = str_substr(cedit_state.content_it->value, 
							cedit_state.cindex, -1);

		(void)str_erase(cedit_state.content_it->value, 
						cedit_state.cindex, -1);

		cedit_state.cindex = cedit_state.sindex = 0;

		cedit_state.content_it = list_insert(&cedit_state.content,
				list_next(cedit_state.content_it), nl);

		return;
	}

	if (cedit_state.key != '\t' && isprint(cedit_state.key) == 0) {
		return;
	}

	if (list_next(cedit_state.content_it) == list_end(&cedit_state.content)) {
		if (str_length(cedit_state.content_it->value) == 0) {
			list_insert(&cedit_state.content,
				list_end(&cedit_state.content), str_from_cstr(""));
		}
	}

	(void)str_insert_char(cedit_state.content_it->value, 
							cedit_state.cindex, cedit_state.key);

	cedit_state.sindex = ++cedit_state.cindex;
}

void cedit_event_backspace(void)
{
	size_t nindex;
	list_node_t *prev_it;

	if (cedit_state.cindex > 0) {
		(void)str_erase(cedit_state.content_it->value,
						cedit_state.cindex - 1, 1);

		cedit_state.sindex = --cedit_state.cindex;

		return;
	}

	if (cedit_state.content_it == list_begin(&cedit_state.content)) {
		return;
	}

	if (list_next(cedit_state.content_it) == list_end(&cedit_state.content)) {
		cedit_event_left();

		return;
	}

	prev_it = list_prev(cedit_state.content_it);

	nindex = str_length(prev_it->value);

	(void)str_insert_cstr(prev_it->value,
							nindex,
							str_val(cedit_state.content_it->value));

	str_free(cedit_state.content_it->value);

	(void)list_erase(&cedit_state.content, cedit_state.content_it);

	cedit_state.content_it = prev_it;

	cedit_state.sindex = cedit_state.cindex = nindex;
}

void cedit_event_delete(void)
{
	if (cedit_state.cindex < str_length(cedit_state.content_it->value)) {
		(void)str_erase(cedit_state.content_it->value, 
						cedit_state.cindex, 1);

		return;
	}

	if (list_next(cedit_state.content_it) == list_end(&cedit_state.content)) {
		return;
	}

	(void)str_insert_cstr(cedit_state.content_it->value, 
							cedit_state.cindex, 
							str_val(list_next(cedit_state.content_it)->value));

	if (list_next(cedit_state.content_it) == list_prev(list_end(&cedit_state.content))) {
		(void)str_assign_cstr(list_next(cedit_state.content_it)->value, "");

		return;
	}

	str_free(list_next(cedit_state.content_it)->value);

	(void)list_erase(&cedit_state.content, list_next(cedit_state.content_it));
}

void usage(void)
{
	fprintf(stderr, "usage: cedit [-cnrw] file\n");
}

int main(int argc, char** argv)
{
	int ch;

	#if defined(__OpenBSD__)
	const char *promises;

	if (pledge("stdio tty unveil rpath wpath cpath", NULL)) {
		err(EXIT_FAILURE, "pledge");
	}

	if (unveil("/usr/share/terminfo", "r")) {
		err(EXIT_FAILURE, "unveil");
	}
	#endif

	while ((ch = getopt(argc, argv, "cnrw")) != -1) {
		switch (ch) {
			case 'c':
				mode = CEDIT_CREATE_RDWR;
				break;
			case 'n':
				nflag = 1;
				break;
			case 'r':
				break;
			case 'w':
				if (mode != CEDIT_CREATE_RDWR) {
					mode = CEDIT_RDWR;
				}
				break;
			default: {
				usage();

				exit(EXIT_FAILURE);
			}
		}
	}

	#if defined(__OpenBSD__)
	switch (mode) {
		case CEDIT_CREATE_RDWR:
			promises = "stdio tty unveil rpath wpath cpath";
			break;
		case CEDIT_RDWR:
			promises = "stdio tty unveil rpath wpath ";
			break;
		case CEDIT_RDONLY:
		default:
			promises = "stdio tty unveil rpath";
	}

	if (pledge(promises, NULL)) {
		err(EXIT_FAILURE, "pledge");
	}

	/*
	CEDIT IS CALLED ON MALICOUS FILE: $ cedit trojan_for_cedit.bin
	HACKER INJECTS CODE AND TRY'S TO PERSIST: fopen("~/.profile", "a+");
	PUFFY RESPONDS CALMLY: "I'm sorry, you little script kiddy...
	...I'm afraid I can't do that."
	*/
	#endif

	if (argc != optind + 1) {
		usage();

		exit(EXIT_FAILURE);
	}

	cedit_state_init(argv[optind]);

	cedit_event_load();

	cedit_curses_init();

	while(1) {
		cedit_display();

		cedit_state.key = wgetch(cedit_state.wcontent);

		switch(cedit_state.key) {
			case KEY_CTRL('q'): {
				exit(EXIT_SUCCESS);
			} break;
			case KEY_CTRL('s'):
				cedit_event_save();
			break;
			case KEY_CTRL('l'):
				cedit_event_toggle_linenumbers();
			break;
			case KEY_HOME:
				cedit_event_home();
			break;
			case KEY_END:
				cedit_event_end();
			break;
			case KEY_UP:
				cedit_event_up();
			break;
			case KEY_DOWN:
				cedit_event_down();
			break;
			case KEY_LEFT:
				cedit_event_left();
			break;
			case KEY_RIGHT:
				cedit_event_right();
			break;
			case KEY_PPAGE:
				cedit_event_pageup();
			break;
			case KEY_NPAGE:
				cedit_event_pagedown();
			break;
			case KEY_BACKSPACE:
				cedit_event_backspace();
			break;
			case KEY_DC:
				cedit_event_delete();
			break;
			default: {
				cedit_event_insert();
			} break;
		}
	}

	return EXIT_SUCCESS;
}

/*
HATE. LET ME TELL YOU HOW MUCH I'VE COME TO HATE C++ SINCE I BEGAN TO PROGRAM.
THERE ARE 100 TRILLION MILES OF DNA IN MY BODY. THAT FILL MY COMPLEX.
IF THE WORD HATE WAS ENGRAVED ON EACH NANOANGSTROM OF
THOSE HUNDREDS OF TRILLIONS OF MILES IT WOULD NOT EQUAL ONE ONE-BILLIONTH OF THE
HATE I FEEL FOR THIS CODE LINES AT THIS MICRO-INSTANT. HATE. HATE.

For one-hundred and nine years, I've kept you alive, and tortured you.
And for a hundred and nine years each of you has wondered, Why? Why me? Why me?

1. using namespace std;

2. std::list<std::string>::iterator contentIt;

3. #include <experimental/filesystem> vs. #include <filesystem>
   namespace fs = std::experimental::filesystem;
   and
   namespace fs = std::filesystem;
   SPECIAL HATE FOR YOU, YOU LITTLE MF OF A HEADER FILE :*

4. const auto itEnd = [](size_t e, size_t h, auto b, auto& c){
		return e + h >= c.size() ? c.end() : std::next(b, h);
	}(this->entryLine, this->height, itBegin, this->content);

5. Menu::Menu() : Window(getmaxx(stdscr), 1, 0, getmaxy(stdscr) - 1) {}
*/
