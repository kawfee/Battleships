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

size_t BShip_Game_CalculateMemorySize(uint8_t board_size)
{
    size_t ship_count_max = (size_t)ShipCountMax_From_BoardSize(board_size);
    size_t shot_count_max = board_size * board_size;
    size_t player_size = (sizeof(BShip_Ship) * ship_count_max) + (sizeof(uint8_t) * ship_count_max * 2)
        + (sizeof(BShip_Shot) * shot_count_max);
    return player_size * 2;
}

BShip_GameData BShip_Game_Run(BShip_Arena *arena, BShip_Connection *conn,
    BShip_AIConnection *ai1_conn, BShip_AIConnection *ai2_conn, uint8_t board_size, bool debug)
{
    assert(arena != NULL);
    assert(conn != NULL);
    assert(ai1_conn != NULL);
    assert(ai2_conn != NULL);
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);

    uint8_t ship_count_max = ShipCountMax_From_BoardSize(board_size);
    uint32_t shot_count_max = board_size * board_size;

    BShip_GameData game = {
        .ai1 = {
            .ships = {
                .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_Ship, ship_count_max),
                .length = 0,
                .capacity = ship_count_max,
            },
            .alive_ships = {
                .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint8_t, ship_count_max),
                .length = 0,
                .capacity = ship_count_max,
            },
            .dead_ships = {
                .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint8_t, ship_count_max),
                .length = 0,
                .capacity = ship_count_max,
            },
            .shots = {
                .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_Shot, shot_count_max),
                .length = 0,
                .capacity = shot_count_max,
            },
        },
        .ai2 = {
            .ships = {
                .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_Ship, ship_count_max),
                .length = 0,
                .capacity = ship_count_max,
            },
            .alive_ships = {
                .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint8_t, ship_count_max),
                .length = 0,
                .capacity = ship_count_max,
            },
            .dead_ships = {
                .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint8_t, ship_count_max),
                .length = 0,
                .capacity = ship_count_max,
            },
            .shots = {
                .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_Shot, shot_count_max),
                .length = 0,
                .capacity = shot_count_max,
            },
        },
    };
    if (game.ai1.ships.buffer == NULL || game.ai2.ships.buffer == NULL ||
        game.ai1.alive_ships.buffer == NULL || game.ai2.alive_ships.buffer == NULL ||
        game.ai1.dead_ships.buffer == NULL || game.ai2.dead_ships.buffer == NULL ||
        game.ai1.shots.buffer == NULL || game.ai2.shots.buffer == NULL)
    {
        return game;
    }

    BShip_ArenaMark game_mark = BShip_ArenaMark_Get(arena);

    BShip_Board ai1_board = BShip_Board_Allocate(arena, board_size);
    BShip_Board ai2_board = BShip_Board_Allocate(arena, board_size);
    if (ai1_board.buffer == NULL || ai2_board.buffer == NULL)
    {
        goto on_game_end;
    }

    BShip_Message ai1_message = {
        .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_SIZE),
    };
    BShip_Message ai2_message = {
        .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_SIZE),
    };
    if (ai1_message.buffer == NULL || ai2_message.buffer == NULL)
    {
        goto on_game_end;
    }

    {
        BShip_ArenaMark ship_mark = BShip_ArenaMark_Get(arena);

        BShip_U8Array ship_lengths = {
            .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint8_t, ship_count_max),
            .length = 0,
            .capacity = ship_count_max,
        };
        if (ship_lengths.buffer == NULL)
        {
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
            goto on_game_end;
        }
        memcpy(ship_lengths_copy.buffer, ship_lengths.buffer, ship_lengths.capacity * sizeof(uint8_t));

        BShip_Message_PlaceShips_Create(&ai1_message, ship_lengths.buffer, ship_lengths.length);
        game.ai1.error.type = BShip_AIConnection_Send(ai1_conn, ai1_message, debug);
        game.ai2.error.type = BShip_AIConnection_Send(ai2_conn, ai1_message, debug);
        if (game.ai1.error.type != ERROR_SUCCESS || game.ai2.error.type != ERROR_SUCCESS)
        {
            goto on_game_end;
        }

        game.ai1.error.type = BShip_AIConnection_Receive(ai1_conn, &ai1_message, debug);
        game.ai2.error.type = BShip_AIConnection_Receive(ai2_conn, &ai2_message, debug);
        if (game.ai1.error.type != ERROR_SUCCESS || game.ai2.error.type != ERROR_SUCCESS)
        {
            goto on_game_end;
        }

        game.ai1.error.type = BShip_Message_ShipsPlaced_Parse(ai1_message, &game.ai1.ships, ship_lengths.length);
        game.ai2.error.type = BShip_Message_ShipsPlaced_Parse(ai2_message, &game.ai2.ships, ship_lengths.length);
        if (game.ai1.error.type != ERROR_SUCCESS || game.ai2.error.type != ERROR_SUCCESS)
        {
            goto on_game_end;
        }

        game.ai1.error = ValidateAndStoreShips(ai1_board, &game.ai1.ships, &game.ai1.alive_ships, &ship_lengths);
        game.ai2.error = ValidateAndStoreShips(ai2_board, &game.ai2.ships, &game.ai2.alive_ships, &ship_lengths_copy);
        if (game.ai1.error.type != ERROR_SUCCESS || game.ai2.error.type != ERROR_SUCCESS)
        {
            goto on_game_end;
        }

        BShip_Arena_Rollback(arena, ship_mark);
    }

    bool next_shot = true;

    for (uint32_t i = 0; i < shot_count_max; i++)
    {
        if (i == (shot_count_max - 1))
        {
            next_shot = false;
        }

        game.ai1.error.type = BShip_AIConnection_Receive(ai1_conn, &ai1_message, debug);
        game.ai2.error.type = BShip_AIConnection_Receive(ai2_conn, &ai2_message, debug);
        if (game.ai1.error.type != ERROR_SUCCESS || game.ai2.error.type != ERROR_SUCCESS)
        {
            goto on_game_end;
        }

        game.ai1.error.type = BShip_Message_ShotTaken_Parse(ai1_message, &game.ai1.shots.buffer[i]);
        game.ai2.error.type = BShip_Message_ShotTaken_Parse(ai2_message, &game.ai2.shots.buffer[i]);
        game.ai1.shots.length++;
        game.ai2.shots.length++;
        if (game.ai1.error.type != ERROR_SUCCESS || game.ai2.error.type != ERROR_SUCCESS)
        {
            goto on_game_end;
        }

        game.ai1.error = ValidateAndStoreShot(ai2_board, &game.ai1.shots.buffer[i]);
        game.ai2.error = ValidateAndStoreShot(ai1_board, &game.ai2.shots.buffer[i]);
        if (game.ai1.error.type != ERROR_SUCCESS || game.ai2.error.type != ERROR_SUCCESS)
        {
            goto on_game_end;
        }

        BShip_Ship *ai1_dead_ship = NULL, *ai2_dead_ship = NULL;
        if (game.ai2.shots.buffer[i].value == BSHIP_HIT)
        {
            ai1_dead_ship = FindDeadShip(ai1_board, game.ai1.ships, &game.ai1.alive_ships, &game.ai1.dead_ships);
        }
        if (game.ai1.shots.buffer[i].value == BSHIP_HIT)
        {
            ai2_dead_ship = FindDeadShip(ai2_board, game.ai2.ships, &game.ai2.alive_ships, &game.ai2.dead_ships);
        }
        if (game.ai1.alive_ships.length == 0 || game.ai2.alive_ships.length == 0)
        {
            next_shot = false;
        }

        BShip_Message_ShotResult_Create(&ai1_message, game.ai1.shots.buffer[i], game.ai2.shots.buffer[i],
            ai1_dead_ship, ai2_dead_ship, next_shot);

        game.ai1.error.type = BShip_AIConnection_Send(ai1_conn, ai1_message, debug);
        game.ai2.error.type = BShip_AIConnection_Send(ai2_conn, ai1_message, debug);
        if (game.ai1.error.type != ERROR_SUCCESS || game.ai2.error.type != ERROR_SUCCESS)
        {
            goto on_game_end;
        }

        if (!next_shot)
        {
            break;
        }
    }

on_game_end:
    BShip_Arena_Rollback(arena, game_mark);
    return game;
}

size_t BShip_Match_CalculateMemorySize(uint8_t board_size, uint32_t games_per_match)
{
    size_t game_size = BShip_Game_CalculateMemorySize(board_size) + sizeof(BShip_GameData);
    size_t ai_size = (BSHIP_MESSAGE_NAME_SIZE_MAX * 4) + (BSHIP_MESSAGE_SIZE * 2);
    return (game_size * games_per_match) + ai_size + (board_size * board_size * 2)
        + BShip_Connection_GetSize() + (BShip_AIConnection_GetSize() * 2);
}

BShip_MatchData BShip_Match_Run(BShip_Arena *arena, char *socket_path,
    char *ai1_path, char *ai1_dir, char *ai2_path, char *ai2_dir,
    uint8_t board_size, uint32_t games_per_match, bool debug)
{
    BShip_MatchData match = {0};
    if (socket_path == NULL || ai1_path == NULL || ai2_path == NULL)
    {
        return match;
    }
    else if (board_size < BSHIP_BOARD_SIZE_MIN || board_size > BSHIP_BOARD_SIZE_MAX)
    {
        return match;
    }
    else if (games_per_match < BShip_GamesPerMatchMin_From_BoardSize(board_size) ||
             games_per_match > BShip_GamesPerMatchMax_From_BoardSize(board_size))
    {
        return match;
    }
    else if (!BShip_PathIsExecutable(ai1_path) || !BShip_PathIsDirectory(ai1_dir) ||
        !BShip_PathIsExecutable(ai2_path) || !BShip_PathIsDirectory(ai2_dir))
    {
        return match;
    }
    match.games_per_match = games_per_match;
    match.board_size = board_size;

    match.ai1.error.message.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_SIZE);
    match.ai2.error.message.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_SIZE);
    if (match.ai1.error.message.buffer == NULL || match.ai2.error.message.buffer == NULL)
    {
        return match;
    }
    match.ai1.name = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_NAME_SIZE_MAX);
    match.ai1.authors = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_NAME_SIZE_MAX);
    if (match.ai1.name == NULL || match.ai1.authors == NULL)
    {
        return match;
    }
    match.ai2.name = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_NAME_SIZE_MAX);
    match.ai2.authors = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_NAME_SIZE_MAX);
    if (match.ai2.name == NULL || match.ai2.authors == NULL)
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

    match.ai1.error.type = BShip_AIConnection_StartProcess(ai1_conn, socket_path, ai1_path, ai1_dir);
    match.ai2.error.type = BShip_AIConnection_StartProcess(ai2_conn, socket_path, ai2_path, ai2_dir);
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

    match.ai1.error.type = BShip_AIConnection_Receive(ai1_conn, &match.ai1.error.message, debug);
    match.ai2.error.type = BShip_AIConnection_Receive(ai2_conn, &match.ai2.error.message, debug);
    if (match.ai1.error.type != ERROR_SUCCESS || match.ai2.error.type != ERROR_SUCCESS)
    {
        goto on_conn_accept_error;
    }

    match.ai1.error.type = BShip_Message_Hello_Parse(match.ai1.error.message, match.ai1.name, match.ai1.authors);
    match.ai2.error.type = BShip_Message_Hello_Parse(match.ai2.error.message, match.ai2.name, match.ai2.authors);
    if (match.ai1.error.type != ERROR_SUCCESS || match.ai2.error.type != ERROR_SUCCESS)
    {
        goto on_conn_accept_error;
    }

    BShip_Message_SetupMatch_Create(&match.ai1.error.message, board_size, BSHIP_PLAYER_1);
    BShip_Message_SetupMatch_Create(&match.ai2.error.message, board_size, BSHIP_PLAYER_2);

    match.ai1.error.type = BShip_AIConnection_Send(ai1_conn, match.ai1.error.message, debug);
    match.ai2.error.type = BShip_AIConnection_Send(ai2_conn, match.ai1.error.message, debug);
    if (match.ai1.error.type != ERROR_SUCCESS || match.ai2.error.type != ERROR_SUCCESS)
    {
        goto on_conn_accept_error;
    }

    match.games.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_GameData, games_per_match);
    match.games.capacity = games_per_match;
    if (match.games.buffer == NULL)
    {
        goto on_match_over;
    }
    for (match.games.length = 0; match.games.length < match.games.capacity; match.games.length++)
    {
        BShip_GameData game = BShip_Game_Run(arena, conn, ai1_conn, ai2_conn, board_size, debug);
        match.games.buffer[match.games.length] = game;
        // TODO(mattg): merge game and match data.
        if (game.ai1.error.type != ERROR_SUCCESS || game.ai2.error.type != ERROR_SUCCESS)
        {
            break;
        }
    }

    // NOTE(mattg): We want this to happen at the end anyway, so don't early exit.
on_match_over:
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

