#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need SDL2 (http://www.libsdl.org):
# Linux:
#   apt-get install libsdl2-dev
# Mac OS X:
#   brew install sdl2
# MSYS2:
#   pacman -S mingw-w64-i686-SDL
#

#CXX = g++
#CXX = clang++

EXE = AnimatorsPlace
SOURCES = main.cpp imgui_impl_sdl.cpp imgui_impl_opengl2.cpp
SOURCES += imgui.cpp imgui_draw.cpp imgui_widgets.cpp

OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

LIBS = -lmingw32 -lgdi32 -lopengl32 -lglu32 -lSDL2main -lSDL2 -lSDL2_image -limm32 

CXXFLAGS = -Wall -Wformat -static-libgcc -static-libstdc++ 
#-mwindows
CFLAGS = $(CXXFLAGS)


%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:../%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:../../%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
