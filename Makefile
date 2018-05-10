# include ../agg-2.5/Makefile.in.$(shell uname).SDL
AGGLIBS= -lagg  -L/usr/local/lib -lSDLmain -lSDL -framework Cocoa -framework OpenGL
AGGCXXFLAGS = -O3 -I/Library/Frameworks/SDL.framework/Headers -L/usr/lib
CXX = g++
C = gcc
#CXX = icc
LIB = ar cr -s

PLATFORM=sdl

PLATFORMSOURCES=./agg-2.5/src/platform/$(PLATFORM)/agg_platform_support.o

CXXFLAGS= $(AGGCXXFLAGS) -I./agg-2.5/include \
-I/usr/local/include/SDL \
-L./agg-2.5/src \
$(PIXFMT)

CXXFREETYPEFLAGS= $(AGGCXXFLAGS) -Wall \
-I./agg-2.5/include \
-I./agg-2.5/font_freetype \
-I/usr/local/include/freetype2 \
-L./agg-2.5/src \
$(PIXFMT)

LIBS = $(AGGLIBS) -lm

canvas: ./canvas.o $(PLATFORMSOURCES) 
	$(CXX) $(CXXFLAGS) $^ -o canvas $(LIBS)

clean:
	rm -f ./*.o

all:
	make clean
	make canvas

.PHONY : clean