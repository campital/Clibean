SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)
CXXFLAGS := -Os
LDLIBS := -lssl -lcrypto

clibean: $(OBJ)
	$(CXX) -o clibean $(OBJ) $(LDLIBS)

main.o: main.cpp http.h netUtil.h
	$(CXX) $(CXXFLAGS) -c -o main.o main.cpp

http.o: http.cpp http.h netUtil.h
	$(CXX) $(CXXFLAGS) -c -o http.o http.cpp

netUtil.o: netUtil.cpp netUtil.h
	$(CXX) $(CXXFLAGS) -c -o netUtil.o netUtil.cpp

clean:
	rm clibean *.o