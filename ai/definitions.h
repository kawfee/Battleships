/**
 * @file definitions.h
 * @author Matthew Getgen
 * @brief Standard Definitions for all Player AIs.
 * @date 2026-05-27
 */

#ifndef DEFINES_H
#define DEFINES_H

#include <stdio.h>

#define PRINT_ERROR(message) \
    do { \
        fprintf(stderr, "[%s:%d] AI ERROR: %s\n", __FILE__, __LINE__, message); \
        fflush(stderr); \
    } while (0)

#define PRINT_ERROR_F(fmt, ...) \
    do { \
        fprintf(stderr, "[%s:%d] AI ERROR: " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); \
        fflush(stderr); \
    } while (0)

#define MAX_MESSAGE_SIZE 256
#define MAX_NAME_SIZE 100

// JSON MESSAGE KEYS -- used by the player and server to create and parse messages
#define MESSAGE_TYPE_KEY "mt"
#define AI_NAME_KEY      "ai"
#define AUTHOR_NAMES_KEY "au"
#define PLAYER_NUM_KEY   "pn"
#define BOARD_SIZE_KEY   "bs"
#define LENGTH_KEY       "l"
#define ROW_KEY          "r"
#define COLUMN_KEY       "c"
#define DIRECTION_KEY    "d"
#define SHIP_KEY         "sp"
#define SHOT_KEY         "st"
#define NEXT_SHOT_KEY    "ns"

/// @brief Message Types that are sent and received. Ordered by occurrence in protocol.
enum MessageType {
    MESSAGE_HELLO,
    MESSAGE_SETUP_MATCH,
    MESSAGE_PLACE_SHIPS,
    MESSAGE_SHIPS_PLACED,
    MESSAGE_SHOT_TAKEN,
    MESSAGE_SHOT_RESULT,
    MESSAGE_MATCH_OVER,
};

/// @brief Number values for different players.
enum PlayerNum {
    /// @brief Whether you are PLAYER 1 or PLAYER 2.
    PLAYER_1 = 1,
    PLAYER_2 = 2,
};

/// @brief possible directions of a ship.
enum Direction {
    /// @brief Whether the ship is HORIZONTAL or VERTICAL.
    HORIZONTAL,
    VERTICAL,
};

/// @brief Possible values in a board, based on result of a shot made.
enum BoardValue {
    /// @brief WATER is only used by the player to clear a board, or to check if you have a shot in a location or not.
    WATER,
    /// @brief SHIP is only used by the player to store where you placed your ship.
    SHIP,
    /// @brief HIT is returned from shot_return messages when you hit a ship, and to store where you have already shot.
    HIT,
    /// @brief MISS is returned from shot_return messages when you hit water, and to store where you have already shot.
    MISS,
    /// @brief KILL is only used by the player to store a ship in a killed state.
    KILL,
    /// @brief DUPLICATE_HIT is returned from shot_return messages when you hit a ship twice, and to store where you have already shot.
    DUPLICATE_HIT,
    /// @brief DUPLICATE_MISS is returned from shot_return messages when you hit water twice, and to store where you have already shot.
    DUPLICATE_MISS,
    /// @brief DUPLICATE_KILL is returned from shot_return messages when you hit a dead ship twice, and to store where you have already shot.
    DUPLICATE_KILL,
};

/// @brief Possible result of a game.
enum GameResult {
    WIN,
    LOSS,
    TIE,
};

/// @brief A structure used to store ship location information.
/// Also used to send to the server.
struct Ship {
    int row;
    int col;
    int len;
    Direction dir;
};

/// @brief A structure used to store shot location amd value information.
/// Also used to send to the server.
struct Shot {
    int row;
    int col;
    BoardValue value;
};

#endif // DEFINES_H

