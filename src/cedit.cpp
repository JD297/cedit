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
	filename("Neuer Puffer")
{
	this->wheader = newwin(1, this->width, 0, 0);
	this->wcontent = newwin(this->height, this->width, 2, 0);
	this->wmenu = newwin(1, this->width, this->rheight-3, 0);
	this->wfooter = newwin(2, this->width, this->rheight-2, 0);

	this->content.emplace_back("");
	this->contentIt = this->content.begin();

	keypad(this->wcontent, true);
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
		this->display();

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
			case 263:
				this->event_backspace();
			break;
			case 330:
				this->event_delete();
			break;
			case 260:
				this->event_left();
			break;
			case 261:
				this->event_right();
			break;
			case 259:
				this->event_up();
			break;
			case 258:
				this->event_down();
			break;
			case 338:
				this->event_pagedown();
			break;
			case 339:
				this->event_pageup();
			break;
			default:
				this->event_write();
			break;
		}
	}
}

void Cedit::event_save()
{
	std::ofstream output;
	output.open(this->filename.c_str());

	auto it = std::prev(this->content.end());

	if(!it->empty())
	{
		it->append("\n");
		this->content.emplace_back("");
	}

	for(auto it = this->content.begin(); it != std::prev(this->content.end()); it++)
	{
		output << *it;
	}

	output.close();

	std::stringstream menu_text;
	menu_text << "[ " << content.size()-1 << " Zeilen geschrieben ]";

	wattron(wmenu, A_REVERSE);
	menu_print_clear(menu_text.str(), (size_t)(this->width / 2 - menu_text.str().size() / 2), 0);
	wattroff(wmenu, A_REVERSE);
}

void Cedit::event_load(const char* filename)
{
	this->filename = std::string(filename);

	std::ifstream f(this->filename, std::ios::in);

	this->reset();

	if(!f.good())
	{
		std::string message = "";
		if(errno == 13)
		{
			this->filename = "Neuer Puffer";
			this->refreshHeader = true;

			message = "[ Fehler beim Lesen von " + std::string(filename) + ": Keine Berechtigung ]";
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

	this->content.clear();

	while(!f.eof())
	{
		std::string temp;
		getline(f, temp);

		this->content.emplace_back(temp+"\n"); 
	}
	f.close();

	this->content.rbegin()->clear();

	this->contentIt = this->content.begin();

	std::stringstream menu_text;
	menu_text << "[ " << this->content.size()-1 << " Zeilen gelesen ]";

	wattron(this->wmenu, A_REVERSE);
	menu_print_clear(menu_text.str(), (size_t)(this->width / 2 - menu_text.str().size() / 2), 0);
	wattroff(this->wmenu, A_REVERSE);
}

void Cedit::event_open()
{
	wbkgd(this->wmenu, A_REVERSE);

	this->key = 0;
	bool run = true;

	std::string filename = "";

	while(run)
	{
		const std::string promt = "Dateiname zum öffnen [von ./]: ";

		this->menu_print_clear(promt, 0, 0);
		this->menu_print(filename, promt.length(), 0);

		this->key = wgetch(this->wmenu);

		switch(key)
		{
			case 3:
				run = false;
				this->menu_reset();
			break;
			case 10:
				wbkgd(this->wmenu, A_NORMAL);

				this->refreshHeader = true;

				this->filename = filename;
				this->event_load(this->filename.c_str());

				run = false;
			break;
			default:

			break;
		}

		filename = filename + (char)key;
	}
}

void Cedit::event_write()
{
	this->contentIt->insert(this->currentIndex, 1, (char)key);

	if(this->key == 10)
	{
		if(this->currentIndex < this->contentIt->length()-1)
		{
			this->content.insert(std::next(this->contentIt), this->contentIt->substr(this->currentIndex+1));

			*this->contentIt = this->contentIt->substr(0, this->currentIndex+1);
		}
		else
		{
			this->content.insert(std::next(this->contentIt), "");
		}

		this->contentIt = std::next(this->contentIt);

		this->currentIndex = 0;
		this->savedIndex = 0;
	}
	else
	{
		this->currentIndex++;
		this->savedIndex = this->currentIndex;
	}
}

void Cedit::event_backspace()
{
	if(this->currentIndex == 0 && this->contentIt != this->content.begin())
	{
		if(std::prev(this->contentIt)->length() != 1)
		{
			this->contentIt = std::prev(this->contentIt);

			this->currentIndex = this->contentIt->length()-1;
			this->savedIndex = this->contentIt->length()-1;

			if(this->contentIt->length() > 0) {
				this->contentIt->append(std::next(this->contentIt)->substr(0));

				this->contentIt->erase(this->contentIt->find("\n"), 1);
			}

			this->content.erase(std::next(this->contentIt));
		}
		else if(std::prev(this->contentIt)->length() == 1)
		{
			this->content.erase(std::prev(this->contentIt));

			this->currentIndex = 0;
			this->savedIndex = 0;
		}
	}
	else if(!(this->currentIndex == 0 && this->contentIt == this->content.begin()))
	{
		this->contentIt->erase(this->currentIndex-1, 1);

		this->currentIndex--;
		this->savedIndex = this->currentIndex;
	}
}

void Cedit::event_delete() { }

void Cedit::event_up()
{
	if(this->contentIt != this->content.begin())
	{
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
	if(this->contentIt != std::prev(this->content.end()))
	{
		this->contentIt = std::next(this->contentIt);

		if(this->contentIt->length() < this->savedIndex && this->contentIt == std::prev(this->content.end()))
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
		this->contentIt = std::prev(this->contentIt);

		this->currentIndex = this->contentIt->length()-1;
		this->savedIndex = this->currentIndex;
	}
}

void Cedit::event_right()
{
	if(
		this->currentIndex < this->contentIt->length()-1 || (
		this->currentIndex <= this->contentIt->length()-1 &&
		this->contentIt == std::prev(this->content.end())
	))
	{
		this->currentIndex++;
		this->savedIndex = this->currentIndex;
	}
	else if(this->contentIt != std::prev(this->content.end()))
	{
		this->contentIt = std::next(this->contentIt);

		this->currentIndex = 0;
		this->savedIndex = 0;
	}
}

void Cedit::event_pagedown()
{
	if(this->entryLine >= this->content.size()-height)
	{
		return;
	}

	this->entryLine += 10;

	advance(this->contentIt, 10);

	this->currentIndex = 0;
	this->savedIndex = 0;

	wrefresh(this->wcontent);
}

void Cedit::event_pageup()
{
	if(this->entryLine <= 0)
	{
		return;
	}

	this->entryLine -= 10;

	advance(this->contentIt, -10);

	this->currentIndex = 0;
	this->savedIndex = 0;

	wrefresh(this->wcontent);
}

void Cedit::display()
{
	this->display_header();
	this->display_content();
	this->display_footer();
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
	if(this->refreshHeader)
	{
		wclear(this->wheader);
		wbkgd(this->wheader, A_REVERSE);
		mvwprintw(this->wheader, 0, 2, VERSION);
		mvwprintw(this->wheader, 0, this->width / 2 - this->filename.size() / 2, this->filename.c_str());

		wrefresh(this->wheader);

		this->refreshHeader = false;
	}
}

void Cedit::display_content()
{
	wclear(this->wcontent);

	const auto itBegin = std::next(this->content.begin(),this->entryLine);

	const auto itEnd = [](size_t e, size_t h, auto b, auto& c){
		return e + h >= c.size() ? c.end() : std::next(b, h);
	}(this->entryLine, this->height, itBegin, this->content);

	for(auto it = itBegin; it != itEnd; it++)
	{
		if(this->contentIt == it)
		{
			for(std::size_t i = 0; i < it->length(); i++)
			{
				if(i == this->currentIndex)
				{
					if(it->at(i) == '\n') {
						wattron(this->wcontent, A_REVERSE);
						wprintw(this->wcontent, "%c", ' ');
						wattroff(this->wcontent, A_REVERSE);
					}

					wattron(this->wcontent, A_REVERSE);
					wprintw(this->wcontent, "%c", it->at(i));
					wattroff(this->wcontent, A_REVERSE);

					continue;
				}

				wprintw(this->wcontent, "%c", it->at(i));
			}

			if(it->length() <= this->currentIndex)
			{
				wattron(this->wcontent, A_REVERSE);
				wprintw(this->wcontent, "%c", ' ');
				wattroff(this->wcontent, A_REVERSE);
			}
		}
		else
		{
			wprintw(this->wcontent, "%s", it->c_str());
		}
	}
}

void Cedit::display_help(std::string key, std::string text)
{
	wattron(this->wfooter, A_REVERSE);
	wprintw(this->wfooter , key.c_str());
	wattroff(this->wfooter, A_REVERSE);
	wprintw(this->wfooter, text.c_str());
}

void Cedit::display_footer()
{
	if(this->refreshDisplay)
	{
		this->display_help("Esc", " Beenden      ");
		this->display_help("^S" , " Speichern      ");
		this->display_help("^O" , " Datei öffnen      ");

		wrefresh(this->wfooter);

		this->refreshDisplay = false;
	}
}

}
