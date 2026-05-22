/**
 * @file game.c
 * @author Matthew Getgen
 * @brief Logic for running a game.
 * @date 2026-05-12
 */

#define _GNU_SOURCE
#include <math.h>

#include "platforms/platform.h"
#include "message.c"

uint8_t ShipCountMin_From_BoardSize(uint8_t board_size)
{
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);
    uint8_t ship_count_min = (uint8_t)((2.0f * log((double)board_size - 4.0f)) + (double)BSHIP_SHIP_COUNT_MIN);
    return ship_count_min;
}

uint8_t ShipCountMax_From_BoardSize(uint8_t board_size)
{
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);
    uint8_t ship_count_max = (uint8_t)((M_PI * log((double)board_size - 4.0f)) + (double)BSHIP_SHIP_COUNT_MIN);
    return ship_count_max;
}

uint8_t ShipLengthMin_From_BoardSize(uint8_t board_size)
{
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);
    uint8_t ship_length_min = (uint8_t)(
        (M_PI_2 * log((double)board_size - 4.0f)) + (double)BSHIP_SHIP_COUNT_MIN
    );
    return ship_length_min;
}

uint8_t ShipLengthMax_From_BoardSize(uint8_t board_size)
{
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);
    uint8_t ship_length_max = (uint8_t)(
        (pow((M_PI / M_E), 2.0f) * log((double)board_size - 4.0f)) + (double)BSHIP_SHIP_COUNT_MIN
    );
    return ship_length_max;
}

typedef struct {
    uint8_t *buffer;
    uint8_t size;
} Board;

Board Board_Push(BShip_Arena *arena, uint8_t board_size)
{
    assert(arena != NULL);
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);
    Board board = {
        .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint8_t, board_size * board_size),
        .size = board_size,
    };
    if (board.buffer != NULL)
    {
        memset(board.buffer, (uint8_t)BSHIP_WATER, board.size * board.size);
    }
    return board;
}

BShip_BoardValue Board_Get(Board board, uint8_t row, uint8_t column)
{
    assert(board.buffer != NULL);
    assert(board.size >= BSHIP_BOARD_SIZE_MIN);
    assert(board.size <= BSHIP_BOARD_SIZE_MAX);
    assert(row < board.size);
    assert(column < board.size);
    uint32_t index = (row * board.size) + column;
    return (BShip_BoardValue)board.buffer[index];
}

void Board_Set(Board board, uint8_t row, uint8_t column, BShip_BoardValue value)
{
    assert(board.buffer != NULL);
    assert(board.size >= BSHIP_BOARD_SIZE_MIN);
    assert(board.size <= BSHIP_BOARD_SIZE_MAX);
    assert(row < board.size);
    assert(column < board.size);
    uint32_t index = (row * board.size) + column;
    board.buffer[index] = value;
}

BShip_U8Array BShip_U8Array_ShipLengths_Create(BShip_Arena *arena, uint8_t board_size)
{
    assert(arena != NULL);
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);
    // uint8_t ship_count_min = ShipCountMin_From_BoardSize(board_size);
    uint8_t ship_count_max = ShipCountMax_From_BoardSize(board_size);
    // uint8_t ship_length_min = ShipLengthMin_From_BoardSize(board_size);
    uint8_t ship_length_max = ShipLengthMax_From_BoardSize(board_size);

    // TODO: Randomly decide on the ship count.
    uint8_t ship_count = ship_count_max;
    BShip_U8Array array = {
        .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint8_t, ship_count),
        .length = 0,
        .capacity = ship_count,
    };
    if (array.buffer == NULL)
    {
        return array;
    }

    for (array.length = 0; array.length < array.capacity; array.length++) {
        // TODO: Randomly decide each ship length.
        array.buffer[array.length] = ship_length_max;
    }
    return array;
}

void BShip_U8Array_SwapBack(BShip_U8Array *array, uint8_t index)
{
    assert(array != NULL);
    assert(array->buffer != NULL);
    assert(array->length <= array->capacity);
    if (array->length == 0)
    {
        return;
    }
    assert(index < array->length);

    array->length--;
    array->buffer[index] = array->buffer[array->length];
    array->buffer[array->length] = 0;
}

BShip_Error ValidateAndStoreShips(Board board, BShip_ShipArray *ships,
    BShip_U8Array *alive_ships, BShip_U8Array *ship_lengths)
{
    assert(board.buffer != NULL);
    assert(board.size >= BSHIP_BOARD_SIZE_MIN);
    assert(board.size <= BSHIP_BOARD_SIZE_MAX);
    assert(ships != NULL);
    assert(ships->buffer != NULL);
    assert(alive_ships != NULL);
    assert(alive_ships->buffer != NULL);
    assert(ships->capacity == alive_ships->capacity);
    assert(ship_lengths != NULL);
    assert(ship_lengths->buffer != NULL);
    // NOTE(mattg): this check should have already been handled by the messages.
    assert(ships->length == ship_lengths->length);
    BShip_Error error = {
        .type = ERROR_SUCCESS,
    };

    for (uint8_t i = 0; i < ships->length; i++)
    {
        BShip_Ship ship = ships->buffer[i];
        // We are going to support returning a ship array in any 
        bool valid_length = false;
        for (uint8_t j = 0; j < ship_lengths->length; j++)
        {
            if (ship.length == ship_lengths->buffer[j])
            {
                valid_length = true;
                BShip_U8Array_SwapBack(ship_lengths, j);
                break;
            }
        }
        if (!valid_length)
        {
            error.type = ERROR_SHIP_LENGTH_INVALID;
            error.ship = ship;
            PRINT_ERROR_F("Ship returned with length: %d\n", ship.length);
            fprintf(stderr, "\tbut expected one of: [ ");
            for (uint8_t j = 0; j < ship_lengths->length; j++)
            {
                if (j == 0)
                {
                    fprintf(stderr, "%d", ship_lengths->buffer[j]);
                }
                else
                {
                    fprintf(stderr, ", %d", ship_lengths->buffer[j]);
                }
            }
            fprintf(stderr, " ]\n");
            break;
        }
        
        uint8_t front = 0, end = board.size, check = 0;
        switch (ship.direction)
        {
        case BSHIP_HORIZONTAL:
            front = ship.column;
            end = ship.column + (ship.length - 1);
            check = ship.row;
            break;
        case BSHIP_VERTICAL:
            front = ship.row;
            end = ship.row + (ship.length - 1);
            check = ship.column;
            break;
        }
        if (front >= board.size || end >= board.size || check >= board.size)
        {
            error.type = ERROR_SHIP_OFF_BOARD;
            error.ship = ship;
            PRINT_ERROR_F("Ship returned doesn't fit on a %dx%d board\n", board.size, board.size);
            fprintf(stderr, "\trow: %d\n\tcolumn: %d\n\tlength: %d\n\tdirection: %s\n",
                ship.row, ship.column, ship.length,
                ship.direction == BSHIP_HORIZONTAL ? "HORIZONTAL" : "VERTICAL");
            break;
        }
        uint8_t row_multiplier = ship.direction == BSHIP_HORIZONTAL;
        uint8_t column_multiplier = ship.direction == BSHIP_VERTICAL;
        for (uint8_t i = 0; i < ship.length; i++)
        {
            uint8_t row = ship.row + (i * row_multiplier);
            uint8_t column = ship.column + (i * column_multiplier);
            if (Board_Get(board, row, column) != BSHIP_WATER)
            {
                error.type = ERROR_SHIP_OVERLAP;
                error.ship = ship;
                PRINT_ERROR("Ship returned overlaps with another ship already on the board\n");
                fprintf(stderr, "\trow: %d\n\tcolumn: %d\n\tlength: %d\n\tdirection: %s\n",
                    ship.row, ship.column, ship.length,
                    ship.direction == BSHIP_HORIZONTAL ? "HORIZONTAL" : "VERTICAL");
                break;
            }
            else
            {
                Board_Set(board, row, column, BSHIP_SHIP);
            }
        }
        alive_ships->buffer[alive_ships->length] = i;
        alive_ships->length++;
    }

    return error;
}

BShip_Error ValidateAndStoreShot(Board opponent_board, BShip_Shot *shot)
{
    assert(opponent_board.buffer != NULL);
    assert(opponent_board.size >= BSHIP_BOARD_SIZE_MIN);
    assert(opponent_board.size <= BSHIP_BOARD_SIZE_MAX);
    assert(shot != NULL);

    BShip_Error error = {
        .type = ERROR_SUCCESS,
    };

    if (shot->row >= opponent_board.size || shot->column >= opponent_board.size)
    {
        error.type = ERROR_SHOT_OFF_BOARD;
        error.shot = *shot;
        PRINT_ERROR_F("Shot returned doesn't fit on a %dx%d board\n", opponent_board.size, opponent_board.size);
        fprintf(stderr, "\trow: %d\n\tcolumn: %d\n", shot->row, shot->column);
        return error;
    }

    BShip_BoardValue current_value = Board_Get(opponent_board, shot->row, shot->column);
    switch (current_value)
    {
    case BSHIP_WATER:
        shot->value = BSHIP_MISS;
        break;
    case BSHIP_SHIP:
        shot->value = BSHIP_HIT;
        break;
    case BSHIP_HIT:
    case BSHIP_DUPLICATE_HIT:
        shot->value = BSHIP_DUPLICATE_HIT;
        break;
    case BSHIP_MISS:
    case BSHIP_DUPLICATE_MISS:
        shot->value = BSHIP_DUPLICATE_MISS;
        break;
    case BSHIP_KILL:
    case BSHIP_DUPLICATE_KILL:
        shot->value = BSHIP_DUPLICATE_KILL;
        break;
    }

    Board_Set(opponent_board, shot->row, shot->column, shot->value);

    // TODO(mattg): Check for duplicate and error only if that setting is present
    return error;
}

BShip_Ship *FindDeadShip(Board board, BShip_ShipArray ships,
    BShip_U8Array *alive_ships, BShip_U8Array *dead_ships)
{
    assert(board.buffer != NULL);
    assert(ships.buffer != NULL);
    assert(alive_ships != NULL);
    assert(alive_ships->buffer != NULL);
    assert(dead_ships != NULL);
    assert(dead_ships->buffer != NULL);

    for (uint8_t i = 0; i < alive_ships->length; i++)
    {
        uint8_t index = alive_ships->buffer[i];
        assert(index < ships.length);
        BShip_Ship *ship = &ships.buffer[index];

        uint8_t hit_count = 0;
        uint8_t row_multiplier = ship->direction == BSHIP_HORIZONTAL;
        uint8_t column_multiplier = ship->direction == BSHIP_VERTICAL;
        for (uint8_t j = 0; j < ship->length; j++)
        {
            uint8_t row = ship->row + (j * row_multiplier);
            uint8_t column = ship->column + (j * column_multiplier);
            BShip_BoardValue value = Board_Get(board, row, column);

            if (value == BSHIP_SHIP)
            {
                break;
            }
            else if (value == BSHIP_HIT || value == BSHIP_DUPLICATE_HIT)
            {
                hit_count++;
            }
            else
            {
                // NOTE(mattg): If this condition is ever met, it means where a ship is supposed to be
                // something else is, which is supposed to never happen.
                assert(false);
            }
        }
        if (hit_count == ship->length)
        {
            for(uint8_t j = 0; j < ship->length; j++)
            {
                uint8_t row = ship->row + (j * row_multiplier);
                uint8_t column = ship->column + (j * column_multiplier);
                Board_Set(board, row, column, BSHIP_KILL);
            }
            BShip_U8Array_SwapBack(alive_ships, i);
            dead_ships->buffer[dead_ships->length] = index;
            dead_ships->length++;

            return ship;
        }
    }
    return NULL;
}

BShip_GameData BShip_RunGame(BShip_Arena *arena, BShip_Connection *conn,
    BShip_AIConnection *ai1_conn, BShip_AIConnection *ai2_conn, uint8_t board_size)
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

    Board ai1_board = Board_Push(arena, board_size);
    Board ai2_board = Board_Push(arena, board_size);
    if (ai1_board.buffer == NULL || ai2_board.buffer == NULL)
    {
        goto on_game_end;
    }

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
        goto on_game_end;
    }

    {
        BShip_ArenaMark ship_mark = BShip_ArenaMark_Get(arena);

        BShip_U8Array ship_lengths = BShip_U8Array_ShipLengths_Create(arena, board_size);
        if (ship_lengths.buffer == NULL)
        {
            goto on_game_end;
        }
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
        if (ship_lengths.buffer == NULL)
        {
            goto on_game_end;
        }

        BShip_Message_PlaceShips_Create(&ai1_message, ship_lengths.buffer, ship_lengths.length);
        game.ai1.error.type = BShip_AIConnection_Send(ai1_conn, ai1_message);
        game.ai2.error.type = BShip_AIConnection_Send(ai2_conn, ai1_message);
        if (game.ai1.error.type != ERROR_SUCCESS || game.ai2.error.type != ERROR_SUCCESS)
        {
            goto on_game_end;
        }

        game.ai1.error.type = BShip_AIConnection_Receive(ai1_conn, &ai1_message);
        game.ai2.error.type = BShip_AIConnection_Receive(ai2_conn, &ai2_message);
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

        game.ai1.error.type = BShip_AIConnection_Receive(ai1_conn, &ai1_message);
        game.ai2.error.type = BShip_AIConnection_Receive(ai2_conn, &ai2_message);
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

        game.ai1.error.type = BShip_AIConnection_Send(ai1_conn, ai1_message);
        game.ai2.error.type = BShip_AIConnection_Send(ai2_conn, ai1_message);
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

