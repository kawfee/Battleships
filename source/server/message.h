/**
 * @file message.h
 * @author Matthew Getgen
 * @brief Battleships Message Definitions
 * @date 2023-08-27
 *
 * JSON C++ Library from [nlohmann](https://github.com/nlohmann/json)
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include "../defines.h"
#include "../json.hpp"

using json = nlohmann::json;

/// @brief Message Types that are sent and received. Numbered in order of occurrence.
enum MessageType {
    // SERVER MESSAGE TYPES -- used by the server to create messages, used by the client to parse messages
    SetupMatch = 2,
    StartGame = 3,
    PlaceShip = 4,
    TakeShot = 6,
    ShotReturn = 8,
    GameOver = 9,
    MatchOver = 10,
    // CLIENT MESSAGE TYPES -- used by client to create messages, used by server to parse messages
    Hello = 1,
    ShipPlaced = 5,
    ShotTaken = 7,
};


/* ──────────────────────── *
 * CREATE MESSAGE FUNCTIONS *
 * ──────────────────────── */

/// @brief A helper function that converts a JSON object into a c string.
/// @param msg message buffer.
/// @param j JSON object.
void append_json_to_msg(char *msg, json &j);

/// @brief Creates a setup_match message.
/// @param msg message buffer.
/// @param board_size size of the board.
/// @param num player number (1 or 2).
void create_setup_match_msg(char *msg, int board_size, PlayerNum num);

/// @brief Creates a start_game message.
/// @param msg message buffer.
void create_start_game_msg(char *msg);

/// @brief Creates a place_ship message.
/// @param msg message buffer.
/// @param length length of the ship.
void create_place_ship_msg(char *msg, int length);

/// @brief Creates a take_shot message.
/// @param msg message buffer.
void create_take_shot_msg(char *msg);

/// @brief Creates a shot_return message.
/// @param msg message buffer.
/// @param shot1 shot returned by player 1.
/// @param shot2 shot returned by player 2.
/// @param game Game log to get shots that are dead.
/// @param next_shot whether the player should take a shot next or not.
void create_shot_return_msg(char *msg, Shot &shot1, Shot &shot2, GameLog &game, bool next_shot);

/// @brief Creates a game_over message.
/// @param msg message buffer.
/// @param stats stats from the match.
void create_game_over_msg(char *msg, GameStats &stats);

/// @brief Creates a match_over message.
/// @param msg message buffer.
void create_match_over_msg(char *msg);


/* ─────────────────────── *
 * PARSE MESSAGE FUNCTIONS *
 * ─────────────────────── */

/// @brief Validates and parses a hello message.
/// @param msg message buffer.
/// @param ai_name player AI name.
/// @param author_names author name buffer.
/// @return OK on valid, BAD_HELLO_MSG on invalid.
Error parse_hello_msg(char *msg, string &ai_name, string &author_names);

/// @brief Validates a hello message.
/// @param msg message buffer.
/// @param j JSON struct to check against.
/// @return true if valid, false if invalid.
bool validate_hello_msg(char *msg, json &j);

/// @brief Validates and parses a ship_placed message.
/// @param msg message buffer to read from.
/// @param ship struct to store ship data.
/// @return OK on valid, BAD_SHIP_PLACED_MSG on invalid.
Error parse_ship_placed_msg(char *msg, Ship &ship);

/// @brief Validates a ship_placed message.
/// @param msg message buffer to read from.
/// @param j JSON struct to check against.
/// @return true if valid, false if invalid.
bool validate_ship_placed_msg(char *msg, json &j);

/// @brief Validates and parses a shot_taken message.
/// @param msg message buffer.
/// @param shot struct to store shot data.
/// @return OK on valid, BAD_SHOT_TAKEN_MSG on invalid.
Error parse_shot_taken_msg(char *msg, Shot &shot);

/// @brief Validates a shot_taken message.
/// @param msg message buffer to read from.
/// @param j JSON struct to check against.
/// @return 
bool validate_shot_taken_msg(char *msg, json &j);

#endif

