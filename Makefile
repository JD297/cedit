TARGET=bin/cedit
SRC=src
OBJ=obj
SRC_FILES=$(wildcard $(SRC)/*.cpp)
OBJ_FILES=$(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(SRC_FILES))

CXX=g++
CXXFLAGS=-std=c++17 -O3 -Wall -Wextra -Wpedantic
CXXLIBS=-lncurses

$(TARGET): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(OBJ_FILES) $(CXXLIBS) -o $(TARGET)

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ_FILES) $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/bin

uninstall:
	rm -f /usr/$(TARGET)
