/**
 * @file defines.h
 * @author Matthew Getgen
 * @brief Standard Definitions for all Player AIs.
 * @date 2022-11-22
 */

#ifndef DEFINES_H
#define DEFINES_H

#define MAX_MSG_SIZE 256
#define MAX_NAME_SIZE 64

// JSON MESSAGE KEYS -- used by the player and server to create and parse messages
#define MESSAGE_TYPE_KEY    "mt"
#define PLAYER_NUM_KEY      "pn"
#define AI_NAME_KEY         "ai"
#define AUTHOR_NAMES_KEY    "au"
#define BOARD_SIZE_KEY      "bs"
#define LEN_KEY             "l"
#define ROW_KEY             "r"
#define COL_KEY             "c"
#define DIR_KEY             "d"
#define VALUE_KEY           "v"
#define PLAYER_1_KEY        "p1"
#define PLAYER_2_KEY        "p2"
#define SHIP_KEY            "sp"
#define SHOT_KEY            "st"
#define NEXT_SHOT_KEY       "ns"
#define GAME_RESULT_KEY     "gr"
#define NUM_BOARD_SHOT_KEY  "nb"
#define NUM_HITS_KEY        "nh"
#define NUM_MISSES_KEY      "nm"
#define NUM_DUPLICATES_KEY  "nd"
#define SHIPS_KILLED_KEY    "sk"

/// @brief Message Types that are sent and received. Numbered in order of occurrence.
enum MessageType {
    /// @brief SERVER MESSAGE TYPES -- used by the server to create messages, used by the client to parse messages
    setup_match = 2, start_game = 3, place_ship = 4, take_shot = 6, shot_return = 8, game_over = 9, match_over = 10,
    /// @brief CLIENT MESSAGE TYPES -- used by client to create messages, used by server to parse messages
    hello = 1, ship_placed = 5, shot_taken = 7,
};

/// @brief Number values for different players.
enum PlayerNum {
    /// @brief Whether you are PLAYER 1 or PLAYER 2.
    PLAYER_1 = 1, PLAYER_2 = 2,
};

/// @brief possible directions of a ship.
enum Direction : char {
    /// @brief Whether the ship is HORIZONTAL or VERTICAL.
    HORIZONTAL = 'H', VERTICAL = 'V',
};

/// @brief Possible values in a board, based on result of a shot made.
enum BoardValue : char {
    /// @brief WATER is only used by the player to clear a board, or to check if you have a shot in a location or not.
    WATER = '~',
    /// @brief SHIP is only used by the player to store where you placed your ship.
    SHIP = 'S',
    /// @brief HIT is returned from shot_return messages when you hit a ship, and to store where you have already shot.
    HIT = 'X',
    /// @brief MISS is returned from shot_return messages when you hit water, and to store where you have already shot.
    MISS = '*',
    /// @brief KILL is only used by the player to store a ship in a killed state.
    KILL = 'K',
    /// @brief DUPLICATE_HIT is returned from shot_return messages when you hit a ship twice, and to store where you have already shot.
    DUPLICATE_HIT = 34,
    /// @brief DUPLICATE_MISS is returned from shot_return messages when you hit water twice, and to store where you have already shot.
    DUPLICATE_MISS = 35,
    /// @brief DUPLICATE_KILL is returned from shot_return messages when you hit a dead ship twice, and to store where you have already shot.
    DUPLICATE_KILL = 36,
};

/// @brief Possible result of a game.
enum GameResult : char {
    WIN = 'W', LOSS = 'L', TIE = 'T',
};

/// @brief A structure used to store ship location information.
/// Also used to send to the server.
struct Ship {
    int len;
    int row;
    int col;
    Direction dir;
};

/// @brief A structure used to store shot location amd value information.
/// Also used to send to the server.
struct Shot {
    int row;
    int col;
    BoardValue value;
};

/// @brief A structure used to store game stats information.
struct GameStats {
    int num_board_shot;
    int hits;
    int misses;
    int duplicates;
    int ships_killed;
    GameResult result;
};

/// @brief A structure used to store match stats information.
struct MatchStats {
    int total_num_board_shot;
    int total_hits;
    int total_misses;
    int total_duplicates;
    int total_ships_killed;
    int wins;
    int losses;
    int ties;
};

#endif

