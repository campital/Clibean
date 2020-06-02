SRC := $(wildcard *.cpp) $(wildcard ui/*.cpp)
OBJ := $(SRC:.cpp=.o)
CXXFLAGS := -Os
LDLIBS := -lssl -lcrypto

clibean: $(OBJ)
	$(CXX) -o clibean $(OBJ) $(LDLIBS)

main.o: main.cpp http.h netUtil.h ui/baseUI.h ui/basicUI.h
	$(CXX) $(CXXFLAGS) -c -o main.o main.cpp

http.o: http.cpp http.h netUtil.h
	$(CXX) $(CXXFLAGS) -c -o http.o http.cpp

netUtil.o: netUtil.cpp netUtil.h
	$(CXX) $(CXXFLAGS) -c -o netUtil.o netUtil.cpp

ui/basicUI.o: ui/basicUI.cpp ui/baseUI.h ui/basicUI.h
	$(CXX) $(CXXFLAGS) -c -o ui/basicUI.o ui/basicUI.cpp

clean:
	rm clibean *.o ui/*.o