/**
 * @file display_match.h
 * @author Matthew Getgen
 * @brief Display Match functionality for Battleships
 * @date 2023-09-08
 */

#ifndef DISPLAY_MATCH_H
#define DISPLAY_MATCH_H

#include "display_game.h"


/// @brief Sets up all the data structures for the match display.
/// @param match MatchLog struct to display.
/// @param options MatchOptions with options contained.
/// @param row Row that questions are currently at. Used by any future questions to ask about the display.
void display_match_with_options(MatchLog &match, MatchOptions &options, int row);

/// @brief Displays the match based on the display type from the user.
/// @param info Info for how to display.
/// @param match MatchLog struct to display.
/// @param board Board struct to store actions to.
void display_match(DisplayInfo &info, MatchLog &match, Board &board);

/// @brief Decides whether to step through, or display game list. Dependant on what user chose.
/// @param info Info for how to display.
/// @param match MatchLog struct to get games from.
/// @param board Board struct to store actions to.
/// @param game_list List of games to display or step through. The list is filled by the display option chosen by the user.
void handle_game_list(DisplayInfo &info, MatchLog &match, Board &board, vector<int> &game_list);

/// @brief Displays all games in the game list.
/// @param info Info for how to display.
/// @param match MatchLog struct to get games from.
/// @param board Board struct to store actions to.
/// @param game_list List of games to display. The list is filled by the display option chosen by the user.
void display_game_list(DisplayInfo &info, MatchLog &match, Board &board, vector<int> &game_list);

/// @brief Steps through all games in the game list.
/// @param info Info for how to display step through values.
/// @param match MatchLog struct to get games from.
/// @param board Board struct to store actions to.
/// @param game_list List of games to step through. The list is filled by the display option chosen by the user.
void step_through_game_list(DisplayInfo &info, MatchLog &match, Board &board, vector<int> &game_list);

/// @brief Displays the match player VS player at the top of the screen.
/// @param info Info for how to display.
void display_match_vs(DisplayInfo &info);

/// @brief Displays the match result (winner, or a tie).
/// @param info Info for how to display.
/// @param match MatchLog struct to get result info from.
void display_match_result(DisplayInfo &info, MatchLog &match);

/// @brief Prints the author name if present, and AI name in bold.
/// @param name Name struct to print.
/// @return string of printed name.
string print_author_and_ai(Name &name);

/// @brief Displays both errors depending on if they happened or not.
/// @param info Info for how to display.
/// @param match MatchLog struct to get errors from.
void display_match_errors(DisplayInfo &info, MatchLog &match);

/// @brief Displays a comprehensive error for the user to understand what went wrong.
/// @param info Info for how to display.
/// @param error Error struct storing error data.
/// @param player Number for which player it is.
void display_match_error(DisplayInfo &info, Error &error, PlayerNum player);

/// @brief Displays the stats of both players for the match as a table.
/// @param info Info for how to display.
/// @param match MatchLog struct to access the stats.
void display_match_stats(DisplayInfo &info, MatchLog &match);

/// @brief Displays the elapsed time for the match.
/// @param info Info for how to display.
/// @param elapsed_time Time elapsed for the match.
void display_elapsed_time(DisplayInfo &info, float elapsed_time);

/// @brief Clears the screen and resets the display row.
/// @param info Info for how to display.
void reset_screen(DisplayInfo &info);

/// @brief Shortens the name of the player for display reasons (if too long).
/// @param name Name to shorten.
/// @return String value of shortened name.
string clean_name(string name);

/// @brief Calculates the average percent board hit, based on the total number of shots, 
/// number of games, and size of the board.
/// @param total_num_board_shot Total number of shots made by the player for a match. 
/// @param board_size Size of the board.
/// @param num_games Number of games played.
/// @return Percentage returned out of 100.
int calculate_avg_percent_board_hit(int total_num_board_shot, int board_size, int num_games);

#endif

