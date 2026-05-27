/**
 * @file game.c
 * @author Matthew Getgen
 * @brief Game state management and logic which is allocation-free and test-friendly.
 * @date 2026-05-23
 */

#define _GNU_SOURCE
#include <math.h>

#include "battleshipslib.h"

uint8_t ShipCountMin_From_BoardSize(uint8_t board_size)
{
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);
    uint8_t ship_count_min = (uint8_t)((2.0f * logf((float)board_size - 4.0f)) + (float)BSHIP_SHIP_COUNT_MIN);
    return ship_count_min;
}

uint8_t ShipCountMax_From_BoardSize(uint8_t board_size)
{
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);
    uint8_t ship_count_max = (uint8_t)((M_PI * logf((float)board_size - 4.0f)) + (float)BSHIP_SHIP_COUNT_MIN);
    return ship_count_max;
}

uint8_t ShipLengthMin_From_BoardSize(uint8_t board_size)
{
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);
    uint8_t ship_length_min = (uint8_t)(
        (M_PI_2 * logf((float)board_size - 4.0f)) + (float)BSHIP_SHIP_COUNT_MIN
    );
    return ship_length_min;
}

uint8_t ShipLengthMax_From_BoardSize(uint8_t board_size)
{
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);
    uint8_t ship_length_max = (uint8_t)(
        (powf((M_PI / M_E), 2.0f) * logf((float)board_size - 4.0f)) + (float)BSHIP_SHIP_COUNT_MIN
    );
    return ship_length_max;
}

BShip_BoardValue BShip_Board_Get(BShip_Board board, uint8_t row, uint8_t column)
{
    assert(board.buffer != NULL);
    assert(board.size >= BSHIP_BOARD_SIZE_MIN);
    assert(board.size <= BSHIP_BOARD_SIZE_MAX);
    assert(row < board.size);
    assert(column < board.size);
    uint32_t index = (row * board.size) + column;
    return (BShip_BoardValue)board.buffer[index];
}

void BShip_Board_Set(BShip_Board board, uint8_t row, uint8_t column, BShip_BoardValue value)
{
    assert(board.buffer != NULL);
    assert(board.size >= BSHIP_BOARD_SIZE_MIN);
    assert(board.size <= BSHIP_BOARD_SIZE_MAX);
    assert(row < board.size);
    assert(column < board.size);
    uint32_t index = (row * board.size) + column;
    board.buffer[index] = value;
}

void ShipLengths_Calculate(BShip_U8Array *array, uint8_t board_size)
{
    assert(array != NULL);
    assert(array->buffer != NULL);
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);
    // uint8_t ship_count_min = ShipCountMin_From_BoardSize(board_size);
    uint8_t ship_count_max = ShipCountMax_From_BoardSize(board_size);
    // uint8_t ship_length_min = ShipLengthMin_From_BoardSize(board_size);
    uint8_t ship_length_max = ShipLengthMax_From_BoardSize(board_size);

    assert(array->capacity >= ship_count_max);

    // TODO: Randomly decide on the ship count.
    uint8_t ship_count = ship_count_max;
    for (array->length = 0; array->length < ship_count; array->length++)
    {
        // TODO: Randomly decide each ship length.
        array->buffer[array->length] = ship_length_max;
    }
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

BShip_Error ValidateAndStoreShips(BShip_Board board, BShip_ShipArray *ships,
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
            if (BShip_Board_Get(board, row, column) != BSHIP_WATER)
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
                BShip_Board_Set(board, row, column, BSHIP_SHIP);
            }
        }
        alive_ships->buffer[alive_ships->length] = i;
        alive_ships->length++;
    }

    return error;
}

BShip_Error ValidateAndStoreShot(BShip_Board opponent_board, BShip_Shot *shot)
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

    BShip_BoardValue current_value = BShip_Board_Get(opponent_board, shot->row, shot->column);
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

    BShip_Board_Set(opponent_board, shot->row, shot->column, shot->value);

    // TODO(mattg): Check for duplicate and error only if that setting is present
    return error;
}

BShip_Ship *FindDeadShip(BShip_Board board, BShip_ShipArray ships,
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
            BShip_BoardValue value = BShip_Board_Get(board, row, column);

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
                BShip_Board_Set(board, row, column, BSHIP_KILL);
            }
            BShip_U8Array_SwapBack(alive_ships, i);
            dead_ships->buffer[dead_ships->length] = index;
            dead_ships->length++;

            return ship;
        }
    }
    return NULL;
}

