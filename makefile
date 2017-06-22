

main: main.cpp bitmapobject.cpp
	g++ -v -g -pedantic -o main.exe main.cpp bitmapobject.cpp -lGdi32