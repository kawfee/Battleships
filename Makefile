.PHONY: default all run clean_controller player clean_player clean

CC = g++
CFLAGS = -g -Wall -Wextra -Werror -O3

obj_files = source/game_logic.o	\
			source/message.o 	\
			source/server.o 	\
			source/board.o 		\
			source/display.o 	\
			source/conio.o 		\
			source/logger.o 	\
			source/questions.o

default: all

all: controller player

run: controller
	@./controller

controller: $(obj_files)
	@echo "\tbuilding $@"
	@$(CC) $(CFLAGS) -o $@ controller.cpp $(obj_files)

clean_controller:
	@cd source; make clean; cd ..;

$(obj_files):
	@echo "\n### Source ###\n"
	@cd source; make all; cd ..;

player:
	@echo "\n### Players ###\n"
	@cd AI_Files; make all; cd ..;

clean_player:
	@cd AI_Files; make clean; cd ..;

clean: clean_controller clean_player
	rm -f controller battleships.socket logs/match_log.json logs/contest_log.json

