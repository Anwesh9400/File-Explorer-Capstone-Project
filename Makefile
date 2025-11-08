CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
SRC = src/main.cpp src/explorer.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = fileexplorer

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) $(OBJ)