.SUFFIXES:
.SUFFIXES: .cpp .o
.PHONY: default contest all run build player clean clean_controller clean_player options

# Default Compilation rules
CXX = g++
PLAYER_CXXFLAGS = -g -static -Wall -Wextra
RELEASE_CXXFLAGS = -static -Wall -Wextra -O3
DEBUG_CXXFLAGS = -g $(RELEASE_CXXFLAGS) -Werror

# source directory info
src_dir = source/
server_dir = $(src_dir)server/
logic_dir = $(src_dir)logic/
display_dir = $(src_dir)display/

# source file info
srcs =	$(wildcard $(server_dir)*.cpp) \
		$(wildcard $(logic_dir)*.cpp)  \
		$(wildcard $(display_dir)*.cpp)
objs =	$(patsubst %.cpp, %.o, $(srcs))

# ai directory info
ai_dir = ai_files/
protect_dir = $(ai_dir)protected/

# protected file info
player_file = 	$(protect_dir)Player.cpp
player_obj =	$(protect_dir)Player.o

# ai file info
ai_srcs =	$(wildcard $(ai_dir)*.cpp)
ai_execs =	$(patsubst %.cpp, %, $(ai_srcs))

default: all

# the default, no player optimization
all: CXXFLAGS = $(DEBUG_CXXFLAGS)
all: run

# a contest, build players with optimization flags
contest: CXXFLAGS = $(RELEASE_CXXFLAGS)
contest: PLAYER_CXXFLAGS = $(RELEASE_CXXFLAGS)
contest: clean run

run: build
	@echo "running controller"
	@./controller

build: controller player

# controller and source
controller: $(objs)
	@echo "building $@"
	@$(CXX) $(CXXFLAGS) -o $@ controller.cpp $(objs)


$(src_dir)%.o: $(src_dir)%.cpp
	@echo "building $@"
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

# normal players
player: CXXFLAGS = $(PLAYER_CXXFLAGS)
player: $(player_obj) $(ai_execs)

$(player_obj): $(player_file)
	@echo "building $@"
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

$(ai_dir)%: $(ai_dir)%.cpp $(player_obj)
	@echo "building $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(player_obj)

options: example-options.json
	@echo "creating options.json"
	@cp example-options.json options.json

# cleanup
clean: clean_controller clean_player

clean_controller:
	@echo "removing controller executable and battleships socket"
	@rm -f controller battleships.socket

	@echo "removing logs"
	@rm -f logs/match_log.json logs/contest_log.json

	@echo "removing binary object source files"
	@rm -f $(objs)

clean_player:
	@echo "removing Player.cpp object files"
	@rm -f $(player_obj)

	@echo "removing player executables"
	@rm -f $(ai_execs)

