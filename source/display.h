/**
 * @file display.h
 * @author Matthew Getgen
 * @brief Battleships Display header file
 * @date 2022-04-25
 */

#ifndef DISPLAY_H
#define DISPLAY_H


#include <fstream>
#include <unistd.h>

#include "defines.h"
#include "json.hpp"
#include "questions.h"
#include "board.h"
#include "logger.h"
#include "buffer_toggle.h"

#define LEFT_COL_OFFSET 2   // for displaying the column offset for both boards
#define BOARD_1_OFFSET  1   // for displaying the column board values onto board 1
#define BOARD_2_OFFSET  50  // for displaying the column board values onto board 2
#define TOP_ROW_OFFSET  11  // for displaying the row board values onto the boards
#define MAX_NAME_LEN    20  // for the max width to display Authors and AI names.
#define SLEEP_TIME      3   // seconds to wait for sleep


struct Name {
    string p1;
    string p2;
    string a1;
    string a2;
};

/* ───────────────────────── *
 * DISPLAY CONTEST FUNCTIONS *
 * ───────────────────────── */

/**
 * @brief Loads in the contest_log.json file and validates it before displaying.
 * @param delay delay time between each action.
 * @param log_dir directory to check for logs.
 * @param leaderboard_only Display only the leaderboard of the game.
 */
void replay_contest(int delay, string log_dir, bool leaderboard_only);

/**
 * @brief Displays the contest JSON.
 * @param  contest_log contest JSON to display.
 * @param delay delay time between each action.
 */
void display_contest(json &contest_log, int delay);

/**
 * @brief Converts JSON list of players and converts it into a structured list.
 * @param contest_players JSON object to check.
 * @param get_end_value Stores the value that's stored in the log, rather than setting it to 0.
 * @return vector of ContestPlayer struct values.
 */
vector<ContestPlayer> get_contest_players(json &contest_players, bool get_end_value);

/**
 * @brief Gets the AI in the vector associated with the AI in the JSON.
 * @param players set of all players in the contest.
 * @param idx index of player in JSON.
 * @return index of player in the set.
 */
int get_ai_num_by_idx(vector<ContestPlayer> &players, int idx);

/**
 * @brief For each contest match, combine match W/L/T with contest W/L/T.
 * @param player contest player reference.
 * @param contest_player json value of contest player per match.
 */
void store_match_player_values(ContestPlayer &player, json &contest_player);

/**
 * @brief Displays the Leaderboard for a contest.
 * @param players set of all players in the contest.
 * @param show_all If true, displays all players. Otherwise, display the Top 10.
 */
void display_leaderboard(vector<ContestPlayer> &players, bool show_all);

/**
 * @brief Sorts the players in the vector based on who has the most wins.
 * If wins are equal, then its who has the least losses.
 * @param a first player to compare.
 * @param b second player to compare.
 * @return true if a has more wins than b, else false.
 */
bool sort_players_by_winner(const ContestPlayer &a, const ContestPlayer &b);


/* ─────────────────────── *
 * DISPLAY MATCH FUNCTIONS *
 * ─────────────────────── */

/**
 * @brief Loads in the match_log.json file and validates it before displaying.
 * @param display display type for the games.
 * @param delay delay time between each action.
 * @param log_dir directory to check for logs.
 * @param step_through whether to step through the game or not.
 */
void replay_match(DisplayType display, int delay, string log_dir, bool step_through);

/**
 * @brief Displays the match JSON.
 * Action differs by display type.
 * @param match_log match JSON to display.
 * @param display display type for the games.
 * @param delay delay time between each action.
 * @param step_through whether to step through the game or not.
 */
void display_match(json &match_log, DisplayType display, int delay, bool step_through);

/**
 * @brief Displays the match JSON per game chosen to display.
 * @param match_log match JSON to display.
 * @param boards boards to use for the games.
 * @param game_indexes indexes of the games to display
 * @param delay delay time between each action.
 * @param names struct of names for the AIs.
 */
void display_each_game(json &match_log, Board &boards, vector<int> &game_indexes, int delay, Name &names);

/**
 * @brief Creates a step-through match log per game chosen to display, then manages the display of it.
 * @param match_log match JSON to display.
 * @param boards boards to use for the games.
 * @param game_indexes indexes of the games to display
 * @param names struct of names for the AIs.
 */
void step_through_each_game(json &match_log, Board &boards, vector<int> &game_indexes, Name &names);

/**
 * @brief Displays the errors stored in the match log.
 * @param match_log match JSON to access the error values.
 * @param board_size size of the board.
 */
void display_match_errors(json &match_log, int board_size);

/**
 * @brief Displays any error message with corresponding error enum.
 * @param error error enum value to display.
 * @param row_offset offset for the specific board size.
 * @param col_offset offset for the specific player.
 */
void display_error_by_code(ErrorNum error, int row_offset, int col_offset);

/**
 * @brief Displays the results of the match at the top of the screen.
 * @param match_log match JSON log to display.
 * @param names struct of names for the AIs.
 */
void only_display_match_results(json &match_log, Name &names);

/**
 * @brief Displays the results of the match at the bottom in a table.
 * @param match_log match JSON log to display.
 * @param board_size size of the board to display.
 * @param names struct of names for the AIs.
 */
void display_match_results(json &match_log, int board_size, Name &names);

/**
 * @brief Displays the results of the match for one player.
 * @param match_player player JSON with values to display.
 * @param p player name to display.
 * @param display_offset max size of the player names for displaying the spacing offset.
 */
void display_match_results_by_player(json &match_player, string &p, int display_offset);


/* ────────────────────── *
 * DISPLAY GAME FUNCTIONS *
 * ────────────────────── */

/**
 * @brief Handles and displays the logged game.
 * @param game_log game JSON log.
 * @param boards boards to work with and display.
 * @param delay time delay between shots.
 * @param game_num game number. If -1, it is a contest game (always last game).
 * @param names struct of names for the AIs.
 */
void display_game(json &game_log, Board &boards, int delay, int game_num, Name &names);


void step_through_game2(json &step_through_log, Board &boards, vector<int> game_indexes, Name &names);

/**
 * @brief Handles the user input for stepping through a game.
 * @param game_log game JSON log.
 * @param boards boards to work with and display.
 * @param game_num game number. If -1, it is a contest game (always last game).
 * @param names struct of names for the AIs.
 */
void step_through_game(json &game_log, Board &boards, int game_num, Name &names);

/**
 * @brief Processes each step and stores the values into the boards.
 * @param step_log JSON log created with the entire game built to step through.
 * @param boards boards to store processed values into.
 * @param step number of the step to access values from.
 */
void process_step(json &step_log, Board &boards, int step);

/**
 * @brief Handles logic for killed ships. Stores the ship's value to the board.
 * @param shot shot made, used to store the value as a kill if so.
 * @param shot_log shot made, but has info on whether the shot was a kill or not.
 * @param ships JSON list of ships.
 * @param player number for the player (1 or 2).
 * @param boards boards to store the state of the killed ship.
 * @param display whether to display the killed ship then or not.
 */
void handle_killed_ship(Shot &shot, json &shot_log, json &ships, PlayerNum player, Board &boards, bool display);

/**
 * @brief Displays the Game number to the terminal.
 * @param game_num Number to display. If -1, display "Last Game" instead;
 */
void display_game_number(int game_num);

/**
 * @brief Displays the results for the game (W/L/T).
 * @param game_log Game JSON storing the results for the game.
 * @param board_size size of the board to display correctly.
 * @param p1 player1 ai name.
 * @param p2 player2 ai name.
 */
void display_game_result(json &game_log, int board_size, Name &names);

/**
 * @brief Displays each game result per player.
 * @param result Result of the game for that player.
 * @param p player name to display.
 * @param row_offset offset for the specific board size.
 * @param col_offset offset for the specific player.
 */
void display_game_result_by_player(GameResult result, string &p, int row_offset, int col_offset);


/* ─────────────────────── *
 * DISPLAY BOARD FUNCTIONS *
 * ─────────────────────── */

/**
 * @brief Display the the two boards initially.
 * @param board_size size of the board.
 * @param names struct of names for the AIs.
 * @param is_final if true, displays "Final Status of " with the ai name.
 */
void display_init_boards(int board_size, Name &names, bool is_final);

/**
 * @brief Displays the values stored on the board.
 * Including ship segments not hit.
 * @param boards boards to display.
 */
void display_entire_board(Board &boards);

/**
 * @brief Displays any character with the correct background and foreground color chosen at a given location.
 * @param player number for the player (1 or 2).
 * @param shot shot structure to display.
 * @param is_current if true, display current hit with black background and color foreground.
 */
void display_board_value(PlayerNum player, Shot shot, bool is_current);

/**
 * @brief Displays the shot value in words and colors, as well as the shot row and col.
 * @param board_size size of the board for displaying.
 * @param player number for the player (1 or 2).
 * @param shot shot structure to display value and row col of.
 */
void display_shot_value(int board_size, PlayerNum player, Shot &shot);

/**
 * @brief Displays a ship based on it's value.
 * @param player number for the player (1 or 2).
 * @param ship ship structure to diplay.
 * @param value value of the ship (either SHIP or KILL).
 */
void display_ship(PlayerNum player, Ship &ship, BoardValue value);

/**
 * @brief Displays the ship placement in words if it is a step-through.
 * @param board_size size of the board for displaying.
 * @param player number for the player (1 or 2).
 * @param ship ship struct to display values from.
 */
void display_ship_placement(int board_size, PlayerNum player, Ship &ship);

/**
 * @brief Cleans the name length to display it within 20 characters
 * Summarizes the rest with "..."
 * @param s string to shorten if necessary
 */
void clean_name(string &s);

/**
 * @brief Set cursor to standard color and background, 
 * as well as location to far down on the screen.
 */
void reset_cursor();

#endif

