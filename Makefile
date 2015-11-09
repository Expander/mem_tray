CXX      := g++
CXXFLAGS := -std=c++11 -O2 -Wall -Wextra -Wno-deprecated-declarations -Wno-pmf-conversions
CPPFLAGS := $(shell pkg-config gtk+-3.0 --cflags)
GTKLIBS  := $(shell pkg-config gtk+-3.0 --libs)

.PHONY: all clean

all: mem_tray.x

clean:
	-rm -f mem_tray.x mem_tray.o

mem_tray.x: mem_tray.o
	$(CXX) -o $@ $^ $(GTKLIBS)
