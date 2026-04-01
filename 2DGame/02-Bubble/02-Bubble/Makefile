CXX      = g++
CC       = gcc
CXXFLAGS = -std=c++17 -Wall -O2
CFLAGS   = -Wall -O2

# SOIL path (use absolute to avoid space issues)
SOIL_DIR = $(shell cd ../../../libs/Simple\ OpenGL\ Image\ Library/src && pwd)

# Include dirs
INCLUDES = -I"$(SOIL_DIR)"

# Libraries (system packages on Fedora: glew-devel glfw-devel mesa-libGL-devel)
LIBS = -lGLEW -lglfw -lGL

# Auto-detect SDL2 and SDL2_mixer — sound is optional
SDL2_OK := $(shell pkg-config --exists sdl2 sdl2_mixer 2>/dev/null && echo yes)
ifeq ($(SDL2_OK),yes)
  CXXFLAGS += -DUSE_SDL
  LIBS     += -lSDL2 -lSDL2_mixer
endif

# C++ source files for 02-Bubble
CPP_SRCS = \
  main.cpp \
  Game.cpp \
  Scene.cpp \
  Player.cpp \
  Sprite.cpp \
  Shader.cpp \
  ShaderProgram.cpp \
  Texture.cpp \
  TileMap.cpp \
  Enemy.cpp \

CPP_OBJS = $(CPP_SRCS:.cpp=.o)

# SOIL C object files
C_OBJS = SOIL.o image_DXT.o image_helper.o stb_image_aug.o

TARGET = bubble

all: $(TARGET)

$(TARGET): $(CPP_OBJS) $(C_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

SOIL.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c "$(SOIL_DIR)/SOIL.c" -o $@

image_DXT.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c "$(SOIL_DIR)/image_DXT.c" -o $@

image_helper.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c "$(SOIL_DIR)/image_helper.c" -o $@

stb_image_aug.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c "$(SOIL_DIR)/stb_image_aug.c" -o $@

clean:
	rm -f $(CPP_OBJS) $(C_OBJS) $(TARGET)

.PHONY: all clean

