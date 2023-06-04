CC = g++
CFLAGS = -g -Wall -Wextra -Werror -O3

cpp_files = game_logic.cpp \
			message.cpp \
			server.cpp \
			board.cpp \
			display.cpp \
			conio.cpp \
			logger.cpp \
			questions.cpp

header_files = $(patsubst %.cpp, %.h, $(cpp_files))
obj_files = $(patsubst %.cpp, %.o, $(cpp_files))

all: $(header_files) $(cpp_files) $(obj_files)

%.o: %.cpp %.h
	@echo "\tbuilding $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(obj_files)

