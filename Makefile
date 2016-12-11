CXX=g++
CXXFLAGS=-Wall -g -std=c++14

TESTS=virus_genealogy_test.cc

.PHONY: all clean

all: $(TESTS:.cc=)

%.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $(@:.o=.cc)

virus_genealogy_test: virus_genealogy_test.o
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TESTS:.cc=) *.o
