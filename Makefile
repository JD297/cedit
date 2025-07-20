.POSIX:

CC            = c++
CFLAGS        = -std=c++17 -Wall -Wextra -Wpedantic -g
LDFLAGS       = -lcurses

TARGET        = cedit
PREFIX        = /usr/local
BINDIR        = $(PREFIX)/bin
MANDIR        = $(PREFIX)/share/man
SRCDIR        = src
BUILDDIR      = build

OBJFILES      = $(BUILDDIR)/cedit.o $(BUILDDIR)/main.o \
                $(BUILDDIR)/menu.o $(BUILDDIR)/window.o

HEADERS       = $(SRCDIR)/cedit.hpp $(SRCDIR)/menu.hpp \
                $(SRCDIR)/version.hpp $(SRCDIR)/window.hpp

$(BUILDDIR)/$(TARGET): $(OBJFILES)
	$(CC) -o $@ $(OBJFILES) $(LDFLAGS)

$(BUILDDIR)/main.o: $(HEADERS) $(SRCDIR)/main.cpp
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/main.cpp

$(BUILDDIR)/cedit.o: $(HEADERS) $(SRCDIR)/cedit.cpp
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/cedit.cpp

$(BUILDDIR)/menu.o: $(HEADERS) $(SRCDIR)/menu.cpp
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/menu.cpp

$(BUILDDIR)/window.o: $(HEADERS) $(SRCDIR)/window.cpp
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/window.cpp

clean:
	rm -f $(BUILDDIR)/*

install: $(BUILDDIR)/$(TARGET)
	cp $(BUILDDIR)/$(TARGET) $(BIN)/$(TARGET)

uninstall:
	rm -f $(BINDIR)/$(TARGET)
