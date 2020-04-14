#include "menu.hpp"

namespace cedit {

Menu::Menu() : Window(getmaxx(stdscr), 1, 0, getmaxy(stdscr)-3)
{
      
}

void Menu::display(const char* text) {
    wbkgd(this->window, A_REVERSE);  

    wclear(this->window);
    wprintw(this->window, text);

    mvwprintw(this->window, 0, std::strlen(text), "%s", this->content.c_str());

    wmove(this->window, 0, std::strlen(text) + this->currentIndex); // Cursour

	// Folgender Code wurde entfernt. Das Betriebssystem bietet bereits einen Cursour
    /*for(auto it = this->content.begin(); it != this->content.end(); it++) {
        if(std::distance(it, std::next(this->content.begin(), this->currentIndex))  == 0 ) {
            wattron(this->window, A_UNDERLINE);
            wprintw(this->window, "%c", *it);
            wattroff(this->window, A_UNDERLINE);
        } else {
            wprintw(this->window, "%c", *it);
        }
    }*/

    /*if(this->currentIndex == this->content.length()) {
        wattron(this->window, A_UNDERLINE);
        wprintw(this->window, "%c", ' ');
        wattroff(this->window, A_UNDERLINE);
    }*/
}

void Menu::event_insert() {
    if(this->key >= 32 && this->key <= 126) {
        this->content.insert(this->currentIndex, 1, (char)this->key);

        this->currentIndex++;
    }
}

void Menu::event_left() {
    if(this->currentIndex <= 0) {
        return;
    }

    this->currentIndex--;
}

void Menu::event_right() {
    if(this->content.length() <= this->currentIndex ) {
        return;
    }

    this->currentIndex++;
}

void Menu::event_backspace() {
    if(this->currentIndex <= 0) {
        return;
    }

    this->content.erase(this->currentIndex - 1, 1);
    this->currentIndex--;
}

void Menu::event_delete() {
    if(this->content.length() <= this->currentIndex ) {
        return;
    }

    this->content.erase(this->currentIndex, 1);
}

void Menu::event_pos_1() {
    this->currentIndex = 0;
}

void Menu::event_pos_end() {
    this->currentIndex = this->content.length();
}

// nodiscard: Rueckgabewert MUSS verarbeitet werden im Code(z.B if), wenn nicht
// warnt der Compiler.
// true: OK
// false: error or user terminated (strg+c)
[[nodiscard]] bool Menu::type(const char* text, const char* content) {
    curs_set(1);

    this->content = content;
    this->currentIndex = 0;

    while(true) {

        this->display(text);

        this->key = wgetch(this->window);

        switch(this->key) {
            case 10: // OK enter
                wbkgd(this->window, A_NORMAL);  
                this->window_clear_refresh();
                
                curs_set(0);
                return this->content.length() > 0;
            break;
            case 3: // abort strg+c
                wbkgd(this->window, A_NORMAL); 
                this->window_clear_refresh();

                curs_set(0);
                return false;
            break;
            case KEY_LEFT:
                this->event_left();
            break;
            case KEY_RIGHT:
                this->event_right();
            break;
            case KEY_BACKSPACE:
                this->event_backspace();
            break;
            case KEY_DC:
                this->event_delete();
            break;
            case KEY_HOME:
                this->event_pos_1();
            break;
            case KEY_END:
                this->event_pos_end();
            break;
            default:
                this->event_insert();
            break;
        }
    }

    return false;
}

}
