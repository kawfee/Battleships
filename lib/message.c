/**
 * @file message.c
 * @author Matthew Getgen
 * @brief Battleships message logic
 * @date 2026-05-15
 */

#include <string.h>

#include "vendor/yyjson/src/yyjson.h"
#include "battleshipslib.h"

#define MESSAGE_TYPE_KEY "mt"
#define PLAYER_NUM_KEY   "pn"
#define AI_NAME_KEY      "ai"
#define AUTHOR_NAMES_KEY "au"
#define BOARD_SIZE_KEY   "bs"
#define LENGTH_KEY       "l"
#define ROW_KEY          "r"
#define COLUMN_KEY       "c"
#define SHIP_KEY         "sp"
#define SHOT_KEY         "st"
#define NEXT_SHOT_KEY    "ns"

typedef enum {
    MESSAGE_HELLO,
    MESSAGE_SETUP_MATCH,
    MESSAGE_PLACE_SHIPS,
    MESSAGE_SHIPS_PLACED,
    MESSAGE_SHOT_TAKEN,
    MESSAGE_SHOT_RESULT,
    MESSAGE_MATCH_OVER,
} BShip_MessageType;

BShip_Error BShip_Message_Hello_Parse(BShip_Message message, char *ai_name, char *author_names)
{
    assert(message.buffer != NULL);
    assert(ai_name != NULL);
    assert(author_names != NULL);

    BShip_Error error = {
        .type = ERROR_SUCCESS,
    };

    yyjson_doc *doc = yyjson_read(message.buffer, strlen(message.buffer), 0);
    if (doc == NULL) goto on_error;

    yyjson_val *root = yyjson_doc_get_root(doc);
    {
        yyjson_val *obj = yyjson_obj_get(root, MESSAGE_TYPE_KEY);
        if (!yyjson_is_uint(obj)) goto on_error;
        uint64_t type = yyjson_get_uint(obj);
        if (type != MESSAGE_HELLO) goto on_error;
    }
    {
        yyjson_val *obj = yyjson_obj_get(root, AI_NAME_KEY);
        if (!yyjson_is_str(obj)) goto on_error;
        size_t ai_name_len = yyjson_get_len(obj);
        if (ai_name_len > BSHIP_MESSAGE_NAME_SIZE_MAX)
        {
            ai_name_len = BSHIP_MESSAGE_NAME_SIZE_MAX;
        }
        const char *ai_name_input = yyjson_get_str(obj);
        strncpy(ai_name, ai_name_input, ai_name_len);
    }
    {
        yyjson_val *obj = yyjson_obj_get(root, AUTHOR_NAMES_KEY);
        if (!yyjson_is_str(obj)) goto on_error;
        size_t author_names_len = yyjson_get_len(obj);
        if (author_names_len > BSHIP_MESSAGE_NAME_SIZE_MAX)
        {
            author_names_len = BSHIP_MESSAGE_NAME_SIZE_MAX;
        }
        const char *author_names_input = yyjson_get_str(obj);
        strncpy(author_names, author_names_input, author_names_len);
    }

    yyjson_doc_free(doc);
    return error;
on_error:
    PRINT_ERROR_F("Invalid \"Hello\" message received: <%s>", message.buffer);
    yyjson_doc_free(doc);
    error.type = ERROR_MESSAGE_HELLO_INVALID;
    error.value.message = message;
    return error;
}

void BShip_Message_SetupMatch_Create(BShip_Message *message, uint8_t board_size, BShip_PlayerNum player_num)
{
    assert(message != NULL);
    assert(message->buffer != NULL);
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);
    memset(message->buffer, 0, BSHIP_MESSAGE_SIZE);

    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    yyjson_mut_obj_add_int(doc, root, MESSAGE_TYPE_KEY, MESSAGE_SETUP_MATCH);
    yyjson_mut_obj_add_int(doc, root, BOARD_SIZE_KEY, board_size);
    yyjson_mut_obj_add_int(doc, root, PLAYER_NUM_KEY, player_num);

    size_t length = 0;
    char *json = yyjson_mut_write(doc, 0, &length);
    assert(length < BSHIP_MESSAGE_SIZE);

    strncpy(message->buffer, json, length);

    free(json);
    yyjson_mut_doc_free(doc);
}

void BShip_Message_PlaceShips_Create(BShip_Message *message, uint8_t *ship_lengths, uint8_t ship_count)
{
    assert(message != NULL);
    assert(message->buffer != NULL);
    assert(ship_lengths != NULL);
    assert(ship_count >= BSHIP_SHIP_COUNT_MIN);
    assert(ship_count <= BSHIP_SHIP_COUNT_MAX);
    memset(message->buffer, 0, BSHIP_MESSAGE_SIZE);

    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    yyjson_mut_obj_add_int(doc, root, MESSAGE_TYPE_KEY, MESSAGE_PLACE_SHIPS);
    yyjson_mut_val *length_arr = yyjson_mut_obj_add_arr(doc, root, LENGTH_KEY);
    for (uint8_t i = 0; i < ship_count; i++)
    {
        uint8_t length = ship_lengths[i];
        if (length == 0 || length > BSHIP_SHIP_LENGTH_MAX)
        {
            break;
        }
        yyjson_mut_arr_add_int(doc, length_arr, length);
    }

    size_t length = 0;
    char *json = yyjson_mut_write(doc, 0, &length);
    assert(length < BSHIP_MESSAGE_SIZE);

    strncpy(message->buffer, json, length);

    free(json);
    yyjson_mut_doc_free(doc);
}

BShip_Error BShip_Message_ShipsPlaced_Parse(BShip_Message message, BShip_ShipArray *ships, uint8_t ship_count)
{
    assert(message.buffer != NULL);
    assert(ships != NULL);
    assert(ships->buffer != NULL);
    assert(ships->capacity >= ship_count);
    assert(ship_count >= BSHIP_SHIP_COUNT_MIN);
    assert(ship_count <= BSHIP_SHIP_COUNT_MAX);

    BShip_Error error = {
        .type = ERROR_SUCCESS,
    };

    yyjson_doc *doc = yyjson_read(message.buffer, strlen(message.buffer), 0);
    if (doc == NULL) goto on_error;

    yyjson_val *root = yyjson_doc_get_root(doc);
    {
        yyjson_val *obj = yyjson_obj_get(root, MESSAGE_TYPE_KEY);
        if (!yyjson_is_uint(obj)) goto on_error;
        uint64_t type = yyjson_get_uint(obj);
        if (type != MESSAGE_SHIPS_PLACED) goto on_error;
    }
    yyjson_val *ships_obj = yyjson_obj_get(root, SHIP_KEY);
    if (!yyjson_is_arr(ships_obj)) goto on_error;
    size_t ships_obj_length = yyjson_arr_size(ships_obj);
    if (ships_obj_length != ship_count) goto on_error;

    for (ships->length = 0; ships->length < ship_count; ships->length++)
    {
        yyjson_val *ship_obj = yyjson_arr_get(ships_obj, ships->length);
        if (!yyjson_is_arr(ship_obj)) goto on_error;
        size_t ship_obj_length = yyjson_arr_size(ship_obj);
        if (ship_obj_length != 4) goto on_error;

        BShip_Ship ship = {0};
        
        yyjson_val *row_obj = yyjson_arr_get(ship_obj, 0);
        if (!yyjson_is_uint(row_obj)) goto on_error;
        ship.row = yyjson_get_uint(row_obj);
        
        yyjson_val *column_obj = yyjson_arr_get(ship_obj, 1);
        if (!yyjson_is_uint(column_obj)) goto on_error;
        ship.column = yyjson_get_uint(column_obj);
        
        yyjson_val *length_obj = yyjson_arr_get(ship_obj, 2);
        if (!yyjson_is_uint(length_obj)) goto on_error;
        ship.length = yyjson_get_uint(length_obj);
        
        yyjson_val *direction_obj = yyjson_arr_get(ship_obj, 3);
        if (!yyjson_is_uint(direction_obj)) goto on_error;
        ship.direction = (BShip_Direction)yyjson_get_uint(direction_obj);

        ships->buffer[ships->length] = ship;
    }

    yyjson_doc_free(doc);
    return error;
on_error:
    PRINT_ERROR_F("Invalid \"Ships Placed\" message received: <%s>", message.buffer);
    yyjson_doc_free(doc);
    error.type = ERROR_MESSAGE_SHIPS_PLACED_INVALID;
    error.value.message = message;
    return error;
}

BShip_Error BShip_Message_ShotTaken_Parse(BShip_Message message, BShip_Shot *shot)
{
    assert(message.buffer != NULL);
    assert(shot != NULL);

    BShip_Error error = {
        .type = ERROR_SUCCESS,
    };

    yyjson_doc *doc = yyjson_read(message.buffer, strlen(message.buffer), 0);
    if (doc == NULL) goto on_error;

    yyjson_val *root = yyjson_doc_get_root(doc);
    {
        yyjson_val *obj = yyjson_obj_get(root, MESSAGE_TYPE_KEY);
        if (!yyjson_is_uint(obj)) goto on_error;
        uint64_t type = yyjson_get_uint(obj);
        if (type != MESSAGE_SHOT_TAKEN) goto on_error;
    }
    {
        yyjson_val *obj = yyjson_obj_get(root, ROW_KEY);
        if (!yyjson_is_uint(obj)) goto on_error;
        uint64_t row = yyjson_get_uint(obj);
        if (row >= BSHIP_BOARD_SIZE_MAX) goto on_error;

        shot->row = row;
    }
    {
        yyjson_val *obj = yyjson_obj_get(root, COLUMN_KEY);
        if (!yyjson_is_uint(obj)) goto on_error;
        uint64_t column = yyjson_get_uint(obj);
        if (column >= BSHIP_BOARD_SIZE_MAX) goto on_error;

        shot->column = column;
    }

    yyjson_doc_free(doc);
    return error;
on_error:
    PRINT_ERROR_F("Invalid \"Shot Taken\" message received: <%s>", message.buffer);
    yyjson_doc_free(doc);
    error.type = ERROR_MESSAGE_SHOT_TAKEN_INVALID;
    error.value.message = message;
    return error;
}

void BShip_Message_ShotResult_Create(BShip_Message *message, BShip_Shot shot1, BShip_Shot shot2,
        BShip_Ship *ai1_ship_killed, BShip_Ship *ai2_ship_killed, bool next_shot)
{
    assert(message != NULL);
    assert(message->buffer != NULL);
    memset(message->buffer, 0, BSHIP_MESSAGE_SIZE);

    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    yyjson_mut_obj_add_int(doc, root, MESSAGE_TYPE_KEY, MESSAGE_SHOT_RESULT);

    yyjson_mut_val *shots_arr = yyjson_mut_obj_add_arr(doc, root, SHOT_KEY);
    yyjson_mut_val *ai1_shot_arr = yyjson_mut_arr_add_arr(doc, shots_arr);
    yyjson_mut_arr_add_int(doc, ai1_shot_arr, shot1.row);
    yyjson_mut_arr_add_int(doc, ai1_shot_arr, shot1.column);
    yyjson_mut_arr_add_int(doc, ai1_shot_arr, shot1.value);

    yyjson_mut_val *ai2_shot_arr = yyjson_mut_arr_add_arr(doc, shots_arr);
    yyjson_mut_arr_add_int(doc, ai2_shot_arr, shot2.row);
    yyjson_mut_arr_add_int(doc, ai2_shot_arr, shot2.column);
    yyjson_mut_arr_add_int(doc, ai2_shot_arr, shot2.value);

    if (ai1_ship_killed != NULL || ai2_ship_killed != NULL)
    {
        yyjson_mut_val *ships_arr = yyjson_mut_obj_add_arr(doc, root, SHIP_KEY);
        if (ai1_ship_killed == NULL)
        {
            yyjson_mut_arr_add_null(doc, ships_arr);
        }
        else
        {
            yyjson_mut_val *ai1_ship_arr = yyjson_mut_arr_add_arr(doc, ships_arr);
            yyjson_mut_arr_add_int(doc, ai1_ship_arr, ai1_ship_killed->row);
            yyjson_mut_arr_add_int(doc, ai1_ship_arr, ai1_ship_killed->column);
            yyjson_mut_arr_add_int(doc, ai1_ship_arr, ai1_ship_killed->length);
            yyjson_mut_arr_add_int(doc, ai1_ship_arr, ai1_ship_killed->direction);
        }
        if (ai2_ship_killed == NULL)
        {
            yyjson_mut_arr_add_null(doc, ships_arr);
        }
        else
        {
            yyjson_mut_val *ai2_ship_arr = yyjson_mut_arr_add_arr(doc, ships_arr);
            yyjson_mut_arr_add_int(doc, ai2_ship_arr, ai2_ship_killed->row);
            yyjson_mut_arr_add_int(doc, ai2_ship_arr, ai2_ship_killed->column);
            yyjson_mut_arr_add_int(doc, ai2_ship_arr, ai2_ship_killed->length);
            yyjson_mut_arr_add_int(doc, ai2_ship_arr, ai2_ship_killed->direction);
        }
    }

    yyjson_mut_obj_add_bool(doc, root, NEXT_SHOT_KEY, next_shot);

    size_t length = 0;
    char *json = yyjson_mut_write(doc, 0, &length);
    assert(length <= BSHIP_MESSAGE_SIZE);

    strncpy(message->buffer, json, length);

    free(json);
    yyjson_mut_doc_free(doc);
}

void BShip_Message_MatchOver_Create(BShip_Message *message)
{
    assert(message != NULL);
    assert(message->buffer != NULL);
    memset(message->buffer, 0, BSHIP_MESSAGE_SIZE);

    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    yyjson_mut_obj_add_int(doc, root, MESSAGE_TYPE_KEY, MESSAGE_MATCH_OVER);

    size_t length = 0;
    char *json = yyjson_mut_write(doc, 0, &length);
    assert(length <= BSHIP_MESSAGE_SIZE);

    strncpy(message->buffer, json, length);

    free(json);
    yyjson_mut_doc_free(doc);
}

