/**
 * @file runtime.c
 * @author Matthew Getgen
 * @brief Runtime logic for running games, matches, and contests.
 * @date 2026-05-27
 */

#define _GNU_SOURCE
#include <math.h>

#include "arena.c"
#include "message.c"
#include "game.c"
#include "contest.c"

BSHIP_DEFINE_ARRAY_PUSH(BShip_ShipArray, BShip_Ship)
BSHIP_DEFINE_ARRAY_PUSH(BShip_ShotArray, BShip_Shot)
BSHIP_DEFINE_ARRAY_PUSH(BShip_EventArray, BShip_Event)
BSHIP_DEFINE_ARRAY_PUSH(BShip_U32Array, uint32_t)

size_t BShip_Game_CalculateMemorySize(uint8_t board_size)
{
    size_t ship_count_max = (size_t)ShipCountMax_From_BoardSize(board_size);
    size_t per_ai_mem_size = (sizeof(BShip_Ship) * ship_count_max) + (sizeof(uint8_t) * ship_count_max * 4)
        + (sizeof(uint8_t) * board_size * board_size);
    return per_ai_mem_size * 2;
}

bool BShip_Game_Run(BShip_Arena *arena, BShip_Connection *conn,
    BShip_AIConnection *ai1_conn, BShip_AIConnection *ai2_conn,
    BShip_MatchData *match, BShip_Message *ai1_message, BShip_Message *ai2_message,
    uint8_t board_size, bool debug)
{
    assert(arena != NULL);
    assert(conn != NULL);
    assert(ai1_conn != NULL);
    assert(ai2_conn != NULL);
    assert(match != NULL);
    assert(ai1_message != NULL);
    assert(ai1_message->buffer != NULL);
    assert(ai2_message != NULL);
    assert(ai2_message->buffer != NULL);
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);

    bool okay = true;

    uint8_t ship_count_max = ShipCountMax_From_BoardSize(board_size);
    uint32_t shot_count_max = board_size * board_size;

    typedef struct {
        BShip_ShipArray ships;
        BShip_U8Array alive_ships;
        BShip_U8Array dead_ships;
        uint32_t ship_index_start;
    } BShip_AIGameState;

    BShip_AIGameState ai1 = {
        .ships = {
            .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_Ship, ship_count_max),
            .capacity = ship_count_max,
        },
        .alive_ships = {
            .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint8_t, ship_count_max),
            .capacity = ship_count_max,
        },
        .dead_ships = {
            .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint8_t, ship_count_max),
            .capacity = ship_count_max,
        },
        .ship_index_start = match->ai1.ships.length,
    };

    BShip_AIGameState ai2 = {
        .ships = {
            .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_Ship, ship_count_max),
            .capacity = ship_count_max,
        },
        .alive_ships = {
            .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint8_t, ship_count_max),
            .capacity = ship_count_max,
        },
        .dead_ships = {
            .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint8_t, ship_count_max),
            .capacity = ship_count_max,
        },
        .ship_index_start = match->ai2.ships.length,
    };
    if (ai1.ships.buffer == NULL || ai1.alive_ships.buffer == NULL || ai1.dead_ships.buffer == NULL ||
        ai2.ships.buffer == NULL || ai2.alive_ships.buffer == NULL || ai2.dead_ships.buffer == NULL)
    {
        okay = false;
        goto on_game_end;
    }

    BShip_Board ai1_board = BShip_Board_Allocate(arena, board_size);
    BShip_Board ai2_board = BShip_Board_Allocate(arena, board_size);
    if (ai1_board.buffer == NULL || ai2_board.buffer == NULL)
    {
        okay = false;
        goto on_game_end;
    }

    BShip_U8Array ship_lengths = {
        .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint8_t, ship_count_max),
        .capacity = ship_count_max,
    };
    if (ship_lengths.buffer == NULL)
    {
        okay = false;
        goto on_game_end;
    }
    ShipLengths_Calculate(&ship_lengths, board_size);
    // NOTE(mattg): we need a copy of this to use when comparing ship lengths for both AIs.
    BShip_U8Array ship_lengths_copy = {
        .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint8_t, ship_lengths.capacity),
        .length = ship_lengths.length,
        .capacity = ship_lengths.capacity,
    };
    if (ship_lengths_copy.buffer == NULL)
    {
        okay = false;
        goto on_game_end;
    }
    memcpy(ship_lengths_copy.buffer, ship_lengths.buffer, ship_lengths.capacity * sizeof(uint8_t));

    BShip_Event game_start_event = {
        .type = BSHIP_EVENT_GAME_START,
    };
    BShip_EventArray_Push(&match->events, game_start_event);

    BShip_Message_PlaceShips_Create(ai1_message, ship_lengths.buffer, ship_lengths.length);
    match->ai1.error.type = BShip_AIConnection_Send(ai1_conn, *ai1_message, debug);
    match->ai2.error.type = BShip_AIConnection_Send(ai2_conn, *ai1_message, debug);
    if (match->ai1.error.type != ERROR_SUCCESS || match->ai2.error.type != ERROR_SUCCESS)
    {
        goto on_game_end;
    }

    match->ai1.error.type = BShip_AIConnection_Receive(ai1_conn, ai1_message, debug);
    match->ai2.error.type = BShip_AIConnection_Receive(ai2_conn, ai2_message, debug);
    if (match->ai1.error.type != ERROR_SUCCESS || match->ai2.error.type != ERROR_SUCCESS)
    {
        goto on_game_end;
    }

    match->ai1.error = BShip_Message_ShipsPlaced_Parse(*ai1_message, &ai1.ships, ship_lengths.length);
    match->ai2.error = BShip_Message_ShipsPlaced_Parse(*ai2_message, &ai2.ships, ship_lengths.length);
    if (match->ai1.error.type != ERROR_SUCCESS || match->ai2.error.type != ERROR_SUCCESS)
    {
        goto on_game_end;
    }
    assert(ai1.ships.length == ship_lengths.length);
    assert(ai2.ships.length == ship_lengths.length);

    size_t ship_count = ship_lengths.length;
    for (size_t i = 0; i < ship_count; i++)
    {
        match->ai1.error = ValidateAndStoreShip(ai1_board, ai1.ships.buffer[i], &ship_lengths);
        match->ai2.error = ValidateAndStoreShip(ai2_board, ai2.ships.buffer[i], &ship_lengths_copy);
        if (match->ai1.error.type != ERROR_SUCCESS || match->ai2.error.type != ERROR_SUCCESS)
        {
            goto on_game_end;
        }
        BShip_U8Array_Push(&ai1.alive_ships, i);
        BShip_U8Array_Push(&ai2.alive_ships, i);

        BShip_Event ship_place_event = {
            .type = BSHIP_EVENT_SHIP_PLACEMENT,
            .value.indexes.ai1_ship_index = match->ai1.ships.length,
            .value.indexes.ai2_ship_index = match->ai2.ships.length,
        };
        BShip_EventArray_Push(&match->events, ship_place_event);

        BShip_ShipArray_Push(&match->ai1.ships, ai1.ships.buffer[i]);
        BShip_ShipArray_Push(&match->ai2.ships, ai2.ships.buffer[i]);
    }

    bool next_shot = true;
    for (uint32_t i = 0; i < shot_count_max; i++)
    {
        if (i == (shot_count_max - 1))
        {
            next_shot = false;
        }

        match->ai1.error.type = BShip_AIConnection_Receive(ai1_conn, ai1_message, debug);
        match->ai2.error.type = BShip_AIConnection_Receive(ai2_conn, ai2_message, debug);
        if (match->ai1.error.type != ERROR_SUCCESS || match->ai2.error.type != ERROR_SUCCESS)
        {
            goto on_game_end;
        }

        BShip_Shot ai1_shot = {0};
        BShip_Shot ai2_shot = {0};
        match->ai1.error = BShip_Message_ShotTaken_Parse(*ai1_message, &ai1_shot);
        match->ai2.error = BShip_Message_ShotTaken_Parse(*ai2_message, &ai2_shot);
        if (match->ai1.error.type != ERROR_SUCCESS || match->ai2.error.type != ERROR_SUCCESS)
        {
            goto on_game_end;
        }

        match->ai1.error = ValidateAndStoreShot(ai2_board, &ai1_shot);
        match->ai2.error = ValidateAndStoreShot(ai1_board, &ai2_shot);
        if (match->ai1.error.type != ERROR_SUCCESS || match->ai2.error.type != ERROR_SUCCESS)
        {
            goto on_game_end;
        }

        uint32_t ai1_dead_ship_index = 0, ai2_dead_ship_index = 0;
        bool ai1_ship_dead = false, ai2_ship_dead = false;
        if (ai2_shot.value == BSHIP_HIT)
        {
            ai1_ship_dead = FindAndStoreDeadShip(ai1_board, ai1.ships,
                &ai1.alive_ships, &ai1.dead_ships, &ai1_dead_ship_index);
        }
        if (ai1_shot.value == BSHIP_HIT)
        {
            ai2_ship_dead = FindAndStoreDeadShip(ai2_board, ai2.ships,
                &ai2.alive_ships, &ai2.dead_ships, &ai2_dead_ship_index);
        }

        BShip_Event shot_result_event = {
            .type = BSHIP_EVENT_SHOT_RESULT,
            .value.indexes.ai1_ship_index = ai1_ship_dead ? ai1.ship_index_start + ai1_dead_ship_index : 0,
            .value.indexes.ai2_ship_index = ai2_ship_dead ? ai2.ship_index_start + ai2_dead_ship_index : 0,
            .value.indexes.ai1_shot_index = match->ai1.shots.length,
            .value.indexes.ai2_shot_index = match->ai2.shots.length,
        };
        BShip_EventArray_Push(&match->events, shot_result_event);

        BShip_ShotArray_Push(&match->ai1.shots, ai1_shot);
        BShip_ShotArray_Push(&match->ai2.shots, ai2_shot);

        if (ai1.alive_ships.length == 0 || ai2.alive_ships.length == 0)
        {
            next_shot = false;
        }

        BShip_Ship *ai1_dead_ship = NULL;
        BShip_Ship *ai2_dead_ship = NULL;
        if (ai1_ship_dead) ai1_dead_ship = &ai1.ships.buffer[ai1_dead_ship_index];
        if (ai2_ship_dead) ai2_dead_ship = &ai2.ships.buffer[ai2_dead_ship_index];
        BShip_Message_ShotResult_Create(ai1_message, ai1_shot, ai2_shot, 
            ai1_dead_ship, ai2_dead_ship, next_shot);

        match->ai1.error.type = BShip_AIConnection_Send(ai1_conn, *ai1_message, debug);
        match->ai2.error.type = BShip_AIConnection_Send(ai2_conn, *ai1_message, debug);
        if (match->ai1.error.type != ERROR_SUCCESS || match->ai2.error.type != ERROR_SUCCESS)
        {
            goto on_game_end;
        }

        if (!next_shot)
        {
            break;
        }
    }

on_game_end:
    ; // NOTE(mattg): Keep this here to silence the compiler
    BShip_GameResult ai1_game_result = BSHIP_WIN;
    // NOTE(mattg): error order precedence: any error occuring which precedes 0 alive ships left
    // which precedes the count of ships still alive.
    bool ai1_errored = match->ai1.error.type != ERROR_SUCCESS;
    bool ai2_errored = match->ai2.error.type != ERROR_SUCCESS;
    if      (!ai1_errored && ai2_errored) ai1_game_result = BSHIP_WIN;
    else if (ai1_errored && !ai2_errored) ai1_game_result = BSHIP_LOSS;
    else
    {
        bool ai1_has_ships = !!ai1.alive_ships.length;
        bool ai2_has_ships = !!ai2.alive_ships.length;
        if      (ai1_has_ships && !ai2_has_ships) ai1_game_result = BSHIP_WIN;
        else if (!ai1_has_ships && ai2_has_ships) ai1_game_result = BSHIP_LOSS;
        else
        {
            size_t ai1_alive_count = ai1.alive_ships.length;
            size_t ai2_alive_count = ai2.alive_ships.length;
            if      (ai1_alive_count < ai2_alive_count) ai1_game_result = BSHIP_WIN;
            else if (ai1_alive_count > ai2_alive_count) ai1_game_result = BSHIP_LOSS;
            else                                        ai1_game_result = BSHIP_TIE;
        }
    }
    BShip_Event game_result_event = {
        .type = BSHIP_EVENT_GAME_RESULT,
        .value.ai1_game_result = ai1_game_result,
    };
    BShip_EventArray_Push(&match->events, game_result_event);

    if (match->ai1.error.type != ERROR_SUCCESS || match->ai2.error.type != ERROR_SUCCESS)
    {
        okay = false;
    }

    return okay;
}

size_t BShip_Match_CalculateMemorySize(uint8_t board_size, uint32_t games_per_match)
{
    size_t ship_count_max = (size_t)ShipCountMax_From_BoardSize(board_size) * games_per_match;
    size_t shot_count_max = board_size * board_size * games_per_match;
    size_t game_temp_mem_size = BShip_Game_CalculateMemorySize(board_size);
    size_t per_ai_mem_size = (BSHIP_MESSAGE_NAME_SIZE_MAX * 4) + (BSHIP_MESSAGE_SIZE * 2)
        + (sizeof(BShip_Ship) * (ship_count_max+1)) + (sizeof(BShip_Shot) * (shot_count_max+1))
        + BShip_AIConnection_GetSize();
    return game_temp_mem_size + (per_ai_mem_size * 2) + BShip_Connection_GetSize() +
        (sizeof(uint32_t) * games_per_match) +
        (sizeof(BShip_Event) * (ship_count_max + shot_count_max + (games_per_match * 2)));
}

BShip_MatchData BShip_Match_Run(BShip_Arena *arena, char *socket_path,
    BShip_AIFileData ai1_file_data, BShip_AIFileData ai2_file_data,
    uint8_t board_size, uint32_t games_per_match, bool debug)
{
    BShip_MatchData match = {0};
    if (
        socket_path == NULL ||
        board_size < BSHIP_BOARD_SIZE_MIN || board_size > BSHIP_BOARD_SIZE_MAX ||
        games_per_match < BShip_GamesPerMatchMin_From_BoardSize(board_size) ||
        games_per_match > BShip_GamesPerMatchMax_From_BoardSize(board_size) ||
        ai1_file_data.file_path == NULL || ai1_file_data.runtime_directory == NULL ||
        ai2_file_data.file_path == NULL || ai2_file_data.runtime_directory == NULL ||
        !BShip_PathIsExecutable(ai1_file_data.file_path) ||
        !BShip_PathIsDirectory(ai1_file_data.runtime_directory) ||
        !BShip_PathIsExecutable(ai2_file_data.file_path) ||
        !BShip_PathIsDirectory(ai2_file_data.runtime_directory)
        )
    {
        return match;
    }

    match.games_per_match = games_per_match;
    match.board_size = board_size;
    match.ai1.error.type = ERROR_SUCCESS;
    match.ai2.error.type = ERROR_SUCCESS;

    BShip_Message ai1_message = {
        .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_SIZE),
        .length = 0,
    };
    BShip_Message ai2_message = {
        .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_SIZE),
        .length = 0,
    };
    if (ai1_message.buffer == NULL || ai2_message.buffer == NULL)
    {
        return match;
    }
    {
        match.ai1.name = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_NAME_SIZE_MAX);
        match.ai1.authors = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_NAME_SIZE_MAX);
        if (match.ai1.name == NULL || match.ai1.authors == NULL)
        {
            return match;
        }
        memset(match.ai1.name, 0, BSHIP_MESSAGE_NAME_SIZE_MAX);
        memset(match.ai1.authors, 0, BSHIP_MESSAGE_NAME_SIZE_MAX);
    }
    {
        match.ai2.name = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_NAME_SIZE_MAX);
        match.ai2.authors = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_NAME_SIZE_MAX);
        if (match.ai2.name == NULL || match.ai2.authors == NULL)
        {
            return match;
        }
        memset(match.ai2.name, 0, BSHIP_MESSAGE_NAME_SIZE_MAX);
        memset(match.ai2.authors, 0, BSHIP_MESSAGE_NAME_SIZE_MAX);
    }
    uint32_t ship_count_max = ShipCountMax_From_BoardSize(match.board_size) * match.games_per_match;
    {
        uint32_t ships_capacity = ship_count_max+1;
        match.ai1.ships.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_Ship, ships_capacity);
        match.ai1.ships.capacity = ships_capacity;
        match.ai2.ships.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_Ship, ships_capacity);
        match.ai2.ships.capacity = ships_capacity;
        if (match.ai1.ships.buffer == NULL || match.ai2.ships.buffer == NULL)
        {
            return match;
        }
        BShip_Ship empty_ship = {0};
        BShip_ShipArray_Push(&match.ai1.ships, empty_ship);
        BShip_ShipArray_Push(&match.ai2.ships, empty_ship);
    }
    uint32_t shot_count_max = match.board_size * match.board_size * match.games_per_match;
    {
        uint32_t shots_capacity = shot_count_max+1;
        match.ai1.shots.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_Shot, shots_capacity);
        match.ai1.shots.capacity = shots_capacity;
        match.ai2.shots.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_Shot, shots_capacity);
        match.ai2.shots.capacity = shots_capacity;
        if (match.ai1.shots.buffer == NULL || match.ai2.shots.buffer == NULL)
        {
            return match;
        }
        BShip_Shot empty_shot = {0};
        BShip_ShotArray_Push(&match.ai1.shots, empty_shot);
        BShip_ShotArray_Push(&match.ai2.shots, empty_shot);
    }
    match.game_indexes.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint32_t, match.games_per_match);
    match.game_indexes.capacity = match.games_per_match;
    if (match.game_indexes.buffer == NULL)
    {
        return match;
    }
    uint32_t events_max = ship_count_max + shot_count_max + (match.games_per_match * 2);
    match.events.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_Event, events_max);
    match.events.capacity = events_max;
    if (match.events.buffer == NULL)
    {
        return match;
    }

    BShip_Connection *conn = BShip_Arena_Push(arena, BShip_Connection_GetSize());
    if (conn == NULL)
    {
        return match;
    }

    if (!BShip_Connection_Create(conn, socket_path))
    {
        goto on_conn_create_error;
    }

    BShip_AIConnection *ai1_conn = BShip_Arena_Push(arena, BShip_AIConnection_GetSize());
    BShip_AIConnection *ai2_conn = BShip_Arena_Push(arena, BShip_AIConnection_GetSize());
    if (ai1_conn == NULL || ai2_conn == NULL)
    {
        goto on_conn_create_error;
    }

    match.ai1.error.type = BShip_AIConnection_StartProcess(ai1_conn, socket_path,
        ai1_file_data.file_path, ai1_file_data.runtime_directory);
    match.ai2.error.type = BShip_AIConnection_StartProcess(ai2_conn, socket_path,
        ai2_file_data.file_path, ai2_file_data.runtime_directory);
    if (match.ai1.error.type != ERROR_SUCCESS || match.ai2.error.type != ERROR_SUCCESS)
    {
        goto on_process_error;
    }

    match.ai1.error.type = BShip_AIConnection_Accept(ai1_conn, conn, debug);
    match.ai2.error.type = BShip_AIConnection_Accept(ai2_conn, conn, debug);
    if (match.ai1.error.type != ERROR_SUCCESS || match.ai2.error.type != ERROR_SUCCESS)
    {
        goto on_conn_accept_error;
    }

    match.ai1.error.type = BShip_AIConnection_Receive(ai1_conn, &ai1_message, debug);
    match.ai2.error.type = BShip_AIConnection_Receive(ai2_conn, &ai2_message, debug);
    if (match.ai1.error.type != ERROR_SUCCESS || match.ai2.error.type != ERROR_SUCCESS)
    {
        goto on_conn_accept_error;
    }

    match.ai1.error = BShip_Message_Hello_Parse(ai1_message, match.ai1.name, match.ai1.authors);
    match.ai2.error = BShip_Message_Hello_Parse(ai2_message, match.ai2.name, match.ai2.authors);
    if (match.ai1.error.type != ERROR_SUCCESS || match.ai2.error.type != ERROR_SUCCESS)
    {
        goto on_conn_accept_error;
    }

    BShip_Message_SetupMatch_Create(&ai1_message, match.board_size, BSHIP_PLAYER_1);
    BShip_Message_SetupMatch_Create(&ai2_message, match.board_size, BSHIP_PLAYER_2);

    match.ai1.error.type = BShip_AIConnection_Send(ai1_conn, ai1_message, debug);
    match.ai2.error.type = BShip_AIConnection_Send(ai2_conn, ai1_message, debug);
    if (match.ai1.error.type != ERROR_SUCCESS || match.ai2.error.type != ERROR_SUCCESS)
    {
        goto on_conn_accept_error;
    }

    for (size_t i = 0; i < match.games_per_match; i++)
    {
        BShip_U32Array_Push(&match.game_indexes, match.events.length);

        BSHIP_ARENA_TEMP_BEGIN(arena);
        bool okay = BShip_Game_Run(arena, conn, ai1_conn, ai2_conn,
                &match, &ai1_message, &ai2_message, board_size, debug);
        BSHIP_ARENA_TEMP_END(arena);
        if (!okay || match.ai1.error.type != ERROR_SUCCESS || match.ai2.error.type != ERROR_SUCCESS)
        {
            break;
        }
    }

    // NOTE(mattg): We want this to happen at the end anyway, so don't early exit.
    {
        BSHIP_ARENA_TEMP_BEGIN(arena);
        BShip_Message message = {
            .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_SIZE),
        };
        if (message.buffer == NULL)
        {
            goto on_conn_accept_error;
        }

        BShip_Message_MatchOver_Create(&message);

        if (match.ai1.error.type == ERROR_SUCCESS)
        {
            BShip_AIConnection_Send(ai1_conn, message, debug);
        }
        if (match.ai2.error.type == ERROR_SUCCESS)
        {
            BShip_AIConnection_Send(ai2_conn, message, debug);
        }
        BSHIP_ARENA_TEMP_END(arena);
    }
on_conn_accept_error:
    BShip_AIConnection_Close(ai1_conn);
    BShip_AIConnection_Close(ai2_conn);
on_process_error:
    // TODO(mattg): hook this up with the error handling (status code, exited vs hung)
    if (!BShip_AIConnection_WaitProcess(ai1_conn, debug))
    {
        BShip_AIConnection_KillProcess(ai1_conn);
    }
    if (!BShip_AIConnection_WaitProcess(ai2_conn, debug))
    {
        BShip_AIConnection_KillProcess(ai2_conn);
    }
on_conn_create_error:
    BShip_Connection_Close(conn);
    return match;
}

BShip_Board BShip_Board_Allocate(BShip_Arena *arena, uint8_t board_size)
{
    assert(arena != NULL);
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);
    BShip_Board board = {
        .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint8_t, board_size * board_size),
        .size = board_size,
    };
    if (board.buffer != NULL)
    {
        memset(board.buffer, (uint8_t)BSHIP_WATER, board.size * board.size);
    }
    return board;
}

// void BShip_Contest_Run(const char *socket_path, const char *ai_paths[], uint32_t ai_paths_length,
//     uint8_t board_size, uint32_t games_per_match, BShip_ContestAlgorithm algorithm, bool debug)
// {
//     if (socket_path == NULL || ai_paths == NULL)
//     {
//         return;
//     }
//     else if (board_size < BSHIP_BOARD_SIZE_MIN || board_size > BSHIP_BOARD_SIZE_MAX)
//     {
//         return;
//     }
//     else if (games_per_match < BSHIP_GAMES_PER_MATCH_MIN || games_per_match > BSHIP_GAMES_PER_MATCH_MAX)
//     {
//         return;
//     }
// }

