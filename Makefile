TARGET=cedit

CC=g++
CFLAGS=-std=c++17 -O3 -Wall -Wextra -Wpedantic
CLIBS=-lncurses

OBJS=main.o cedit.o window.o menu.o
OBJSP=./obj/main.o ./obj/cedit.o ./obj/window.o ./obj/menu.o 

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJSP) $(CLIBS) -o $(TARGET)

main.o:
	$(CC) $(CFLAGS) -c ./src/main.cpp -o ./obj/main.o

cedit.o:
	$(CC) $(CFLAGS) -c ./src/cedit.cpp -o ./obj/cedit.o

window.o:
	$(CC) $(CFLAGS) -c ./src/window.cpp -o ./obj/window.o

menu.o:
	$(CC) $(CFLAGS) -c ./src/menu.cpp -o ./obj/menu.o

clean:
	$(RM) $(TARGET) $(OBJSP)
