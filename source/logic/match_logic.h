/**
 * @file match_logic.h
 * @author Matthew Getgen
 * @brief Battleships Match Logic
 * @date 2023-08-28
 */

#ifndef MATCH_LOGIC_H
#define MATCH_LOGIC_H

#include <sys/time.h>

#include "game_logic.h"


/// @brief Starts, executes, and manages a match between two players.
/// @param connect connection struct to use throughout match.
/// @param options options to use during match.
/// @param socket_name name of socket to connect over.
/// @return MatchLog with match values stored.
MatchLog run_match(Connection &connect, MatchOptions &options, const char *socket_name);

/// @brief Starts both player executables and connects to them. If one failed, it will kill the other.
/// @param match MatchLog struct to store values into.
/// @param connect Connection struct to get PIDs from.
/// @param options options to get player executables from.
/// @param socket_name name of socket players connect over.
/// @return Status 0 if all is go, -1 if player1 failed, -2 if player 2 failed, -3 if both failed.
int start_players(MatchLog &match, Connection &connect, MatchOptions &options, const char *socket_name);

/// @brief Receives and parses hello messages from players, and then sends a start match message.
/// @param match MatchLog struct to store values into.
/// @param connect connection to receive and send messages over.
/// @param options match options to inform the players about (board size).
/// @return Status 0 if all is go, -1 if player1 failed, -2 if player 2 failed, -3 if both failed.
int handle_start_match(MatchLog &match, Connection &connect, MatchOptions &options);

/// @brief Creates ship amounts, min, and max and exists if an invalid value is given.
/// @param connect connection to close if errors out.
/// @param board_size size of the board to check for.
/// @return ShipInfo struct to store into.
ShipInfo handle_ship_sizes(Connection &connect, int board_size);

/// @brief Merge game player info and stats into match player info.
/// @param match MatchPlayer struct to merge into.
/// @param game GamePlayer struct to get info from.
void merge_game_and_match_player(MatchPlayer &match, GamePlayer &game);

/// @brief Send players game over messages and stop/kill player processes.
/// @param connect Connection struct to send messages over.
/// @param last_status status of the match at that state.
void handle_match_over(Connection &connect, int last_status);

/// @brief Checks for errors in the match and increments the counter. Only used when an error occurs before the any games start.
/// @param match MatchLog struct to check errors against and save result to.
/// @return Status 0 if all is go, -1 if player1 failed, -2 if player 2 failed, -3 if both failed.
int check_match_errors_save_result(MatchLog &match);

/// @brief Checks for errors in the match. Only occurs at the end of the match to calculate if a game has errored.
/// @param match MatchLog struct to check errors against.
/// @return Status 0 if all is go, -1 if player1 failed, -2 if player 2 failed, -3 if both failed.
int check_match_errors(MatchLog &match);

/// @brief Calculates and stores elapsed time at the end of a match.
/// @param match MatchLog struct to store time into.
/// @param start saved time of match.
/// @param end saved end of match.
void store_elapsed_time(MatchLog &match, timeval &start, timeval &end);

#endif

