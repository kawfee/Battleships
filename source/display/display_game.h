/**
 * @file display_game.h
 * @author Matthew Getgen
 * @brief Battleships Display Game header filed
 * @date 2023-09-08
 */

#ifndef DISPLAY_GAME_H
#define DISPLAY_GAME_H

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <ostream>
#include <unistd.h>
 
#include "conio.h"
#include "buffer_toggle.h"
#include "options.h"

#include "../logic/board.h"
 
#define LEFT_COL_OFFSET 2  // to display the column offset for both boards
#define BOARD_1_OFFSET  1  // to display the column board values for board 1
#define BOARD_2_OFFSET  50 // to display the column board values for board 2
#define MAX_NAME_LEN    20 // to display the max width to display the names
#define SLEEP_TIME      3  // seconds to wait for sleep

/// @brief Name Struct to store names for a player.
struct Name {
    string ai_name;
    string author_name;
};

/// @brief DisplayInfo to help display anything.
struct DisplayInfo {
    Name player1;
    Name player2;
    int display_row;
    int board_row;
    int delay_time;
    bool step_through;
    MatchDisplayType type;
    ContestDisplayType contest_type;
};

/// @brief State of a game while stepping through it.
struct StepThroughState {
    bool no_ships;
    bool some_ships;
    bool full_ships;
    bool no_shots;
    bool some_shots;
    bool full_shots;
};

/// @brief StepThrough info storing logic and state.
struct StepThroughInfo {
    int board_row;
    int question_row;
    int max_games;
    int max_ships;
    int max_shots;
    int game_step;
    int ship_step;
    int shot_step;
    bool quit;
    bool is_toggled;
    BufferToggle toggle;
    StepThroughState state;
};

/// @brief Keys to press when stepping through a game.
enum StepThroughKey {
    ENTER = 10,
    UP    = 65,
    DOWN  = 66,
    RIGHT = 67,
    LEFT  = 68,
    W     = 119,
    A     = 979,
    S     = 115,
    D     = 100,
    H     = 104,
    J     = 106,
    K     = 107,
    L     = 108,
};

/// @brief Constant string values that can be used to build a table.
const string
    vertical = "│",
    horizontal = "─",
    intersection = "┼",
    end_horizontal = "┤";

/// @brief Displays a game.
/// @param info Display info for how to display.
/// @param game GameLog struct to display.
/// @param board Board struct to store to and display.
void display_game(DisplayInfo &info, GameLog &game, Board &board);

/// @brief Handles step through logic for a game.
/// @param info Display info for how to display.
/// @param step_info Info about step logic.
/// @param game GameLog struct to step through.
/// @param board Board struct to store with and display.
void step_through_game(DisplayInfo &info, StepThroughInfo &step_info, GameLog &game, Board &board);

/// @brief Process the key press and what happends as a result.
/// @param step_info Info about the step logic.
/// @param key Key pressed by user.
/// @return true if key press changed the game state, false if not.
bool process_key(StepThroughInfo &step_info, StepThroughKey key);

/// @brief Stores the step through state into a board, and displays data.
/// @param info Display info for how to display.
/// @param step_info Info about the step logic.
/// @param game GameLog struct to display.
/// @param board Board struct to store with and display.
void store_step_through_state(DisplayInfo &info, StepThroughInfo &step_info, GameLog &game, Board &board);

/// @brief Calculates the state of the step through after an arrow press.
/// @param step_info Info about the step logic.
void calculate_step_through_state(StepThroughInfo &step_info);

/// @brief Displays the game number of the match.
/// @param info Display info for how to display.
/// @param game_num Number to display.
void display_game_number(DisplayInfo &info, int game_num);

/// @brief Displays the names of players above their boards.
/// @param info Display info with names to display.
void display_game_board_names(DisplayInfo &info);

/// @brief Displays a board with only water displayed.
/// @param info Display info for how to display.
/// @param board_size Size of the board to display.
void display_empty_boards(DisplayInfo &info, int board_size);

/// @brief Displays all board values currently in the board.
/// @param info Display info for how to display.
/// @param board Board struct to display.
void display_end_game_boards(DisplayInfo &info, Board &board);

/// @brief Displays a ship onto a board.
/// @param info Display info for how to display.
/// @param ship Ship struct to display on board.
/// @param value Value of the ship (SHIP or KILL).
/// @param col_offset Offset of the board to display.
void display_ship(DisplayInfo &info, Ship &ship, BoardValue value, int col_offset);

/// @brief Displays values for both player's ships.
/// @param info Display info for how to display.
/// @param ship1 Ship struct for player1.
/// @param ship2 Ship struct for player2.
void display_ship_infos(DisplayInfo &info, Ship &ship1, Ship &ship2);

/// @brief Displays values from a ship to read easily.
/// @param info Display info for how to display.
/// @param ship Ship struct to display from.
/// @param col_offset Offset of the board to display at.
void display_ship_info(DisplayInfo &info, Ship &ship, int col_offset);

/// @brief Displays both player's shots.
/// @param info Display info for how to display.
/// @param shot1 Shot struct for player1.
/// @param shot2 Shot struct for player2.
/// @param highlight Whether to highlight the shots or not.
void display_shots(DisplayInfo &info, Shot &shot1, Shot &shot2, bool highlight);

/// @brief Displays a shot onto a board.
/// @param info Display info for how to display.
/// @param shot Shot struct to display on board.
/// @param col_offset Offset of board to display.
/// @param highlight Whether to highlight the shot or not.
void display_shot(DisplayInfo &info, Shot &shot, int col_offset, bool highlight);

/// @brief Displays values for both player's shots.
/// @param info Display info for how to display.
/// @param shot1 Shot struct for player1.
/// @param shot2 Shot struct for player2.
void display_shot_infos(DisplayInfo &info, Shot &shot1, Shot &shot2);

/// @brief Displays values from a shot to read easily.
/// @param info Display info for how to display.
/// @param shot Shot struct to display from.
/// @param col_offset Offset of the board to display at.
void display_shot_info(DisplayInfo &info, Shot &shot, int col_offset);

/// @brief Displays the game results and any errors for players.
/// @param info Display info for how to display.
/// @param game GameLog struct to use to display info.
void display_game_results_and_errors(DisplayInfo &info, GameLog &game);

/// @brief Display the game result of a player.
/// @param info Display info for how to display.
/// @param result GameResult to display.
/// @param player player number for player that is being displayed.
void display_game_result(DisplayInfo &info, GameResult result, PlayerNum player);

/// @brief Displays an error message from an error that occcurs during a game.
/// @param info Display info for how to display.
/// @param error ErrorType to display a message for.
/// @param col_offset Offset of the board the player's board is on.
void display_game_error(DisplayInfo &info, ErrorType &error, int col_offset);

/// @brief Displays the stats for both players in a game.
/// @param info Display info for how to display.
/// @param game GameLog struct to display stats from.
/// @param board_size Size of the board.
void display_game_stats(DisplayInfo &info, GameLog &game, int board_size);

/// @brief Prints the color of the name based on the game result.
/// @param name Player name to print correctly.
/// @param result Result for the player.
/// @return String of the player's name with the color modified.
string print_name_by_result(string name, GameResult result);

/// @brief Multiplies one string a number of times.
/// @param s string to multiply.
/// @param num num to multiply string by.
/// @return string that has been multiplied.
string multiply_string(string s, int num);

/// @brief Moves the cursor position, and flushes the display.
/// @param display_row Row to display the cursor at.
void reset_cursor(int display_row);

/// @brief Calculates the percentage of the board hit, based on number of board that was shot at, and the size of the board.
/// @param num_board_shot Total hits and misses in that game.
/// @param board_size Size of the board.
/// @return Integer value of the percentage of the board hit.
int calculate_percent_board_shot(int num_board_shot, int board_size);

#endif

