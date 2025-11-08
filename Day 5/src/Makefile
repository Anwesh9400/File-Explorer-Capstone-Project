CXX = g++
CXXFLAGS = -std=c++17 -Wall -I.

build/fileexplorer: main.cpp explorer.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) main.cpp explorer.cpp -o build/fileexplorer

run: build/fileexplorer
	./build/fileexplorer

clean:
	rm -rf build
