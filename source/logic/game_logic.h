/**
 * @file game_logic.h
 * @author Matthew Getgen
 * @brief Battleships Game Logic
 * @date 2023-08-28
 */

#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "board.h"
#include "../server/message.h"
#include "../server/server.h"


/// @brief Struct for size range of ships.
struct ShipInfo {
    int num_ships;
    int max_len;
    int min_len;
};


/// @brief Runs and manages a game. Creates a Game Log.
/// @param connect Struct that stores connection data.
/// @param board Struct that stores board data.
/// @param ship_info Struct storing ship info like max/min len, etc.
/// @return Struct storing the Game Log.
GameLog run_game(Connection &connect, Board &board, ShipInfo &ship_info);

/// @brief Creates a ship length, and creates a number of ships based on 
/// @param game Struct that stores game data.
/// @param connect Struct that stores connection data.
/// @param board Struct that stores board data.
/// @param ship_info Struct storing ship info like max/min len, etc.
/// @return Status 0 if all is good, -1 if player1 failed, -2 if player 2 failed, -3 if both failed.
int handle_ships(GameLog &game, Connection &connect, Board &board, ShipInfo &ship_info);

/// @brief Receives, parses, and validates ship placed by player.
/// @param game Struct that stores game data.
/// @param connect Struct that stores connection data.
/// @param board Struct that stores board data.
/// @param length Length of the expected ship size.
/// @return Status 0 if all is good, -1 if player1 failed, -2 if player 2 failed, -3 if both failed.
int handle_ship_placement(GameLog &game, Connection &connect, Board &board, int length);

/// @brief Validates that a ship is on a board, not overlapping others, etc.
/// @param board Board to check for a ship.
/// @param board_size Size of the board.
/// @param ship Ship that was placed by player.
/// @param length Length of the expected ship length.
/// @return OK if valid, BAD_SHIP with ship struct if invalid.
Error validate_ship_placement(char **board, int board_size, Ship &ship, int length);

/// @brief Calculates the shot value, checks for dead ships, and sends shot_return messages.
/// @param game Struct that stores game data.
/// @param connect Struct that stores connection data.
/// @param board Struct that stores board data.
/// @param next_shot Check for whether to take a shot next or not.
/// @return Status 0 if all is good, -1 if player1 failed, -2 if player 2 failed, -3 if both failed.
int handle_shots(GameLog &game, Connection &connect, Board &board, bool &next_shot);

/// @brief Receives, parses, and validates shots made by players.
/// @param game Struct that stores game data.
/// @param connect Struct that stores connection data.
/// @param board Struct that stores board data.
/// @param shot1 Struct to store player 1's shot.
/// @param shot2 Struct to store player 2's shot.
/// @return Status 0 if all is good, -1 if player1 failed, -2 if player 2 failed, -3 if both failed.
int handle_shot_placement(GameLog &game, Connection &connect, Board &board, Shot &shot1, Shot &shot2);

/// @brief Validates that a shot is on the board.
/// @param size Size of the board.
/// @param shot Struct that was taken by player.
/// @return OK if valid, BAD_SHOT if invalid.
Error validate_shot_placement(int size, Shot &shot);

/// @brief Calculates the value of a shot, based on the current value on the board.
/// @param stats Struct that stores player stats.
/// @param shot Shot that was taken on an enemy board.
/// @param opponent_board Board that was shot at.
void get_shot_value(GameStats &stats, Shot &shot, char **opponent_board);

/// @brief Finds a dead ship on a board.
/// @param player Struct that stores player data.
/// @param board board to check for a dead ship.
/// @return Index of a dead ship, -1 if none.
int find_dead_ship(GamePlayer &player, char **board);

/// @brief Checks players ships and counts the number of alive ships.
/// @param player Struct that stores player data.
/// @return Number of alive ships for that player.
int check_alive_ship(GamePlayer &player);

/// @brief Checks the alive ships to calculate the winner.
/// @param game Struct that stores game data.
void calculate_winner(GameLog &game);

/// @brief Creates and sends game_over message to each player.
/// @param game Struct that stores game data.
/// @param connect Struct that stores connection data.
/// @return Status 0 if all is good, -1 if player1 failed, -2 if player 2 failed, -3 if both failed.
int handle_game_over(GameLog &game, Connection &connect);

/// @brief Checks for errors from the game.
/// @param game Struct that stores game data.
/// @return Status 0 if all is good, -1 if player1 failed, -2 if player 2 failed, -3 if both failed.
int check_game_errors(GameLog &game);

#endif

