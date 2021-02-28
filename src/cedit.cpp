#include "cedit.hpp"

namespace cedit {

Cedit::Cedit():
	currentIndex(0),
	savedIndex(0),
	entryLine(0),
	key(0),
	refreshDisplay(true),
	refreshHeader(true),
	isrunning(true),
	filename(""),
	showLineNumbers(false)
{
	this->wheader = newwin(1, this->width, 0, 0);
	this->wcontent = newwin(this->height, this->width, 1, 0);
	this->wmenu = newwin(1, this->width, this->rheight - 1, 0);

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
		if(this->refreshDisplay)
			this->display();
		else
			this->display_current_line();

		this->refreshDisplay = true;

		this->key = wgetch(this->wcontent);

		switch(this->key)
		{
			case 19:
				this->event_save();
			break;
			case 15:
				this->event_open();
			break;
			case 27:
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
			case 12:
				this->event_toggle_linenumbers();
			break;
			case 7:
				this->event_goto();
			break;
			default:
				this->event_write();
			break;
		}
	}
}

void Cedit::event_save()
{
	if(this->filename.empty())
	{
		if(!this->menu.type(FILE_SAVE)) {
			return;
		}

		this->filename = this->menu.content;

		this->refreshHeader = true;
	}

	std::ofstream f(this->filename.c_str());

	auto itEnd = std::prev(this->content.end());

	if(!itEnd->empty())
	{
		itEnd->append("\n");
		this->content.emplace_back("");
	}

	for(auto it = this->content.begin(); it != this->content.end(); it++)
	{
		f << *it;
	}

	f.close();

	if(!f)
	{
		if(errno)
		{
			this->menu.display(std::string(FAIL_WRITE) + std::string("\"") + std::string(filename) + "\": " + std::strerror(errno), COLOR_RED);
		}

		return;
	}

	this->menu.display(std::to_string(content.size()-1) + " " + SAVE);
}

void Cedit::event_load(const char* filename)
{
	if(!this->filename.empty())
	{
		this->session[this->filename] = {
			this->content,
			std::distance(this->content.begin(), this->contentIt),
			this->currentIndex,
			this->savedIndex,
			this->entryLine,
			this->showLineNumbers
		};
	}

	this->reset();

	std::string message = "";
	std::string message_filename = "\"" + std::string(filename) + "\"";

	const auto fs_status = std::filesystem::status(filename);

	if(!std::filesystem::exists(fs_status))
	{
		this->filename = std::string(filename);
		this->refreshHeader = true;

		this->menu.display(NEW_FILE);

		return;
	}
	else if(std::filesystem::is_directory(fs_status)) {
		this->menu.display(message_filename + IS_DIRECTORY, COLOR_RED);

		return;
	}
	else if(std::filesystem::is_block_file(fs_status))
	{
		this->menu.display(message_filename + IS_BLOCK_FILE, COLOR_YELLOW);

		return;
	}
	else if(std::filesystem::is_character_file(fs_status))
	{
		this->menu.display(message_filename + IS_CHARACTER_FILE, COLOR_YELLOW);

		return;
	}
	else if(std::filesystem::is_fifo(fs_status))
	{
		this->menu.display(message_filename + IS_FIFO, COLOR_YELLOW);

		return;
	}
	else if(std::filesystem::is_socket(fs_status))
	{
		this->menu.display(message_filename + IS_SOCKET, COLOR_YELLOW);

		return;
	}
	else if(std::filesystem::is_symlink(fs_status))
	{
		this->menu.display(message_filename + IS_SYMLINK, COLOR_YELLOW);

		return;
	}
	else if(!std::filesystem::is_regular_file(fs_status))
	{
		this->menu.display(message_filename + IS_NOT_REGULAR_FILE, COLOR_YELLOW);

		return;
	}

	std::ifstream f(std::string(filename), std::ios::in);

	if(!f.good())
	{
		if(errno)
		{
			this->menu.display(std::string(FAIL_READ) + std::string("\"") + std::string(filename) + "\": " + std::strerror(errno), COLOR_RED);
		}
		else
		{
			this->menu.display(NEW_FILE);
		}

		this->filename = "";

		return;
	}

	this->filename = std::string(filename);

	this->refreshHeader = true;

	this->content.clear();

	while(!f.eof())
	{
		std::string line;
		getline(f, line);

		this->content.emplace_back(line+"\n");
	}

	f.close();

	this->content.rbegin()->clear();

	this->contentIt = this->content.begin();

	if(this->session.find(this->filename) != this->session.end())
	{
		std::advance(this->contentIt, this->session[this->filename].contentIndex);
		this->currentIndex = this->session[this->filename].currentIndex;
		this->savedIndex = this->session[this->filename].savedIndex;
		this->entryLine = this->session[this->filename].entryLine;
		this->showLineNumbers = this->session[this->filename].showLineNumbers;
	}

	this->menu.display(std::to_string(this->content.size()-1) + " " + LOAD);
}

void Cedit::event_open()
{
	if(!this->menu.type(FILE_OPEN)) {
		this->refreshDisplay = false;
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

	// Entertaste
	if(this->key == 10)
	{
		if(this->currentIndex < this->contentIt->length()-1)
		{
			// Neue Zeile mit Inhalt nach dem Cursour in neue Zeile einfügen
			this->content.insert(std::next(this->contentIt), this->contentIt->substr(this->currentIndex+1));

			// Kürzen der Aktuellen Zeile bis vor dem Cursour
			*this->contentIt = this->contentIt->substr(0, this->currentIndex+1);
		}
		else
		{
			//Neue leere Zeile hinter aktueller Zeile einfügen
			this->content.insert(std::next(this->contentIt), "");
		}

		//In die neu erstellte Zeile Springen
		this->contentIt = std::next(this->contentIt);

		this->currentIndex = 0;
		this->savedIndex = 0;
	}
	else if((this->key >= 32 && this->key <= 126) || this->key == 9)
	{
		this->currentIndex++;
		this->savedIndex = this->currentIndex;
		this->refreshDisplay = false;
	}
}

void Cedit::event_backspace()
{
	// Wenn am Anfang der Zeile und aktuelle Zeile ist nicht die Erste Zeile
	if(this->currentIndex == 0 && this->contentIt != this->content.begin())
	{
		this->scrollup();

		// check ob die Zeile die gelöscht werden soll Inhalt enthält
		if(std::prev(this->contentIt)->length() != 1)
		{
			// Springe in die vorherige Zeile
			this->contentIt = std::prev(this->contentIt);

			this->currentIndex = this->contentIt->length()-1;
			this->savedIndex = this->contentIt->length()-1;

			if(this->contentIt->length() > 0) {
				// Füge die gesammte Zeile an die vorherige Zeile an
				this->contentIt->append(std::next(this->contentIt)->substr(0));

				// Den ersten Absatz aus der aktuellen Zeile entfernen
				this->contentIt->erase(this->contentIt->find("\n"), 1);
			}

			// Lösche die Zeile von wo aus kopiert wurde aus der Liste
			this->content.erase(std::next(this->contentIt));
		}
		else if(std::prev(this->contentIt)->length() == 1)
		{
			// Lösche die vorherige Zeile mit "\n", so dass die aktuelle Zeile nach oben verschoben wird
			this->content.erase(std::prev(this->contentIt));

			this->currentIndex = 0;
			this->savedIndex = 0;
		}
	}
	else if(!(this->currentIndex == 0 && this->contentIt == this->content.begin()))
	{
		// Buchstaben löschen der sich vor dem Cursor befindet
		this->contentIt->erase(this->currentIndex-1, 1);

		this->currentIndex--;
		this->savedIndex = this->currentIndex;
		this->refreshDisplay = false;
	}
}

void Cedit::event_delete()
{
	// letztes zeichen und letzte zeile
	if(this->currentIndex == this->contentIt->length() && this->content.end() == std::next(this->contentIt))
	{
		return;
	}
	// zeilen ende: copy next line to this line, delete next line
	else if(this->currentIndex == this->contentIt->length()-1 && this->contentIt->back() == '\n')
	{
		// Ueberprüfe ob die Zeile die geloescht werden soll Inhalt enthält
		// bzw die zeile die kopiert werden soll
		if(std::next(this->contentIt)->length() != 1)
		{
			// entferne den Absatz
			this->contentIt->erase(this->currentIndex, 1);

			// Fuege die gesammte naechte Zeile an die aktuelle Zeile an
			this->contentIt->append(std::next(this->contentIt)->substr(0));

			// Loesche die neachte zeile, von wo aus kopiert wurde
			this->content.erase(std::next(this->contentIt));
		}
		else if(std::next(this->contentIt)->length() == 1)
		{
			// Lösche die neachste Zeile
			this->content.erase(std::next(this->contentIt));
		}
	}
	else
	{
		// delete character
		this->contentIt->erase(this->currentIndex, 1);
		this->refreshDisplay = false;
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
	this->refreshDisplay = false;

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
	this->refreshDisplay = false;

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
	this->refreshDisplay = false;

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
	this->refreshDisplay = false;

	const auto itEnd = std::prev(this->content.end());

	if(this->currentIndex < this->contentIt->length()-1 ||
	  (this->currentIndex < this->contentIt->length() && contentIt == itEnd)
	)
	{
		if(currentIndex == this->contentIt->length() && this->contentIt->back() != '\n')
		{
			return;
		}

		this->refreshDisplay = false;
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

		// compares long int with size_t aka unsigned long int, type cast is required
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
	this->refreshDisplay = false;
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

	this->refreshDisplay = false;
}

void Cedit::event_toggle_linenumbers()
{
	this->showLineNumbers = !this->showLineNumbers;
}

void Cedit::event_goto()
{
	if(!this->menu.type("Springe zu [Zeile Spalte]: ")) {
		this->refreshDisplay = false;
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

void Cedit::display()
{
	this->display_header();
	this->display_content();

	wbkgd(this->menu.window, A_REVERSE);
	wmove(this->menu.window, 0, 0);

	for(auto it = this->session.begin(); it != session.end(); it++) {
		std::string end = this->filename == it->first ? "* " : "  ";

		std::string sessionItem = "[" + std::to_string(std::distance(this->session.begin(), it)) + "]" + it->first + end;

		wprintw(this->menu.window, sessionItem.c_str());
	}

	wrefresh(this->menu.window);
}

void Cedit::display_header()
{
	const std::string filename = this->filename.empty() ? NEW_FILE : this->filename;

	if(this->refreshHeader)
	{
		wclear(this->wheader);
		wbkgd(this->wheader, A_REVERSE);
		mvwprintw(this->wheader, 0, 2, VERSION);
		mvwprintw(this->wheader, 0, this->width / 2 - filename.size() / 2, filename.c_str());

		wrefresh(this->wheader);

		this->refreshHeader = false;
	}
}

void Cedit::display_current_line()
{
	bool isLastline = this->contentIt != std::prev(this->content.end()) && this->contentIt == std::prev(this->displayLastIt()) && this->contentIt->at(this->contentIt->length() - 1) == '\n';

	if(isLastline)
	{
		this->contentIt->pop_back();
	}

	this->cursorY = distance(this->displayFirstIt(), this->contentIt);

	wmove(this->wcontent, this->cursorY, 0);

	this->display_linenumbers(this->contentIt);

	getyx(this->wcontent, this->cursorY, this->cursorXReset);
	wmove(this->wcontent, this->cursorY, this->cursorXReset);

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

	if(this->contentIt->length() == 0)
	{
		this->cursorX = 0;
	}
	else if(this->contentIt->length() == this->currentIndex)
	{
		getyx(this->wcontent, this->cursorY, this->cursorX);
	}

	wmove(this->wcontent, this->cursorY, this->cursorXReset);
	wclrtoeol(this->wcontent);

	this->display_syntax_content(*this->contentIt);

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
		wprintw(this->wcontent, ss.str().c_str());
		wattroff(this->wcontent, A_REVERSE);

		wprintw(this->wcontent, " ");
	}
}

void Cedit::display_content()
{
	wclear(this->wcontent);

	const auto itBegin = this->displayFirstIt();
	const auto itEnd = this->displayLastIt();

	for(auto it = itBegin; it != itEnd; it++)
	{
		// Zeilenangabe ausgeben, wenn aktiviert strg+l
		this->display_linenumbers(it);

		if(this->contentIt == it)
		{
			getyx(this->wcontent, this->cursorY, this->cursorXReset);
			this->display_current_line();
			wmove(this->wcontent, this->cursorY, this->cursorXReset);
		}

		// last line to print to screen
		if(it != std::prev(this->content.end()) && it == std::prev(itEnd) && it->at(it->length() - 1) == '\n')
		{
			this->display_syntax_content(it->substr(0, it->length() - 1));
		}
		else
		{
			this->display_syntax_content(*it);
		}

		if(this->contentIt->length() == 0)
		{
			this->cursorX = 0;
		}
		else if(this->contentIt->length() == this->currentIndex)
		{
			getyx(this->wcontent, this->cursorY, this->cursorX);
		}
	}

	for(size_t i = 0; i < this->height - distance(itBegin, itEnd); i++)
	{
		this->window_print_color(this->wcontent, COLOR_BLUE, "\n~");
		// wprintw(this->wcontent, "\n~");
	}

	wmove(this->wcontent, this->cursorY, this->cursorX);
}

void Cedit::display_syntax_content(std::string line)
{
	std::list<Regex> rlist;

	std::string syntax_filename = std::string(getenv("HOME")) + "/.cedit/c.ceditrc";

	if(std::filesystem::exists(syntax_filename))
	{
		std::ifstream f(syntax_filename, std::ios::in);

		if(f.good())
		{
			while(!f.eof())
			{
				std::string temp;
				getline(f, temp);

				// found color syntax
				if(temp.find("color") == 0)
				{
					// prepare rule
					std::size_t first = temp.find("\"") + 1;
					std::size_t last = temp.rfind("\"");

					std::string rule = temp.substr(first, last - first);

					rule = std::regex_replace(rule, std::regex("<"), "b");
					rule = std::regex_replace(rule, std::regex(">"), "b");

					// prepare color
					std::size_t color_first = 6;
					std::size_t color_last  = temp.find(" ", color_first + 1);

					std::string color_string = temp.substr(color_first, color_last - color_first);

					short color = 0;

					if(color_string == "green")
					{
						color = COLOR_GREEN;
					}
					else if(color_string == "yellow")
					{
						color = COLOR_YELLOW;
					}
					else if(color_string == "magenta")
					{
						color = COLOR_MAGENTA;
					}
					else if(color_string == "cyan")
					{
						color = COLOR_CYAN;
					}
					else
					{
						color = COLOR_RED;
					}

					rlist.push_back(*(new Regex(rule, &line, color)));
				}
			}
		}

		f.close();
	}

	std::list<Syntax> slist;

	for(auto rlit = rlist.begin(); rlit != rlist.end(); rlit++)
	{
		std::size_t rfrom = 0;

		for(std::regex_iterator<std::string::iterator> end; rlit->it != end; rlit->it++)
		{
			rfrom = line.find(rlit->it->str(), rfrom);

			slist.push_back(*(new Syntax(
				rfrom,
				rlit->it->str(),
				rlit->color
			)));

			rfrom++;
		}
	}

	slist.sort();

	std::size_t from = 0, to = 0;

	for(auto it = slist.begin(); it != slist.end(); it++)
	{
		to = line.find(it->text, from);

		wprintw(this->wcontent, "%s", line.substr(from, to - from).c_str());

		this->window_print_color(this->wcontent, it->color, it->text);

		from = to + it->text.length();
	}

	wprintw(this->wcontent, "%s", line.substr(from).c_str());
}

void Cedit::window_print_color(WINDOW* window, short color, std::string line)
{
	wattron(window, COLOR_PAIR(color));
	wprintw(window, "%s", line.c_str());
	wattroff(window, COLOR_PAIR(color));
}

}
