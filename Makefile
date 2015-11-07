CXX      := g++
CXXFLAGS := -O2 -Wall -Wextra -Wno-deprecated-declarations -Wno-pmf-conversions
CPPFLAGS := -std=c++11 $(shell pkg-config gtk+-3.0 --cflags)
GTKLIBS  := $(shell pkg-config gtk+-3.0 --libs)

.PHONY: all clean

all: mem_tray.x

clean:
	-rm -f mem_tray.x mem_tray.o

mem_tray.x: mem_tray.o
	$(CXX) -o $@ $^ $(GTKLIBS)
