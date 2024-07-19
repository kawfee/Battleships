/**
 * @file display_contest.h
 * @author Matthew Getgen
 * @brief Display Contest functionality for Battleships
 * @date 2023-09-08
 */

#ifndef DISPLAY_CONTEST_H
#define DISPLAY_CONTEST_H

#include "display_match.h"
#include "../logic/contest_logic.h"


/* ───────────────────────── *
 * DISPLAY CONTEST FUNCTIONS *
 * ───────────────────────── */

/// @brief Handles display options for the contest display.
/// @param contest ContestLog struct to display.
/// @param options ContestOptions struct with options for display.
void display_contest_with_options(ContestLog &contest, ContestOptions &options);

/// @brief Displays the contest in it's totality.
/// @param info Info for how to display.
/// @param contest ContestLog struct to display.
/// @param board Board struct to store info to.
void display_contest(DisplayInfo &info, ContestLog &contest, Board &board);

/// @brief Displays the leaderboard of all players after the contest has simulated through it.
/// @param info Info for how to display.
/// @param contest ContestLog struct to get players from.
void display_contest_leaderboard(DisplayInfo &info, ContestLog &contest);

/// @brief Prints names based on their final status.
/// @param name Name of player to print.
/// @param width Width the player should be displayed as.
/// @param lives Number of lives the player has.
/// @return string formatted by player name.
string print_name_by_final_status(string name, int width, int lives);


/* ─────────────────────── *
 * DISPLAY ROUND FUNCTIONS *
 * ─────────────────────── */

/// @brief Manages state for each round, and then displays them.
/// @param info Info for how to display.
/// @param contest ContestLog struct to get info from.
/// @param board Board struct to store actions to.
void display_contest_rounds(DisplayInfo &info, ContestLog &contest, Board &board);

/// @brief Clears the screen and displays the round number.
/// @param info Info for how to display.
/// @param round_num Round number to display.
void display_round_screen(DisplayInfo &info, int round_num);

/// @brief Displays the leaderboard at the end of the round.
/// @param info Info for how to display.
/// @param round_players ContestPlayer list with players that were in this round.
void display_round_leaderboard(DisplayInfo &info, vector<ContestPlayer> &round_players);

/// @brief Prints the player's name with the color of their status.
/// @param name Name to print.
/// @param width Width to print.
/// @param lives Number of lives the player has at this point.
/// @return string of formatted name.
string print_name_by_status(string name, int width, int lives);

/// @brief Sorts the players by rank in the contest.
/// @param a ContestPlayer that's first.
/// @param b ContestPlayer that's second.
/// @return true if in order, false if not.
bool sort_players_by_rank(const ContestPlayer &a, const ContestPlayer &b);


/* ─────────────────────── *
 * DISPLAY MATCH FUNCTIONS *
 * ─────────────────────── */

/// @brief Displays contest match information.
/// @param info Info for how to display.
/// @param match ContestMatch struct for the game to display.
/// @param player1 ContestPlayer struct of player1.
/// @param player2 ContestPlayer struct of player2.
/// @param board Board struct to store actions to.
/// @param round_num Round number to print.
void display_contest_match(DisplayInfo &info, ContestMatch &match, ContestPlayer &player1, ContestPlayer &player2, Board &board, int round_num);

/// @brief Displays the last game of the ContestMatch, and the result.
/// @param info Info for how to display.
/// @param match ContestMatch struct for the game to display.
/// @param player1 ContestPlayer struct of player1.
/// @param player2 ContestPlayer struct of player2.
/// @param board Board struct to store actions to.
void display_contest_match_game(DisplayInfo &info, ContestMatch &match, ContestPlayer &player1, ContestPlayer &player2, Board &board);

/// @brief Displays the contest match VS and result.
/// @param info Info for how to display.
/// @param match ContestMatch struct for what to display.
/// @param player1 ContestPlayer struct for player1.
/// @param player2 ContestPlayer struct for player2.
void display_contest_match_result(DisplayInfo &info, ContestMatch &match, ContestPlayer &player1, ContestPlayer &player2);

/// @brief Displays the VS for a match for a contest match.
/// @param info Info for how to display.
/// @param name1 Name of player1.
/// @param name2 Name of player2.
void display_contest_match_vs(DisplayInfo &info, string name1, string name2);

/// @brief Prints the name of the player's name that won, or a tie if a tie. 
/// @param player1_result Result of the game for player 1. All you need to determine the result.
/// @param p1_name Name of player1.
/// @param p2_name Name of player2.
string print_contest_match_by_result(GameResult player1_result, string p1_name, string p2_name);

#endif

