#include "cedit.hpp"

#include <stddef.h>
#include <sys/stat.h>

namespace cedit {

Cedit::Cedit():
	currentIndex(0),
	savedIndex(0),
	entryLine(0),
	key(0),
	refreshHeader(true),
	isrunning(true),
	filename(""),
	showLineNumbers(false)
{
	this->wheader = newwin(1, this->width, 0, 0);
	this->wcontent = newwin(this->height, this->width, 1, 0);

	this->content.emplace_back("");
	this->contentIt = this->content.begin();

	keypad(this->wcontent, true);
	scrollok(this->wcontent, true);
}

void Cedit::reset()
{
	this->content.clear();
	this->content.emplace_back("");
	this->contentIt = this->content.begin();
	this->currentIndex = 0;
	this->savedIndex = 0;
	this->entryLine = 0;
}

void Cedit::run()
{
	while(this->isrunning)
	{
		curs_set(0);

		this->display();

		curs_set(1);

		this->key = wgetch(this->wcontent);

		switch(this->key)
		{
			case KEY_CTRL('s'):
				this->event_save();
			break;
			case KEY_CTRL('o'):
				this->event_open();
			break;
			case KEY_CTRL('q'):
				this->isrunning = false;
				endwin();
			break;
			case KEY_BACKSPACE:
				this->event_backspace();
			break;
			case KEY_DC:
				this->event_delete();
			break;
			case KEY_LEFT:
				this->event_left();
			break;
			case KEY_RIGHT:
				this->event_right();
			break;
			case KEY_UP:
				this->event_up();
			break;
			case KEY_DOWN:
				this->event_down();
			break;
			case KEY_NPAGE:
				this->event_pagedown();
			break;
			case KEY_PPAGE:
				this->event_pageup();
			break;
			case KEY_HOME:
				this->event_pos1();
			break;
			case KEY_END:
				this->event_end();
			break;
			case KEY_CTRL('l'):
				this->event_toggle_linenumbers();
			break;
			case KEY_CTRL('g'):
				this->event_goto();
			break;
			case KEY_CTRL('x'):
				this->event_cut();
			break;
			case KEY_CTRL('v'):
				this->event_paste();
			break;
			default:
				this->event_write();
			break;
		}
	}
}

void Cedit::event_save()
{
	FILE *f;

	if(this->filename.empty())
	{
		if(!this->menu.type(FILE_SAVE)) {
			return;
		}

		this->filename = this->menu.content;

		this->refreshHeader = true;
	}

	if ((f = fopen(this->filename.c_str(), "w")) == NULL) {
		this->menu.display(std::string(FAIL_WRITE) + std::string("\"") + std::string(filename) + "\": " + std::strerror(errno));

		return;
	}

	auto it_end = std::prev(this->content.end());

	if(!it_end->empty())
	{
		it_end->append("\n");

		this->content.emplace_back("");
	}

	for(auto it = this->content.begin(); it != this->content.end(); it++)
	{
		(void)fwrite(it->c_str(), it->length(), sizeof(char), f);

		if (ferror(f)) {
			this->menu.display(std::string(FAIL_WRITE) + std::string("\"") + std::string(filename) + "\": " + std::strerror(errno));
			
			break;
		}
	}

	fclose(f);

	this->menu.display(std::to_string(content.size()-1) + " " + SAVE);
}

void Cedit::event_load(const char* filename)
{
	FILE *f;
	struct stat sb;

	if (filename == NULL) {
		return;
	}

	this->filename = std::string(filename);
	this->refreshHeader = true;

	this->reset();

	std::string message = "";
	std::string message_filename = "\"" + std::string(filename) + "\"";

	if (stat(filename, &sb) == -1) {
		if (errno == ENOENT) {
			this->menu.display(NEW_FILE);

			return;
		}

		this->menu.display(std::string(FAIL_READ) + std::string("\"") + std::string(filename) + "\": " + std::strerror(errno));

		return;
	}

	if (S_ISREG(sb.st_mode) == 0) {
		this->menu.display(message_filename + IS_NOT_REGULAR_FILE);

		return;
	}

	if ((f = fopen(filename, "r")) == NULL) {
		this->menu.display(std::string(FAIL_READ) + std::string("\"") + std::string(filename) + "\": " + std::strerror(errno));

		return;
	}

	this->content.clear();

	char *line = NULL;
	size_t linesz = 0;

	while (getline(&line, &linesz, f) != -1) {
		this->content.emplace_back(std::string(line));

		line = NULL;
	}

	if (ferror(f)) {
		this->content.clear();

		this->menu.display(std::string(FAIL_READ) + std::string("\"") + std::string(filename) + "\": " + std::strerror(errno));

		return;
	}

	fclose(f);

	this->content.emplace_back("");

	this->contentIt = this->content.begin();

	this->menu.display(std::to_string(this->content.size() - 1) + " " + LOAD);
}

void Cedit::event_open()
{
	if(!this->menu.type(FILE_OPEN)) {
		return;
	}

	this->refreshHeader = true;

	this->event_load(this->menu.content.c_str());
}

void Cedit::event_write()
{
	if( (this->key >= 32 && this->key <= 126) || this->key == 10 || this->key == 9)
	{
		this->contentIt->insert(this->currentIndex, 1, (char)key);
	}

	// Return key
	if(this->key == 10)
	{
		if(this->currentIndex < this->contentIt->length()-1)
		{
			// Insert new line with content after cursor in new line
			this->content.insert(std::next(this->contentIt), this->contentIt->substr(this->currentIndex+1));

			// Shorten the current line until the cursor
			*this->contentIt = this->contentIt->substr(0, this->currentIndex+1);
		}
		else
		{
			// Insert new line empty after current line
			this->content.insert(std::next(this->contentIt), "");
		}

		this->contentIt = std::next(this->contentIt);

		this->currentIndex = 0;
		this->savedIndex = 0;
	}
	else if((this->key >= 32 && this->key <= 126) || this->key == 9)
	{
		this->currentIndex++;
		this->savedIndex = this->currentIndex;
	}
}

void Cedit::event_backspace()
{
	// If at beginning of line and current line is not the first line
	if(this->currentIndex == 0 && this->contentIt != this->content.begin())
	{
		this->scrollup();

		// When the line has no content with exception of "\n"
		if(std::prev(this->contentIt)->length() != 1)
		{
			this->contentIt = std::prev(this->contentIt);

			this->currentIndex = this->contentIt->length()-1;
			this->savedIndex = this->contentIt->length()-1;

			if(this->contentIt->length() > 0) {
				// Append the complete line to the previous line
				this->contentIt->append(std::next(this->contentIt)->substr(0));

				// Delete the first line break in the current line
				this->contentIt->erase(this->contentIt->find("\n"), 1);
			}

			// Delete the line from where the list was copied from
			this->content.erase(std::next(this->contentIt));
		}
		else if(std::prev(this->contentIt)->length() == 1)
		{
			// Delete the previous line with the "\n". The line will be poped one row up
			this->content.erase(std::prev(this->contentIt));

			this->currentIndex = 0;
			this->savedIndex = 0;
		}
	}
	else if(!(this->currentIndex == 0 && this->contentIt == this->content.begin()))
	{
		// Deletes the character that is in front of the cursor
		this->contentIt->erase(this->currentIndex-1, 1);

		this->currentIndex--;
		this->savedIndex = this->currentIndex;
	}
}

void Cedit::event_delete()
{
	// Last character and last row
	if(this->currentIndex == this->contentIt->length() && this->content.end() == std::next(this->contentIt))
	{
		return;
	}
	// Line end: copy next line to this line, delete next line
	else if(this->currentIndex == this->contentIt->length()-1 && this->contentIt->back() == '\n')
	{
		// If the line has more content then "\n"
		if(std::next(this->contentIt)->length() != 1)
		{
			// Remove the paragraph
			this->contentIt->erase(this->currentIndex, 1);

			// Copy the entire next line to the current line
			this->contentIt->append(std::next(this->contentIt)->substr(0));

			// Delete the line from where the list was copied from
			this->content.erase(std::next(this->contentIt));
		}
		else if(std::next(this->contentIt)->length() == 1)
		{
			// Delete the next line
			this->content.erase(std::next(this->contentIt));
		}
	}
	else
	{
		// Delete character
		this->contentIt->erase(this->currentIndex, 1);
	}
}

std::list<std::string>::iterator Cedit::displayFirstIt()
{
	return std::next(this->content.begin(), this->entryLine);
}

std::list<std::string>::iterator Cedit::displayLastIt()
{
	const auto itBegin = this->displayFirstIt();

	return this->entryLine + this->height >= this->content.size() ? this->content.end() : std::next(itBegin, this->height);
}

void Cedit::scrollup()
{
	if(this->contentIt == this->displayFirstIt())
	{
		this->entryLine--;
		wscrl(this->wcontent, -1);
	}
}

void Cedit::scrolldown()
{
	if(this->contentIt == this->displayLastIt())
	{
		this->entryLine++;
		wscrl(this->wcontent, 1);
	}
}

void Cedit::event_up()
{
	if(this->contentIt != this->content.begin())
	{
		this->scrollup();
		this->contentIt = std::prev(this->contentIt);

		if(this->contentIt->length()-1 < this->savedIndex)
		{
			this->currentIndex = this->contentIt->length()-1;
		}
		else
		{
			this->currentIndex = this->savedIndex;
		}
	}
}

void Cedit::event_down()
{
	if(this->contentIt == std::prev(this->content.end()))
	{
		return;
	}

	this->contentIt = std::next(this->contentIt);
	this->scrolldown();

	if(this->contentIt->length() <= this->savedIndex && this->contentIt == std::prev(this->content.end()))
	{
		this->currentIndex = this->contentIt->length();
	}
	else if(this->contentIt->length()-1 < this->savedIndex)
	{
		this->currentIndex = this->contentIt->length()-1;
	}
	else
	{
		this->currentIndex = this->savedIndex;
	}
}

void Cedit::event_left()
{
	if(this->currentIndex > 0)
	{
		this->currentIndex--;
		this->savedIndex = this->currentIndex;
	}
	else if(this->currentIndex == 0 && this->contentIt != this->content.begin())
	{
		this->scrollup();
		this->contentIt = std::prev(this->contentIt);

		this->currentIndex = this->contentIt->length()-1;
		this->savedIndex = this->currentIndex;
	}
}

void Cedit::event_right()
{
	const auto itEnd = std::prev(this->content.end());

	if(this->currentIndex < this->contentIt->length()-1 ||
	  (this->currentIndex < this->contentIt->length() && contentIt == itEnd)
	)
	{
		if(currentIndex == this->contentIt->length() && this->contentIt->back() != '\n')
		{
			return;
		}

		this->currentIndex++;
		this->savedIndex = this->currentIndex;
	}
	else if(this->contentIt != itEnd)
	{
		this->contentIt = std::next(this->contentIt);
		this->scrolldown();

		this->currentIndex = 0;
		this->savedIndex = 0;
	}
}

void Cedit::event_pagedown()
{
	if(this->entryLine > this->content.size()-this->height)
	{
		this->contentIt = std::prev(this->content.end());
	}
	else
	{
		this->entryLine += this->height;

		// Compares long int with size_t aka unsigned long int, type cast is required
		if(std::distance(this->contentIt, this->content.end()) > (long int)this->height)
		{
			this->contentIt = std::next(this->contentIt, this->height);
		}
		else
		{
			this->contentIt = std::prev(this->content.end());
		}
	}

	this->currentIndex = 0;
	this->savedIndex = 0;
}

void Cedit::event_pageup()
{
	if(this->entryLine < this->height)
	{
		this->entryLine = 0;
		this->contentIt = this->content.begin();
	}
	else
	{
		this->entryLine -= this->height;
		this->contentIt = std::prev(this->contentIt, this->height);
	}

	this->currentIndex = 0;
	this->savedIndex = 0;
}

void Cedit::event_pos1()
{
	this->currentIndex = 0;
	this->savedIndex = 0;
}

void Cedit::event_end()
{
	if(contentIt != std::prev(this->content.end()))
	{
		this->currentIndex = this->contentIt->length()-1;
		this->savedIndex = this->contentIt->length()-1;
	}
	else
	{
		this->currentIndex = this->contentIt->length();
		this->savedIndex = this->contentIt->length();
	}
}

void Cedit::event_toggle_linenumbers()
{
	this->showLineNumbers = !this->showLineNumbers;
}

void Cedit::event_goto()
{
	if(!this->menu.type("Springe zu [Zeile Spalte]: ")) {
		return;
	}

	std::size_t row;
	std::size_t column = 1;
	std::stringstream converter(this->menu.content);
	converter >> row >> column;
	row--;
	column--;

	if(this->content.size() > row)
	{
		this->contentIt = std::next(this->content.begin(), row);

		this->entryLine = row;

		if(this->contentIt->length() >= column)
		{
			this->currentIndex = column;
			this->savedIndex = column;
		}
		else
		{
			this->currentIndex = 0;
			this->savedIndex = 0;
		}
	}
}

void Cedit::event_cut()
{
	auto it = this->contentIt;

	this->paste_buffer.clear();

	if (it->length() != 0) {
		*it = it->substr(0, it->length() - 1); // \n
		this->paste_buffer.push_back(*it);
	}

	if (it == std::prev(this->content.end()))
	{
		if (it == this->content.begin())
		{
			it->clear();
		}

		return;
	}

	this->event_down();

	this->content.erase(it);
}

void Cedit::event_paste()
{
	this->contentIt->insert(this->currentIndex, *this->paste_buffer.begin());
	this->currentIndex += this->paste_buffer.begin()->length();
}

void Cedit::display()
{
	this->display_header();
	this->display_content();

	wbkgd(this->menu.window, A_REVERSE);
	wmove(this->menu.window, 0, 0);

	wrefresh(this->menu.window);
}

void Cedit::display_header()
{
	const std::string filename = this->filename.empty() ? NEW_FILE : this->filename;

	if(this->refreshHeader)
	{
		werase(this->wheader);
		wbkgd(this->wheader, A_REVERSE);
		mvwprintw(this->wheader, 0, 2, VERSION);
		mvwprintw(this->wheader, 0, this->width / 2 - filename.size() / 2, "%s", filename.c_str());

		wrefresh(this->wheader);

		this->refreshHeader = false;
	}
}

void Cedit::display_current_line()
{
	bool isLastline =	this->contentIt != std::prev(this->content.end()) &&
						this->contentIt == std::prev(this->displayLastIt()) &&
						this->contentIt->at(this->contentIt->length() - 1) == '\n';

	if(isLastline)
	{
		this->contentIt->pop_back();
	}

	this->cursorY = distance(this->displayFirstIt(), this->contentIt);

	wmove(this->wcontent, this->cursorY, 0);

	this->display_linenumbers(this->contentIt);

	getyx(this->wcontent, this->cursorY, this->cursorXReset);

	for(std::size_t i = 0; i < this->contentIt->length(); i++)
	{
		if(i == this->currentIndex)
		{
			getyx(this->wcontent, this->cursorY, this->cursorX);
			wprintw(this->wcontent, "%c", this->contentIt->at(i));
			continue;
		}

		wprintw(this->wcontent, "%c", this->contentIt->at(i));
	}

	if(this->contentIt->length() == 0 && !this->showLineNumbers)
	{
		this->cursorX = 0;
	}
	else if(this->contentIt->length() == this->currentIndex)
	{
		getyx(this->wcontent, this->cursorY, this->cursorX);
	}

	wmove(this->wcontent, this->cursorY, this->cursorXReset);
	wclrtoeol(this->wcontent);

	wprintw(this->wcontent, "%s", this->contentIt->c_str());

	if(isLastline)
	{
		this->contentIt->append("\n");
	}

	wmove(this->wcontent, this->cursorY, this->cursorX);
}

void Cedit::display_linenumbers(std::list<std::string>::iterator it)
{
	if(this->showLineNumbers)
	{
		const auto itBegin = this->displayFirstIt();

		std::stringstream ss;
		ss <<  " " << std::setw(3) << std::distance(itBegin, it) + this->entryLine + 1;

		wattron(this->wcontent, A_REVERSE);
		wprintw(this->wcontent, "%s", ss.str().c_str());
		wattroff(this->wcontent, A_REVERSE);

		wprintw(this->wcontent, " ");
	}
}

void Cedit::display_content()
{
	werase(this->wcontent);

	const auto itBegin = this->displayFirstIt();
	const auto itEnd = this->displayLastIt();

	for(auto it = itBegin; it != itEnd; it++)
	{
		this->display_linenumbers(it);

		if(this->contentIt == it)
		{
			getyx(this->wcontent, this->cursorY, this->cursorXReset);
			this->display_current_line();
			wmove(this->wcontent, this->cursorY, this->cursorXReset);
		}

		// Last line to print to screen
		if(it != std::prev(this->content.end()) && it == std::prev(itEnd) && it->at(it->length() - 1) == '\n')
		{
			wprintw(this->wcontent, "%s", it->substr(0, it->length() - 1).c_str());
		}
		else
		{
			wprintw(this->wcontent, "%s", it->c_str());
		}

		if(this->contentIt->length() == 0 && !this->showLineNumbers)
		{
			this->cursorX = 0;
		}
		else if(this->contentIt->length() == this->currentIndex)
		{
			getyx(this->wcontent, this->cursorY, this->cursorX);
		}
	}

	wmove(this->wcontent, this->cursorY, this->cursorX);
}

void Cedit::window_print_color(WINDOW* window, short color, std::string line)
{
	wattron(window, COLOR_PAIR(color));
	wprintw(window, "%s", line.c_str());
	wattroff(window, COLOR_PAIR(color));
}

}
