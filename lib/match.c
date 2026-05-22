/**
 * @file match.c
 * @author Matthew Getgen
 * @brief Logic for running a game.
 * @date 2026-05-12
 */

#include "game.c"

BShip_MatchData BShip_RunMatch(BShip_Arena *arena, const char *socket_path,
    const char *ai1_path, const char *ai2_path, uint8_t board_size, uint32_t games_per_match, bool debug)
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
    else if (games_per_match < BSHIP_GAMES_PER_MATCH_MIN || games_per_match > BSHIP_GAMES_PER_MATCH_MAX)
    {
        return match;
    }
    match.games_per_match = games_per_match;
    match.board_size = board_size;

    BShip_Connection *conn = BShip_Connection_Allocate(arena);
    if (conn == NULL)
    {
        return match;
    }

    if (BShip_Connection_Create(conn, socket_path, debug) == -1)
    {
        goto on_conn_create_error;
    }

    BShip_AIConnection *ai1_conn = BShip_AIConnection_Allocate(arena);
    BShip_AIConnection *ai2_conn = BShip_AIConnection_Allocate(arena);
    if (ai1_conn == NULL || ai2_conn == NULL)
    {
        goto on_conn_create_error;
    }

    match.ai1.error.type = BShip_AIConnection_StartProcess(ai1_conn, ai1_path, socket_path);
    match.ai2.error.type = BShip_AIConnection_StartProcess(ai2_conn, ai2_path, socket_path);
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
    match.ai1.name = BSHIP_ARENA_PUSH_ARRAY(arena, char, MESSAGE_NAME_SIZE_MAX+1);
    match.ai1.authors = BSHIP_ARENA_PUSH_ARRAY(arena, char, MESSAGE_NAME_SIZE_MAX+1);
    match.ai2.name = BSHIP_ARENA_PUSH_ARRAY(arena, char, MESSAGE_NAME_SIZE_MAX+1);
    match.ai2.authors = BSHIP_ARENA_PUSH_ARRAY(arena, char, MESSAGE_NAME_SIZE_MAX+1);
    if (match.ai1.name == NULL || match.ai1.authors == NULL || match.ai2.name == NULL || match.ai2.authors == NULL)
    {
        goto on_conn_accept_error;
    }
    
    {
        BSHIP_ARENA_TEMP_BEGIN(arena);
        BShip_Message ai1_message = {
        .json = BSHIP_ARENA_PUSH_ARRAY(arena, char, MESSAGE_SIZE_MAX+1),
        .length = 0,
        .capacity = MESSAGE_SIZE_MAX,
        };
        BShip_Message ai2_message = {
            .json = BSHIP_ARENA_PUSH_ARRAY(arena, char, MESSAGE_SIZE_MAX+1),
            .length = 0,
            .capacity = MESSAGE_SIZE_MAX,
        };
        if (ai1_message.json == NULL || ai2_message.json == NULL)
        {
            goto on_conn_accept_error;
        }

        match.ai1.error.type = BShip_AIConnection_Receive(ai1_conn, &ai1_message);
        match.ai2.error.type = BShip_AIConnection_Receive(ai2_conn, &ai2_message);
        if (match.ai1.error.type != ERROR_SUCCESS || match.ai2.error.type != ERROR_SUCCESS)
        {
            goto on_conn_accept_error;
        }

        match.ai1.error.type = BShip_Message_Hello_Parse(ai1_message, match.ai1.name, match.ai1.authors);
        match.ai2.error.type = BShip_Message_Hello_Parse(ai2_message, match.ai2.name, match.ai2.authors);
        if (match.ai1.error.type != ERROR_SUCCESS || match.ai2.error.type != ERROR_SUCCESS)
        {
            goto on_conn_accept_error;
        }

        BShip_Message_SetupMatch_Create(&ai1_message, board_size, BSHIP_PLAYER_1);
        BShip_Message_SetupMatch_Create(&ai2_message, board_size, BSHIP_PLAYER_2);

        match.ai1.error.type = BShip_AIConnection_Send(ai1_conn, ai1_message);
        match.ai2.error.type = BShip_AIConnection_Send(ai2_conn, ai2_message);
        if (match.ai1.error.type != ERROR_SUCCESS || match.ai2.error.type != ERROR_SUCCESS)
        {
            goto on_conn_accept_error;
        }
        BSHIP_ARENA_TEMP_END(arena);
    }

    match.games.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_GameData, games_per_match);
    match.games.capacity = games_per_match;
    if (match.games.buffer == NULL)
    {
        goto on_match_over;
    }
    for (match.games.length = 0; match.games.length < match.games.capacity; match.games.length++)
    {
        BShip_GameData game = BShip_RunGame(arena, conn, ai1_conn, ai2_conn, board_size);
        match.games.buffer[match.games.length] = game;
        // TODO(mattg): merge game and match data.
        if (game.ai1.error.type != ERROR_SUCCESS || game.ai2.error.type != ERROR_SUCCESS)
        {
            break;
        }
    }

on_match_over:
    {
        BSHIP_ARENA_TEMP_BEGIN(arena);
        BShip_Message message = {
        .json = BSHIP_ARENA_PUSH_ARRAY(arena, char, MESSAGE_SIZE_MAX+1),
        .length = 0,
        .capacity = MESSAGE_SIZE_MAX,
        };
        if (message.json == NULL)
        {
            goto on_conn_accept_error;
        }

        BShip_Message_MatchOver_Create(&message);

        if (match.ai1.error.type == ERROR_SUCCESS)
        {
            BShip_AIConnection_Send(ai1_conn, message);
        }
        if (match.ai2.error.type == ERROR_SUCCESS)
        {
            BShip_AIConnection_Send(ai2_conn, message);
        }
        BSHIP_ARENA_TEMP_END(arena);
    }

    // NOTE(mattg): We want this to happen at the end anyway, so don't early exit.
on_conn_accept_error:
    BShip_AIConnection_Close(ai1_conn);
    BShip_AIConnection_Close(ai2_conn);
on_process_error:
    // TODO(mattg): hook this up with the error handling (status code, exited vs hung)
    if (!BShip_AIConnection_WaitProcess(ai1_conn))
    {
        BShip_AIConnection_KillProcess(ai1_conn);
    }
    if (!BShip_AIConnection_WaitProcess(ai2_conn))
    {
        BShip_AIConnection_KillProcess(ai2_conn);
    }
on_conn_create_error:
    BShip_Connection_Close(conn);
    return match;
}

