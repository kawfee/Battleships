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

#define BSHIP_ARENA_BLOCK_SIZE_DEFAULT 4096
#define BSHIP_BOARD_SIZE_MIN 5
#define BSHIP_BOARD_SIZE_MAX 15
#define BSHIP_GAMES_PER_MATCH_MAX 10000
#define BSHIP_GAMES_PER_MATCH_MIN 1
#define BSHIP_MESSAGE_SIZE 256
#define BSHIP_MESSAGE_NAME_SIZE_MAX 96
#define BSHIP_SHIP_COUNT_MIN 3
#define BSHIP_SHIP_COUNT_MAX 10
#define BSHIP_SHIP_LENGTH_MIN 3
#define BSHIP_SHIP_LENGTH_MAX 6
#define BSHIP_SHOT_LENGTH_MAX (BSHIP_BOARD_SIZE_MAX * BSHIP_BOARD_SIZE_MAX)

#define PRINT_ERROR(message) \
    do { \
        fprintf(stderr, "[%s:%d] ERROR: %s\n", __FILE__, __LINE__, message); \
        fflush(stderr); \
    } while (0)

#define PRINT_ERROR_F(fmt, ...) \
    do { \
        fprintf(stderr, "[%s:%d] ERROR: " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); \
        fflush(stderr); \
    } while (0)

typedef enum {
    BSHIP_PLAYER_1 = 1,
    BSHIP_PLAYER_2 = 2,
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

typedef struct BShip_ArenaBlock BShip_ArenaBlock;

struct BShip_ArenaBlock {
    BShip_ArenaBlock *previous;
    size_t capacity;
    size_t offset;

    // NOTE(mattg): this must be the last element in order to point at the memory after the struct.
    uint8_t memory[];
};

typedef struct {
    BShip_ArenaBlock *block;
    size_t offset;
} BShip_ArenaMark;

typedef struct {
    BShip_ArenaBlock *first;
    BShip_ArenaBlock *current;
} BShip_Arena;

typedef struct {
    uint8_t row;
    uint8_t column;
    uint8_t length;
    BShip_Direction direction;
} BShip_Ship;

typedef struct {
    BShip_Ship *buffer;
    uint32_t length;
    uint32_t capacity;
} BShip_ShipArray;

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
    uint8_t *buffer;
    uint8_t size;
} BShip_Board;

typedef struct {
    char *buffer;
    uint8_t length;
} BShip_Message;

typedef struct {
    uint8_t *buffer;
    uint32_t length;
    uint32_t capacity;
} BShip_U8Array;

typedef enum {
    ERROR_SUCCESS,
    // Platform-specific errors
    ERROR_AI_PATH_ISSUE,
    ERROR_PROCESS_FAILED,
    ERROR_CONNECTION_FAILED,
    ERROR_CONNECTION_TIMEOUT,
    ERROR_SEND_FAILED,
    ERROR_SEND_TIMEOUT,
    ERROR_RECEIVE_FAILED,
    ERROR_RECEIVE_TIMEOUT,
    ERROR_RECEIVE_EMPTY_MESSAGE,
    // Message errors
    ERROR_MESSAGE_HELLO_INVALID,
    ERROR_MESSAGE_SHIPS_PLACED_INVALID,
    ERROR_MESSAGE_SHOT_TAKEN_INVALID,
    // Logic Errors
    ERROR_SHIP_LENGTH_INVALID,
    ERROR_SHIP_OFF_BOARD,
    ERROR_SHIP_OVERLAP,
    ERROR_SHOT_OFF_BOARD,
    ERROR_SHOT_DUPLICATE,
} BShip_ErrorType;

typedef struct {
    BShip_ErrorType type;
    BShip_Ship ship;
    BShip_Shot shot;
    BShip_Message message;
    int32_t exit_status;
} BShip_Error;

typedef struct {
    BShip_Error error;
    BShip_ShipArray ships;
    BShip_U8Array alive_ships;
    BShip_U8Array dead_ships;
    BShip_ShotArray shots;
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
    char *name;
    char *authors;
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


#ifdef __cplusplus
extern "C" {
#endif

void BShip_Arena_Initialize(BShip_Arena *arena, size_t capacity);
void BShip_Arena_Destroy(BShip_Arena *arena);
void *BShip_Arena_Push(BShip_Arena *arena, size_t size);
void BShip_Arena_Reset(BShip_Arena *arena);
BShip_ArenaMark BShip_ArenaMark_Get(BShip_Arena *arena);
void BShip_Arena_Rollback(BShip_Arena *arena, BShip_ArenaMark mark);

BShip_Board BShip_Board_Allocate(BShip_Arena *arena, uint8_t board_size);
BShip_BoardValue BShip_Board_Get(BShip_Board board, uint8_t row, uint8_t column);
void BShip_Board_Set(BShip_Board board, uint8_t row, uint8_t column, BShip_BoardValue value);

// void BShip_Contest_Run(const char *socket_path, const char *ai_paths[], uint32_t ai_paths_length,
//     uint8_t board_size, uint32_t games_per_match, BShip_ContestAlgorithm algorithm, bool debug);

size_t BShip_Match_CalculateMemorySize(uint8_t board_size, uint32_t games_per_match);

BShip_MatchData BShip_Match_Run(BShip_Arena *arena, const char *socket_path,
    const char *ai1_path, const char *ai2_path, uint8_t board_size, uint32_t games_per_match, bool debug);

#ifdef __cplusplus
}
#endif

#endif // BATTLESHIPSLIB_H

