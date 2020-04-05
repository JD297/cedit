output: main.o cedit.o
	g++ -std=c++17 -g -O1 -Wall -Wextra -Wpedantic obj/main.o obj/cedit.o -lncurses -o cedit

main.o:
	g++ -std=c++17 -g -O1 -Wall -Wextra -Wpedantic -c src/main.cpp -o obj/main.o

cedit.o:
	g++ -std=c++17 -g -O1 -Wall -Wextra -Wpedantic -c src/cedit.cpp -o obj/cedit.o

clean:
	rm obj/*.o cedit 2>/dev/null

install: output
	sudo cp cedit /usr/bin/cedit

uninstall:
	sudo rm /usr/bin/cedit

test: output
	./cedit
