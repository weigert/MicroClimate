
OBJS = main.cpp

TP = TinyEngine

TINY = $(TP)/include/imgui/imgui.cpp $(TP)/include/imgui/imgui_demo.cpp $(TP)/include/imgui/imgui_draw.cpp $(TP)/include/imgui/imgui_widgets.cpp $(TP)/include/imgui/imgui_impl_opengl3.cpp $(TP)/include/imgui/imgui_impl_sdl.cpp
TINYLINK = -lX11 -lpthread -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lGL -lGLEW -lboost_serialization -lboost_system -lboost_filesystem

CC = g++ -std=c++17
COMPILER_FLAGS = -Wfatal-errors -O
LINKER_FLAGS = -I/usr/local/include -L/usr/local/lib -lnoise
OBJ_NAME = main

all: $(OBJS)
			$(CC) $(OBJS) $(TINY) $(COMPILER_FLAGS) $(LINKER_FLAGS) $(TINYLINK) -o $(OBJ_NAME)
