/**
 * @file battleshipslib.h
 * @author Matthew Getgen
 * @brief Battleships cross-platform Library Definitions.
 * @date 2026-05-09
 */

#ifndef BATTLESHIPSLIB_H
#define BATTLESHIPSLIB_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define BSHIP_BOARD_SIZE_MAX 10
#define BSHIP_BOARD_SIZE_MIN 3
#define BSHIP_GAMES_PER_MATCH_MAX 10000
#define BSHIP_GAMES_PER_MATCH_MIN 1
#define BSHIP_SHIP_COUNT_MAX 6
#define BSHIP_SHIP_LENGTH_MAX 5
#define BSHIP_SHIP_LENGTH_MIN 1
#define BSHIP_SHOT_LENGTH_MAX (BSHIP_BOARD_SIZE_MAX * BSHIP_BOARD_SIZE_MAX)

#define PRINT_ERROR(message) \
    do { \
        fprintf(stderr, "[%s:%d] ERROR: %s\n", __FILE__, __LINE__, message); \
        fflush(stderr); \
    } while (0);

#define PRINT_ERROR_F(fmt, ...) \
    do { \
        fprintf(stderr, "[%s:%d] ERROR: " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); \
        fflush(stderr); \
    } while (0);

typedef enum {
    BSHIP_PLAYER_1,
    BSHIP_PLAYER_2,
} BShip_PlayerNum;

typedef enum {
    BSHIP_HORIZONTAL,
    BSHIP_VERTICAL,
} BShip_Direction;

typedef enum {
    BSHIP_WATER,
    BSHIP_SHIP,
    BSHIP_HIT,
    BSHIP_MISS,
    BSHIP_KILL,
    BSHIP_DUPLICATE_HIT,
    BSHIP_DUPLICATE_MISS,
    BSHIP_DUPLICATE_KILL,
} BShip_BoardValue;

typedef enum {
    BSHIP_WIN,
    BSHIP_LOSS,
    BSHIP_TIE,
} BShip_GameResult;

typedef struct {
    uint8_t row;
    uint8_t column;
    uint8_t length;
    BShip_Direction direction;
    bool alive;
} BShip_Ship;

typedef struct {
    BShip_Ship *buffer;
    uint32_t length;
    uint32_t capacity;
} BShip_ShipArray;

typedef struct {
    uint32_t start_index;
    uint32_t length;
} BShip_ShipArraySlice;

typedef struct {
    uint8_t row;
    uint8_t column;
    BShip_BoardValue value;
} BShip_Shot;

typedef struct {
    BShip_Shot *buffer;
    uint32_t length;
    uint32_t capacity;
} BShip_ShotArray;

typedef struct {
    uint32_t start_index;
    uint32_t length;
} BShip_ShotArraySlice;

typedef struct {
    char *json;
    size_t length;
} BShip_Message;

typedef enum {
    ERROR_SUCCESS,
    // Platform-specific errors
    ERROR_AI_PATH_ISSUE,
    ERROR_PROCESS_FAILED,
    ERROR_CONNECTION_FAILED,
    ERROR_SEND_FAILED,
    ERROR_RECEIVE_FAILED,
    // Message errors
    ERROR_INVALID_HELLO_MESSAGE,
    ERROR_INVALID_SHIPS_PLACED_MESSAGE,
    ERROR_INVALID_SHOT_TAKEN_MESSAGE,
    // Logic Errors
    ERROR_INVALID_SHIP_LENGTH,
    ERROR_SHIP_OFF_BOARD,
    ERROR_SHIP_OVERLAP,
    ERROR_SHOT_OFF_BOARD,
} BShip_ErrorType;

typedef struct {
    BShip_ErrorType type;
    BShip_Ship ship;
    BShip_Shot shot;
    int8_t *message;
    int32_t message_length;
} BShip_Error;

typedef struct {
    BShip_Error error;
    BShip_ShipArraySlice ships;
    BShip_ShotArraySlice shots;
    uint32_t num_board_shot;
    uint32_t hits;
    uint32_t misses;
    uint32_t duplicates;
    uint32_t ships_killed;
    uint32_t ship_start_idx;
    uint32_t shot_start_idx;
    BShip_GameResult result;
} BShip_AIGameData;

typedef struct {
    BShip_AIGameData ai1;
    BShip_AIGameData ai2;
} BShip_GameData;

typedef struct {
    BShip_GameData *buffer;
    uint32_t length;
    uint32_t capacity;
} BShip_GameDataArray;

typedef struct {
    BShip_Error error;
    int8_t *ai_name;
    int8_t *author_name;
    uint32_t ai_name_length;
    uint32_t author_name_length;
    uint32_t wins;
    uint32_t losses;
    uint32_t ties;
    uint32_t total_num_board_shot;
    uint32_t total_hits;
    uint32_t total_misses;
    uint32_t total_duplicates;
    uint32_t total_ships_killed;
} BShip_AIMatchData;

typedef struct {
    BShip_AIMatchData ai1;
    BShip_AIMatchData ai2;
    BShip_GameDataArray games;
    float elapsed_time;
    uint32_t games_per_match;
    uint8_t board_size;
} BShip_MatchData;

typedef enum {
    CONTEST_CLASSIC,
    CONTEST_ROUND_ROBIN,
} BShip_ContestAlgorithm;

void BShip_RunContest(const char *socket_path, const char *ai_paths[], uint32_t ai_paths_length,
    uint8_t board_size, uint32_t games_per_match, BShip_ContestAlgorithm algorithm, bool debug);

BShip_MatchData BShip_RunMatch(const char *socket_path, const char *ai1_path, const char *ai2_path,
    uint8_t board_size, uint32_t games_per_match, bool debug);

#endif // BATTLESHIPSLIB_H

