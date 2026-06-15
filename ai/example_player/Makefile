COMMON_FLAGS = -Wall -Wextra -pedantic
DEBUG_FLAGS = $(COMMON_FLAGS) -O0 -g
RELEASE_FLAGS = $(COMMON_FLAGS) -O3 -march=native

PLAYER_CLASS_OBJ = ../Player.o
AI_CPP_FILE = example_player.cpp
AI_EXECUTABLE_NAME = $(patsubst %.cpp, %, $(AI_CPP_FILE))

default: debug

debug:
	g++ $(DEBUG_FLAGS) -o $(AI_EXECUTABLE_NAME) $(AI_CPP_FILE) $(PLAYER_CLASS_OBJ)

release:
	g++ $(RELEASE_FLAGS) -o $(AI_EXECUTABLE_NAME) $(AI_CPP_FILE) $(PLAYER_CLASS_OBJ)
