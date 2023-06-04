/**
 * @file messages.h
 * @author Matthew Getgen
 * @brief Battleships Message Definitions
 * @date 2022-06-14
 *
 * JSON C++ Library from [nlohmann](https://github.com/nlohmann/json)
 * 
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include "defines.h"

/**
 * @brief Message Types that are sent and received. Numbered in order of their occurence.
 */
enum MessageType {
    // SERVER MESSAGE TYPES -- used by the server to create messages, used by the client to parse messages
    setup_match = 2,
    start_game = 3,
    place_ship = 4,
    take_shot = 6,
    shot_return = 8,
    ship_dead = 9,
    game_over = 10,
    match_over = 11,
    // CLIENT MESSAGE TYPES -- used by client to create messages, used by server to parse messages
    hello = 1,
    ship_placed = 5,
    shot_taken = 7
};


/* ──────────────────────── *
 * CREATE MESSAGE FUNCTIONS *
 * ──────────────────────── */

/**
 * @brief A helper function that converts a JSON object into a c string.
 * @param msg message buffer.
 * @param j JSON object.
 */
void append_json_to_msg(char *msg, json j);

/**
 * @brief Creates a setup_match message.
 * @param msg message buffer.
 * @param board_size board size to append to message.
 * @param num player number (1 or 2).
 */
void create_setup_match_msg(char *msg, int board_size, PlayerNum num);

/**
 * @brief Creates a start_game message.
 * @param msg message buffer.
 */
void create_start_game_msg(char *msg);

/**
 * @brief Creates a place_ship message.
 * @param msg message buffer.
 * @param length length of ship.
 */
void create_place_ship_msg(char *msg, int length);

/**
 * @brief Creates a take_shot message.
 * @param msg message buffer.
 */
void create_take_shot_msg(char *msg);

/**
 * @brief Creates a shot_return message.
 * @param msg message buffer.
 * @param num player number (1 or 2).
 * @param shot shot_info structure to send.
 */
void create_shot_return_msg(char *msg, PlayerNum num, Shot shot);

/**
 * @brief Creates a ship_dead message.
 * @param msg message buffer.
 * @param num player number (1 or 2).
 * @param ship ship_info structure to send.
 */
void create_ship_dead_msg(char *msg, PlayerNum num, Ship ship);

/**
 * @brief Creates a game_over message.
 * @param msg message buffer.
 */
void create_game_over_msg(char *msg);

/**
 * @brief Creates a match_over message.
 * @param msg message buffer.
 */
void create_match_over_msg(char *msg);


/* ─────────────────────── *
 * PARSE MESSAGE FUNCTIONS *
 * ─────────────────────── */

/**
 * @brief Validates and parses a hello message.
 * @param msg message buffer.
 * @param ai_name player AI buffer.
 * @param author_names authors buffer.
 * @return NO_ERR on valid, BAD_HELLO_MSG on invalid.
 */
ErrorNum parse_hello_msg(char *msg, char *ai_name, char *author_names);

/**
 * @brief Validates and parses a ship_placed message.
 * @param msg message buffer.
 * @param ship ship_info structure to store ship data.
 * @return NO_ERR on valid, BAD_SHIP_PLACED_MSG on invalid.
 */
ErrorNum parse_ship_placed_msg(char *msg, Ship &ship);

/**
 * @brief Validates and parses a shot_taken message.
 * @param msg message buffer.
 * @param shot shot_info structure to store shot data.
 * @return NO_ERR on valid, BAD_SHOT_TAKEN_MSG on invalid.
 */
ErrorNum parse_shot_taken_msg(char *msg, Shot &shot);

#endif

