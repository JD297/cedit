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
		if(!this->menu.type("Dateiname zum Speichern [von ./]: ")) {
			return;
		}

		this->filename = this->menu.content;

		this->refreshHeader = true;
	}

	// TODO error checking
	std::ofstream output;
	output.open(this->filename.c_str());

	if(auto it = std::prev(this->content.end()); !it->empty())
	{
		it->append("\n");
		this->content.emplace_back(""); // REMOVE ???
	}

	for(auto it = this->content.begin(); it != std::prev(this->content.end()); it++)
	{
		output << *it;
	}

	output.close();

	std::stringstream menu_text;
	menu_text << content.size()-1 << " " << SAVE_MESSAGE;

	this->menu.display(&menu_text);
}

void Cedit::event_load(const char* filename)
{
	this->filename = std::string(filename);

	std::ifstream f(this->filename, std::ios::in);

	this->content.clear();

	this->refreshHeader = true;

	std::string message = "";

	// Not a regular file
	if(false) {
		this->filename = "";

		message = "[ " + std::string(filename) + " is not a regular file ]";
	}

	// File error
	if(!f.good())
	{
		if(errno)
		{
			this->filename = "";

			message = "[ Fehler beim Lesen von: " + std::string(filename) + " : " + std::strerror(errno)  + " ]";
		}
		else
		{
			message = "[ Neue Datei ]";
		}

		wattron(this->wmenu, A_REVERSE);
		menu_print_clear(message.c_str(), (size_t)(this->width / 2 - message.length() / 2), 0);
		wattroff(this->wmenu, A_REVERSE);

		return;
	}

	while(!f.eof())
	{
		std::string temp;
		getline(f, temp);

		this->content.emplace_back(temp+"\n");
	}
	f.close();

	this->content.rbegin()->clear();

	this->contentIt = this->content.begin();

	if(this->brain.find(this->filename) != this->brain.end())
	{
		std::advance(this->contentIt, this->brain[this->filename].contentIndex);
		this->currentIndex = this->brain[this->filename].currentIndex;
		this->savedIndex = this->brain[this->filename].savedIndex;
		this->entryLine = this->brain[this->filename].entryLine;
		this->showLineNumbers = this->brain[this->filename].showLineNumbers;
	}

	std::stringstream menu_text;
	menu_text << this->content.size()-1 << " " << LOAD_MESSAGE;

	this->menu.display(&menu_text);
}

void Cedit::event_open()
{
	if(!this->menu.type(FILE_OPEN_MESSAGE)) {
		return;
	}

	this->brain[this->filename] = {
		this->content,
		std::distance(this->content.begin(), this->contentIt),
		this->currentIndex,
		this->savedIndex,
		this->entryLine,
		this->showLineNumbers
	};

	this->reset();

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

void Cedit::scrollup(size_t n)
{
	if(this->contentIt == this->displayFirstIt() && n == 1)
	{
		this->entryLine--;
		wscrl(this->wcontent, -1);
	}
}

void Cedit::scrolldown(size_t n)
{
	if(this->contentIt == this->displayLastIt() && n == 1)
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
}

void Cedit::menu_reset()
{
        wbkgd(this->wmenu, A_NORMAL);
        wclear(this->wmenu);
        wrefresh(this->wmenu);
}

void Cedit::menu_print(const std::string text, const size_t x = 0, const size_t y = 0)
{
	mvwprintw(this->wmenu, y, x, "%s", text.c_str());

	wrefresh(this->wmenu);
}

void Cedit::menu_print_clear(const std::string text, const size_t x = 0, const size_t y = 0)
{
	wclear(this->wmenu);

	mvwprintw(this->wmenu, y, x, "%s", text.c_str());

	wrefresh(this->wmenu);
}

void Cedit::display_header()
{
	const std::string filename = this->filename.empty() ? "Neue Datei" : this->filename;

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

void Cedit::display_content()
{
	wclear(this->wcontent);

	const auto itBegin = this->displayFirstIt();
	const auto itEnd = this->displayLastIt();

	for(auto it = itBegin; it != itEnd; it++)
	{
		// Zeilenangabe ausgeben, wenn aktiviert strg+l
		if(this->showLineNumbers)
		{
			std::stringstream ss;
			ss <<  " " << std::setw(3) << distance(itBegin, it) + entryLine + 1;

			wattron(this->wcontent, A_REVERSE);
			wprintw(this->wcontent, ss.str().c_str());
			wattroff(this->wcontent, A_REVERSE);

			wprintw(this->wcontent, " ");
		}

		if(this->contentIt == it)
		{
			getyx(this->wcontent, this->cursorY, this->cursorXReset);

			for(std::size_t i = 0; i < it->length(); i++)
			{
				if(i == this->currentIndex)
				{
					getyx(this->wcontent, this->cursorY, this->cursorX);
					wprintw(this->wcontent, "%c", it->at(i));
					continue;
				}

				wprintw(this->wcontent, "%c", it->at(i));
			}

			// next print will override entire line this is important for syntax highlighting
			wmove(this->wcontent, this->cursorY, this->cursorXReset);
			wclrtoeol(this->wcontent);
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
	// syntax key words regex
	std::regex rgx("(const|void|[^a-zA-Z0-9]int[^a-zA-Z0-9]|return|if|else|while|for|class|namespace|char|switch|case|break|auto)");

	// syntax strings regex
	// std::regex rgx("\"([^\"]*)\"");

	std::regex_iterator<std::string::iterator> it(line.begin(), line.end(), rgx);
	std::regex_iterator<std::string::iterator> end;

	std::size_t from = 0;
	std::size_t to = 0;

	for(; it != end; it++)
	{
		to = line.find(it->str(), from);

		wprintw(this->wcontent, "%s", line.substr(from, to - from).c_str());

		this->window_print_color(this->wcontent, COLOR_MAGENTA, it->str());

		from = to + it->str().length();
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
