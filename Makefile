clibean: http.o main.o
	c++ http.o main.o -o clibean

http.o: http.cpp http.h
	c++ -c http.cpp

main.o: main.cpp
	c++ -c main.cpp

clean:
	rm *.o clibean