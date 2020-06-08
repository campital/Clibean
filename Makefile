SRC := $(wildcard *.cpp) $(wildcard ui/*.cpp)
OBJ := $(SRC:.cpp=.o)
CXXFLAGS := -std=c++11 -Os
LDLIBS := -lssl -lcrypto

clibean: $(OBJ)
	$(CXX) -o clibean $(OBJ) $(LDFLAGS) $(LDLIBS)

main.o: main.cpp http.h netUtil.h ui/baseUI.h ui/basicUI.h session.h
	$(CXX) $(CXXFLAGS) -c -o main.o main.cpp

http.o: http.cpp http.h netUtil.h
	$(CXX) $(CXXFLAGS) -c -o http.o http.cpp

netUtil.o: netUtil.cpp netUtil.h
	$(CXX) $(CXXFLAGS) -c -o netUtil.o netUtil.cpp

session.o: session.cpp session.h netUtil.h ui/baseUI.h ui/basicUI.h http.h
	$(CXX) $(CXXFLAGS) -c -o session.o session.cpp

ui/basicUI.o: ui/basicUI.cpp ui/baseUI.h ui/basicUI.h
	$(CXX) $(CXXFLAGS) -c -o ui/basicUI.o ui/basicUI.cpp

clean:
	rm clibean *.o ui/*.o