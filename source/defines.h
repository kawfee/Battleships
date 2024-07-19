/**
 * @file defines.h
 * @author Matthew Getgen
 * @brief Battleships Common Definitions.
 * @date 2023-08-27
 */

 #ifndef DEFINES_H
 #define DEFINES_H

#include <iostream>
#include <string>
#include <vector>

#define EXEC_DIR        "/ai_files/"
#define PROTECT_DIR     "/protected/"
#define SOCKET_NAME     "/battleships.socket"
#define LOGS_DIR        "/logs/"
#define MATCH_LOG       "/match_log.json"
#define CONTEST_LOG     "/contest_log.json"
#define OPTIONS_FILE    "/options.json"

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
#define PLAYER_1_KEY        "p1"    // also used by the logger
#define PLAYER_2_KEY        "p2"    // also used by the logger
#define SHIP_KEY            "sp"
#define SHOT_KEY            "st"
#define NEXT_SHOT_KEY       "ns"
#define GAME_RESULT_KEY     "gr"    // also used by the logger
#define NUM_BOARD_SHOT_KEY  "nb"    // also used by the logger
#define NUM_HITS_KEY        "nh"    // also used by the logger
#define NUM_MISSES_KEY      "nm"    // also used by the logger
#define NUM_DUPLICATES_KEY  "nd"    // also used by the logger
#define SHIPS_KILLED_KEY    "sk"    // also used by the logger

// JSON LOGGER KEYS -- used by the logger to store values in log
#define ELAPSED_TIME_KEY    "et"
#define WINS_KEY            "W"
#define LOSSES_KEY          "L"
#define TIES_KEY            "T"
#define ERROR_KEY           "err"
#define ERROR_TYPE_KEY      "ert"
#define MESSAGE_KEY         "msg"
#define PLAYERS_KEY         "pls"
#define PLAYER_IDX_KEY      "pid"
#define TOTAL_WINS_KEY      "TW"
#define TOTAL_LOSSES_KEY    "TL"
#define TOTAL_TIES_KEY      "TT"
#define ROUNDS_KEY          "rds"
#define MATCHES_KEY         "mts"
#define LIVES_KEY           "liv"
#define LAST_GAME_KEY       "lg"
#define GAMES_KEY           "gms"
#define SHIPS_KEY           "sps"
#define SHOTS_KEY           "sts"
#define STATS_KEY           "sta"
#define INDEX_SHIP_KEY      "sid"
#define PLAYED_KEY          "pd"

using namespace std;

// SERVER ERROR MESSAGES
const string 
    SOCKET_CREATE_ERR   = "Socket creation failed!",
    SOCKET_NAME_ERR     = "Socket pathname is too long!",
    SOCKET_BIND_ERR     = "Socket binding failed!",
    SOCKET_OPT_ERR      = "Socket option settings failed!",
    PLAYER_FORK_ERR     = "Player process creation failed!",
    PLAYER_EXEC_ERR     = "Player executable failed to run!",
    SOCKET_CONNECT_ERR  = "Connection to player failed!",
    SEND_MESSAGE_ERR    = "Failed to send to player!",
    RECV_MESSAGE_ERR    = "Failed to receive from player!";

// MESSAGE ERROR MESSAGES
const string
    HELLO_MESSAGE_ERR   = "Invalid hello msg from player!",
    SHIP_MESSAGE_ERR    = "Invalid ship msg from player!",
    SHOT_MESSAGE_ERR    = "Invalid shot msg from player!";

// LOGIC ERROR MESSAGES
const string
    BOARD_SIZE_ERR = "Unhandled board size!", // not an ErrorType, never returned
    SHIP_PLACE_ERR = "Invalid ship from player!",
    SHOT_PLACE_ERR = "Invalid shot from player!";


/// @brief Error Number Types for function return values.
enum ErrorType {
    OK,
    // SERVER ERROR TYPES
    ErrFork, ErrConnect, ErrSend, ErrReceive,
    // MESSAGE ERROR TYPES
    ErrHelloMessage, ErrShipPlacedMessage, ErrShotTakenMessage,
    // LOGIC ERROR TYPES
    ErrShipLength, ErrShipOffBoard, ErrShipIntersect, ErrShotOffBoard,
};

/// @brief Number value for different players. This value is passed to the player through start match message. This is important when passing messages.
enum PlayerNum {
    PLAYER_1 = 1, PLAYER_2 = 2,
};

/// @brief Possible direction for a ship.
enum Direction : char {
    HORIZONTAL = 'H', VERTICAL = 'V',
};

/// @brief Possible results of a shot made.
enum BoardValue : char {
    WATER = '~',
    SHIP = 'S',
    HIT = 'X',
    MISS = '*',
    KILL = 'K',
    // NOTE: these duplicate values are set for backwards-compatability.
    DUPLICATE_HIT = 34,
    DUPLICATE_MISS = 35,
    DUPLICATE_KILL = 36,
};

/// @brief Executable values. One is for display, the other is teh full path to the executable.
struct Executable {
    string file_name;
    string exec;
};

/// @brief Type of display for a match chosen by user.
enum MatchDisplayType {
    /// @brief Display the last game (default). Also the only option for a contest display.
    LAST,
    /// @brief Display every. single. game. (bad).
    ALL,
    /// @brief Display the last of every Win, Loss, Tie, Error.
    EACH_TYPE,
    /// @brief Display games at a constant increment.
    INCREMENT,
    /// @brief Display a given game by number.
    CHOICE,
    /// @brief Only display match stats.
    NONE,
};

/// @brief Type of display for a contest chosen by a user.
enum ContestDisplayType {
    /// @brief Displays all matches, rounds, and final.
    NORMAL,
    /// @brief Displays only rounds and final.
    ROUNDS,
    /// @brief Only display the final.
    FINAL,
};

/// @brief The runtime options available.
enum Runtime {
    RunMatch,
    RunContest,
    ReplayMatch,
    ReplayContest,
};

/// @brief Options for a match chosen by a user at runtime.
struct MatchOptions {
    int board_size;
    int num_games;
    int delay_time;
    bool step_through;
    MatchDisplayType display_type;
    Executable exec1;
    Executable exec2;
};

/// @brief Options for a contest chosen by a user at runtime.
struct ContestOptions {
    int board_size;
    int num_games;
    int delay_time;
    ContestDisplayType display_type;
    vector<Executable> execs;
};

/// @brief All options for each runtime type.
struct Options {
    Runtime runtime;
    MatchOptions match_options;
    ContestOptions contest_options;
    // ReplayOptions replay_options;
};

/// @brief Possible result of a game.
enum GameResult : char {
    WIN = 'W', LOSS = 'L', TIE = 'T',
};

/// @brief Struct used to store ship info.
struct Ship {
    int row;
    int col;
    int len;
    bool alive;
    Direction dir;
};

/// @brief Struct used to store shot info.
struct Shot {
    int row;
    int col;
    int ship_sunk_idx;  // NOTE: Optional value used only by logs.
    BoardValue value;
};

/// @brief Struct used to store information about all game errors.
struct Error {
    ErrorType type;
    Ship ship;
    Shot shot;
    string message;
};

/// @brief Data to store for stats, per player, per game.
struct GameStats {
    int num_board_shot;
    int hits;
    int misses;
    int duplicates;
    int ships_killed;
    GameResult result;
};

/// @brief Data to store about each player, per game.
struct GamePlayer {
    vector<Ship> ships;
    vector<Shot> shots;
    GameStats stats;
    Error error;
};

/// @brief Data to store for each game.
struct GameLog {
    GamePlayer player1;
    GamePlayer player2;
};

/// @brief Data to store for stats, per player, per match.
struct MatchStats {
    int wins;
    int losses;
    int ties;
    int total_num_board_shot;
    int total_hits;
    int total_misses;
    int total_duplicates;
    int total_ships_killed;
};

/// @brief Data to store for each player, per match.
struct MatchPlayer {
    string ai_name;
    string author_name;
    MatchStats stats;
    Error error;
};

/// @brief Data to store for each match.
struct MatchLog {
    int board_size;
    float elapsed_time;
    MatchPlayer player1;
    MatchPlayer player2;
    vector<GameLog> games;
};

/// @brief Data to store for stats, for each player, per contest.
struct ContestStats {
    int wins;
    int losses;
    int ties;
    int total_wins;
    int total_losses;
    int total_ties;
};

/// @brief Data to store for each player, per contest.
struct ContestPlayer {
    int lives;
    bool played;
    string ai_name;
    string author_name;
    ContestStats stats;
    Executable exec;
    Error error;
};

/// @brief Data about each player, per match in contest.
struct ContestMatchPlayer {
    int player_idx;
    Executable exec;
    MatchStats stats;
    GameResult match_result;
    Error error;
};

/// @brief Data about each match in contest.
struct ContestMatch {
    float elapsed_time;
    ContestMatchPlayer player1;
    ContestMatchPlayer player2;
    GameLog last_game;
};

/// @brief Data about each round of contests per match.
struct ContestRound {
    vector<ContestMatch> matches;
};

/// @brief Data about the contest.
struct ContestLog {
    int board_size;
    vector<ContestPlayer> players;
    vector<ContestRound> rounds;
};

/// @brief stores the debug state of the process.
extern bool debug;

/// @brief Prints error messages with a specific format.
/// @param error error to print.
/// @param file_name name of the file the error is from.
/// @param line line number of the file.
extern void print_error(const string error, const char *file_name, int line);

 #endif