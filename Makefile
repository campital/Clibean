SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)
CXXFLAGS := -Os
LDLIBS := -lssl -lcrypto

clibean: $(OBJ)
	$(CXX) -o clibean $(OBJ) $(LDLIBS)

main.o: main.cpp http.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

http.o: http.cpp http.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm clibean *.o