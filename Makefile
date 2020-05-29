SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)
CXXFLAGS := -Os
LDLIBS := -lssl -lcrypto

clibean: $(OBJ)
	$(CXX) $(LDLIBS) -o clibean $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm clibean *.o