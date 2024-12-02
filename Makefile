.SUFFIXES:
.SUFFIXES: .cpp .o

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

.PHONY: default
default: all

.PHONY: all
# the default, no player optimization
all: CXXFLAGS = $(DEBUG_CXXFLAGS)
all: run

.PHONY: contest
# a contest, build players with optimization flags
contest: CXXFLAGS = $(RELEASE_CXXFLAGS)
contest: PLAYER_CXXFLAGS = $(RELEASE_CXXFLAGS)
contest: clean run

.PHONY: run
run: build
	@echo "running controller"
	@./controller

.PHONY: build
build: controller player

# controller and source binaries
controller: $(objs)
	@echo "building $@"
	@$(CXX) $(CXXFLAGS) -o $@ controller.cpp $(objs)


$(src_dir)%.o: $(src_dir)%.cpp
	@echo "building $@"
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

# normal players
.PHONY: player
player: CXXFLAGS = $(PLAYER_CXXFLAGS)
player: $(player_obj) $(ai_execs)

$(player_obj): $(player_file)
	@echo "building $@"
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

$(ai_dir)%: $(ai_dir)%.cpp $(player_obj)
	@echo "building $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(player_obj)

.PHONY: options
options: example-options.json
	@echo "creating options.json"
	@cp example-options.json options.json


# cleanup
.PHONY: clean
clean: clean_controller clean_player

.PHONY: clean_controller
clean_controller:
	@echo "removing controller compiled binary and battleships socket file"
	@rm -f controller battleships.socket

	@echo "removing source object files"
	@rm -f $(objs)

	@echo "removing logs"
	@rm -f logs/match_log.json logs/contest_log.json

.PHONY: clean_player
clean_player:
	@echo "removing Player class object file"
	@rm -f $(player_obj)

	@echo "removing player compiled binaries"
	@rm -f $(ai_execs)

.PHONY: clean_options
clean_options:
	@echo "removing options.json file"
	@rm -f options.json

