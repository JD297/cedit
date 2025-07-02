.POSIX

CC            = c++
CFLAGS        = -std=c++17 -Wall -Wextra -Wpedantic -g
LDFLAGS       = -lncurses -lstdc++fs

TARGET        = cedit
PREFIX        = /usr/local
BINDIR        = $(PREFIX)/bin
MANDIR        = $(PREFIX)/share/man
SRCDIR        = src
BUILDDIR      = build

OBJFILES      = $(BUILDIR)/cedit.o $(BUILDIR)/main.o \
                $(BUILDIR)/menu.o $(BUILDIR)/window.o

HEADERS       = $(SRCDIR)/src/cedit.hpp $(SRCDIR)/src/menu.hpp \
                $(SRCDIR)/src/version.hpp $(SRCDIR)/src/window.hpp

$(BUILDDIR)/$(TARGET): $(OBJFILES)
	$(CC) -o $@ $(LDFLAGS) $(OBJFILES)

$(BUILDIR)/main.o: $(HEADERS) $(SRCDIR)/main.cpp
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/main.cpp

$(BUILDIR)/cedit.o: $(HEADERS) $(SRCDIR)/cedit.cpp
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/cedit.cpp

$(BUILDIR)/menu.o: $(HEADERS) $(SRCDIR)/menu.cpp
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/menu.cpp

$(BUILDIR)/window.o: $(HEADERS) $(SRCDIR)/window.cpp
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/window.cpp

clean:
	rm -f $(BUILDDIR)/*

install: $(BUILDDIR)/$(TARGET)
	cp $(BUILDDIR)/$(TARGET) $(BIN)/$(TARGET)

uninstall:
	rm -f $(BINDIR)/$(TARGET)
