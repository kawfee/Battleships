CXX = g++
CXXFLAGS = -Wall -Wextra
PLAYER_FLAGS = -g $(CXXFLAGS)
PROTECT_DIR = protected/

PLAYER_SRC = $(PROTECT_DIR)Player.cpp
PLAYER_OBJ = $(PROTECT_DIR)Player.o

SRCS = $(wildcard *.cpp)
PROGS = $(patsubst %.cpp, %, $(SRCS))

all: $(PLAYER_OBJ) $(PROGS)

$(PLAYER_OBJ): $(PLAYER_SRC)
	@echo "\tbuilding $@" 
	@$(CXX) $(PLAYER_FLAGS) -o $@ -c $<


%: %.cpp $(PLAYER_OBJ)
	@echo "\tbuilding $@"
	@$(CXX) $(PLAYER_FLAGS) -o $@ $< $(PLAYER_OBJ)

clean:
	rm -f $(PLAYER_OBJ) $(PROGS)

