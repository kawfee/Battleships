/**
 * @file game.c
 * @author Matthew Getgen
 * @brief Logic for running a game.
 * @date 2026-05-12
 */

#include "platforms/platform.h"
#include "message.c"

BShip_GameData BShip_RunGame(BShip_Connection *conn, BShip_AIConnection *ai1_conn, BShip_AIConnection *ai2_conn, uint8_t board_size)
{
    assert(conn != NULL);
    assert(ai1_conn != NULL);
    assert(ai2_conn != NULL);
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);
    BShip_GameData game = {0};

    // uint8_t ship_lengths[] = BShip_BoardSize_To_Ship_Lengths[board_size];
    //
    // BShip_Message place_ships = BShip_Message_PlaceShip_Create(ship_lengths);
    // game.ai1.error.type = BShip_AIConnection_Send(ai1_conn, place_ships);
    // game.ai2.error.type = BShip_AIConnection_Send(ai2_conn, place_ships);
    // if (game.ai1.error.type != ERROR_SUCCESS || game.ai2.error.type != ERROR_SUCCESS)
    // {
    //     return game;
    // }
    //
    // BShip_Message ai1_recv_buffer = {
    //     .json = 
    // };

    return game;
}
