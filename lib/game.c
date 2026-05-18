/**
 * @file game.c
 * @author Matthew Getgen
 * @brief Logic for running a game.
 * @date 2026-05-12
 */

#include "platforms/platform.h"
#include "message.c"

const uint8_t BShip_BoardSize_To_ShipLengths[BSHIP_BOARD_SIZE_MAX+1][BSHIP_SHIP_COUNT_MAX+1] = {
    [3] = {2, 1, 1},
    [4] = {3, 2, 1, 1},
    [5] = {3, 3, 2, 2},
    [6] = {3, 3, 2, 2, 2},
    [7] = {4, 3, 3, 2, 2},
    [8] = {4, 4, 3, 3, 3},
    [9] = {5, 4, 4, 3, 3},
    [10] = {5, 4, 4, 3, 3, 3},
};

BShip_GameData BShip_RunGame(BShip_Connection *conn, BShip_AIConnection *ai1_conn, BShip_AIConnection *ai2_conn)
{
    BShip_GameData game_data = {0};

    return game_data;
}
