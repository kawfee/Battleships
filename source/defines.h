/**
 * @file defines.h
 * @author Matthew Getgen
 * @brief Battleships Common Value Definitions
 * @date 2022-05-16
 */

#ifndef DEFINES_H
#define DEFINES_H

#include <iostream>
#include <string>

#include "json.hpp"

#define LOGS_DIR "/logs/"
#define MATCH_LOG "match_log.json"
#define CONTEST_LOG "contest_log.json"

#define MAX_MSG_SIZE 256
#define MAX_NAME_SIZE 64

// JSON MESSAGE KEYS -- used by the client and server to create and parse messages
#define MESSAGE_TYPE_KEY    "mt"
#define PLAYER_NUM_KEY      "pn"
#define AI_NAME_KEY         "ai"    // also used by the logger
#define AUTHOR_NAMES_KEY    "au"    // also used by the logger
#define BOARD_SIZE_KEY      "bs"    // also used by the logger
#define LEN_KEY             "l"     // also used by the logger
#define ROW_KEY             "r"     // also used by the logger
#define COL_KEY             "c"     // also used by the logger
#define DIR_KEY             "d"     // also used by the logger
#define VALUE_KEY           "v"     // also used by the logger
#define OUTCOME_KEY         "o"     // also used by the logger

// JSON LOGGER KEYS -- used by the logger to store values in log
#define PLAYERS_KEY         "pls"
#define PLAYER_IDX_KEY      "pid"
#define TOTAL_WINS_KEY      "tw"
#define TOTAL_LOSSES_KEY    "tl"
#define TOTAL_TIES_KEY      "tt"
#define MATCHES_KEY         "mts"
#define LIVES_KEY           "lvs"
#define LAST_GAME_KEY       "lg"
#define ELAPSED_TIME_KEY    "et"
#define PLAYER_1_KEY        "p1"
#define PLAYER_2_KEY        "p2"
#define WINS_KEY            "W"
#define LOSSES_KEY          "L"
#define TIES_KEY            "T"
#define ERROR_KEY           "e"
#define GAMES_KEY           "gms"
#define SHIPS_KEY           "sps"
#define SHOTS_KEY           "sts"
#define INDEX_SHIP_KEY      "sid"

// SERVER ERRORS
#define SOCKET_CREATE_ERR   "Socket creation failed!"
#define SOCKET_NAME_ERR     "Socket pathname is too long!"
#define SOCKET_BIND_ERR     "Socket binding failed!"
#define SOCKET_OPT_ERR      "Socket option settings failed!"
#define PLAYER_EXEC_ERR     "Player executable failed to run!"   // not an error_num, never returned
#define PLAYER_FORK_ERR     "Process creation failed!"
#define SOCKET_CONNECT_ERR  "Connection failed!"
#define SEND_MESSAGE_ERR    "Message failed to send!"
#define RECV_MESSAGE_ERR    "No message received!"

// MESSAGE ERRORS
#define HELLO_MSG_ERR       "Invalid hello message!"
#define SHIP_MSG_ERR        "Invalid ship_placed message!"
#define SHOT_MSG_ERR        "Invalid shot_taken message!"

// GAME LOGIC ERRORS
#define BOARD_SIZE_ERR      "Unhandled board size!"  // not an error_num, never returned
#define SHIP_PLACE_ERR      "Invalid ship returned!"
#define SHOT_PLACE_ERR      "Invalid shot returned!"

using namespace std;
using json = nlohmann::json;

/**
 * @brief Error Number Types for returning from functions.
 */
enum ErrorNum {
    NO_ERR, BAD_SOCK_CREATE, BAD_SOCK_NAME, BAD_SOCK_BIND, BAD_SOCK_OPT, 
    BAD_FORK, BAD_CONNECT, BAD_SEND, BAD_RECV, 
    BAD_HELLO_MSG, BAD_SHIP_PLACED_MSG, BAD_SHOT_TAKEN_MSG,
    BAD_SHIP, BAD_SHOT,
};

/**
 * @brief number value for different players.
 */
enum PlayerNum {
    PLAYER_1 = 1, PLAYER_2 = 2,
};

/**
 * @brief possible direction of a ship.
 */
enum Direction : char {
    HORIZONTAL = 'H', VERTICAL = 'V',
};

/**
 * @brief possible results of a shot made.
 */
enum BoardValue : char {
    WATER = '~', SHIP = 'S', HIT = 'X', MISS = '*', KILL = 'K', DUPLICATE = '!', DUPLICATE_HIT, DUPLICATE_MISS, DUPLICATE_KILL,
};

/**
 * @brief possible results of a game.
 */
enum GameResult : char {
    WIN = 'W', LOSS = 'L', TIE = 'T',
};

/**
 * @brief Type of display chosen by the user.
 */
enum DisplayType {
    /// @brief Don't display the Match or Contest.
    NONE,
    /// @brief Display every. single. game.
    ALL,
    /// @brief Display the last game. Default for Match, the only way to do it for Contest.
    LAST,
    /// @brief Display each different type if available (WIN, LOSS, TIE, or an ERROR).
    EACH,
    /// @brief Display every Nth game in order.
    INCREMENT,
    /// @brief Contest Display type. Display each match as it's running the contest.
    DURING,
    /// @brief Contest Display type. Display each match after the entire contest has been processed.
    AFTER,
};

/**
 * @brief A structure used to store ship_placed message info.
 */
struct Ship {
    int len;
    int row;
    int col;
    Direction dir;
};

/**
 * @brief A structure used to store shot_taken message info.
 */
struct Shot {
    int row;
    int col;
    BoardValue value;
};

/**
 * @brief Data about the boards and the size of the boards.
 */
struct Board {
    char **board1;
    char **board2;
    int size;
};

/**
 * @brief Data to store about each player for the contest.
 */
struct ContestPlayer {
    string exec;
    char ai_name[MAX_NAME_SIZE];
    char author_name[MAX_NAME_SIZE];
    int lives;
    int total_wins;
    int total_losses;
    int total_ties;
    int idx;
    ErrorNum error;
};

/**
 * @brief Data to store players relevant to a specific match in a contest.
 */
struct ContestMatch {
    ContestPlayer player1;
    ContestPlayer player2;
};

/**
 * @brief Data to store about each player for the match.
 */
struct MatchPlayer {
    char ai_name[MAX_NAME_SIZE];
    char author_name[MAX_NAME_SIZE];
    int wins;
    int losses;
    int ties;
    ErrorNum error;
};

/**
 * @brief Data to store match information. Stores two MatchPlayer instances.
 */
struct Match {
    MatchPlayer player1;
    MatchPlayer player2;
    time_t elapsed_time;
};

// Puts program in debug mode if set.
extern bool debug;

/**
 * @brief Prints error messages with a specific format.
 * @param error error to print.
 * @param file_name name of the file there error is from.
 * @param line line number of the file.
 */
extern void print_error(const char *error, const char *file_name, int line);

#endif

