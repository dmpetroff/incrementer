TARGETS := $(patsubst %.cpp,%,$(wildcard *.cpp))

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

%: %.cpp
	$(CXX) -g -std=c++17 -Wall -O2 -o $@ -pthread $<
