/**
 * @file match.c
 * @author Matthew Getgen
 * @brief Logic for running a game.
 * @date 2026-05-12
 */

#include "game.c"

BShip_MatchData BShip_RunMatch(const char *socket_path, const char *ai1_path, const char *ai2_path,
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
    else if (games_per_match < BSHIP_GAMES_PER_MATCH_MIN || games_per_match > BSHIP_GAMES_PER_MATCH_MAX)
    {
        return match;
    }
    match.games_per_match = games_per_match;
    match.board_size = board_size;

    BShip_Connection *conn = BShip_Connection_Allocate();
    if (conn == NULL)
    {
        return match;
    }

    if (BShip_Connection_Create(conn, socket_path, debug) == -1)
    {
        goto on_conn_error;
    }

    BShip_AIConnection *ai1_conn = BShip_AIConnection_Allocate();
    BShip_AIConnection *ai2_conn = BShip_AIConnection_Allocate();
    if (ai1_conn == NULL || ai2_conn == NULL)
    {
        return match;
    }

    BShip_ErrorType ai1_err = BShip_AIConnection_StartProcess(ai1_conn, ai1_path, socket_path);
    if (ai1_err != ERROR_SUCCESS)
    {
        goto on_process_error;
    }
    BShip_ErrorType ai2_err = BShip_AIConnection_StartProcess(ai2_conn, ai2_path, socket_path);
    if (ai2_err != ERROR_SUCCESS)
    {
        goto on_process_error;
    }

    ai1_err = BShip_AIConnection_Accept(ai1_conn, conn, debug);
    if (ai1_err != ERROR_SUCCESS)
    {
        goto on_ai_conn_error;
    }
    ai2_err = BShip_AIConnection_Accept(ai2_conn, conn, debug);
    if (ai2_err != ERROR_SUCCESS)
    {
        goto on_ai_conn_error;
    }

    


    // NOTE(mattg): We want this to happen at the end anyway, so let's just leave it.
on_ai_conn_error:
    BShip_AIConnection_Close(ai1_conn);
    BShip_AIConnection_Close(ai2_conn);
on_process_error:
    BShip_AIConnection_KillProcess(ai1_conn);
    BShip_AIConnection_KillProcess(ai2_conn);
    BShip_AIConnection_Deallocate(ai1_conn);
    BShip_AIConnection_Deallocate(ai2_conn);
on_conn_error:
    BShip_Connection_Close(conn);
    BShip_Connection_Deallocate(conn);
    return match;
}
