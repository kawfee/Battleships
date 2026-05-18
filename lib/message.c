/**
 * @file message.c
 * @author Matthew Getgen
 * @brief Battleships message logic
 * @date 2026-05-15
 */

#include <string.h>

#include "vendor/yyjson/src/yyjson.h"
#include "battleshipslib.h"

#define MESSAGE_SIZE_MAX 256
#define MESSAGE_NAME_SIZE_MAX 100
#define MESSAGE_TYPE_KEY    "mt"
#define PLAYER_NUM_KEY      "pn"
#define AI_NAME_KEY         "ai"
#define AUTHOR_NAMES_KEY    "au"
#define BOARD_SIZE_KEY      "bs"
#define LEN_KEY             "l"
#define ROW_KEY             "r"
#define COL_KEY             "c"
#define DIR_KEY             "d"
#define VALUE_KEY           "v"
#define SHIP_KEY            "sp"
#define SHOT_KEY            "st"
#define NEXT_SHOT_KEY       "ns"

typedef enum {
    MESSAGE_HELLO,
    MESSAGE_SETUP_MATCH,
    MESSAGE_PLACE_SHIPS,
    MESSAGE_SHIPS_PLACED,
    MESSAGE_SHOT_TAKEN,
    MESSAGE_SHOT_RESULT,
    MESSAGE_MATCH_OVER,
} BShip_MessageType;

void BShip_Message_Deallocate(BShip_Message *message)
{
    if (message == NULL || message->json == NULL)
    {
        return;
    }

    free(message->json);
    message->json = NULL;
    message->length = 0;
}

BShip_ErrorType BShip_Message_Hello_Parse(BShip_Message message, char *ai_name, char *author_names)
{
    assert(message.json != NULL);
    assert(message.length <= MESSAGE_SIZE_MAX);
    assert(ai_name != NULL);
    assert(author_names != NULL);

    yyjson_doc *doc = yyjson_read(message.json, message.length, 0);
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
        if (ai_name_len > MESSAGE_NAME_SIZE_MAX)
        {
            ai_name_len = MESSAGE_NAME_SIZE_MAX;
        }
        const char *ai_name_input = yyjson_get_str(obj);
        strncpy(ai_name, ai_name_input, ai_name_len);
    }
    {
        yyjson_val *obj = yyjson_obj_get(root, AUTHOR_NAMES_KEY);
        if (!yyjson_is_str(obj)) goto on_error;
        size_t author_names_len = yyjson_get_len(obj);
        if (author_names_len > MESSAGE_NAME_SIZE_MAX)
        {
            author_names_len = MESSAGE_NAME_SIZE_MAX;
        }
        const char *author_names_input = yyjson_get_str(obj);
        strncpy(author_names, author_names_input, author_names_len);
    }

    yyjson_doc_free(doc);
    return ERROR_SUCCESS;
on_error:
    yyjson_doc_free(doc);
    return ERROR_INVALID_HELLO_MESSAGE;
}

BShip_Message BShip_Message_SetupMatch_Create(uint8_t board_size, BShip_PlayerNum player_num)
{
    BShip_Message message = {0};
    assert(board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(board_size <= BSHIP_BOARD_SIZE_MAX);

    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    yyjson_mut_obj_add_int(doc, root, MESSAGE_TYPE_KEY, MESSAGE_SETUP_MATCH);
    yyjson_mut_obj_add_int(doc, root, BOARD_SIZE_KEY, board_size);
    yyjson_mut_obj_add_int(doc, root, PLAYER_NUM_KEY, player_num);

    message.json = yyjson_mut_write(doc, 0, &message.length);

    yyjson_mut_doc_free(doc);
    return message;
}

BShip_Message BShip_Message_PlaceShips_Create(uint8_t *ship_lengths)
{
    BShip_Message message = {0};
    assert(ship_lengths != NULL);

    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    yyjson_mut_obj_add_int(doc, root, MESSAGE_TYPE_KEY, MESSAGE_PLACE_SHIPS);
    yyjson_mut_val *len_arr = yyjson_mut_obj_add_arr(doc, root, LEN_KEY);
    for (uint8_t i = 0; i < BSHIP_SHIP_COUNT_MAX; i++)
    {
        uint8_t length = ship_lengths[i];
        if (length == 0 || length > BSHIP_SHIP_LENGTH_MAX)
        {
            break;
        }
        yyjson_mut_arr_add_int(doc, len_arr, length);
    }

    message.json = yyjson_mut_write(doc, 0, &message.length);

    yyjson_mut_doc_free(doc);
    return message;
}

BShip_ErrorType BShip_Message_ShipsPlaced_Parse(BShip_Message message, uint8_t *ship_lengths, BShip_Ship *ships)
{
    assert(message.json != NULL);
    assert(message.length <= MESSAGE_SIZE_MAX);
    assert(ship_lengths != NULL);
    assert(ships != NULL);

    yyjson_doc *doc = yyjson_read(message.json, message.length, 0);
    if (doc != NULL) goto on_error;

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
    if (ships_obj_length == 0 || ships_obj_length > BSHIP_SHIP_COUNT_MAX) goto on_error;

    for (uint8_t i = 0; i < ships_obj_length; i++)
    {
        uint8_t ship_length = ship_lengths[i];
        // returned ship lengths should have equaled expected ship lengths
        if (ship_length == 0) goto on_error;
        yyjson_val *ship_obj = yyjson_arr_get(ships_obj, i);
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

        if (ship.length != ship_length) goto on_error;

        ships[i] = ship;
    }

    yyjson_doc_free(doc);
    return ERROR_SUCCESS;
on_error:
    if (doc != NULL)
    yyjson_doc_free(doc);
    return ERROR_INVALID_SHIPS_PLACED_MESSAGE;
}

BShip_ErrorType BShip_Message_ShotTaken_Parse(BShip_Message message, BShip_Shot *shot)
{
    assert(message.json != NULL);
    assert(message.length <= MESSAGE_SIZE_MAX);
    assert(shot != NULL);

    yyjson_doc *doc = yyjson_read(message.json, message.length, 0);
    if (doc != NULL) goto on_error;

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
        yyjson_val *obj = yyjson_obj_get(root, COL_KEY);
        if (!yyjson_is_uint(obj)) goto on_error;
        uint64_t column = yyjson_get_uint(obj);
        if (column >= BSHIP_BOARD_SIZE_MAX) goto on_error;

        shot->column = column;
    }

    yyjson_doc_free(doc);
    return ERROR_SUCCESS;
on_error:
    if (doc != NULL)
    yyjson_doc_free(doc);
    return ERROR_INVALID_SHOT_TAKEN_MESSAGE;
}

BShip_Message BShip_Message_ShotResult_Create(BShip_Shot shot1, BShip_Shot shot2,
        BShip_Ship *ai1_ship_killed, BShip_Ship *ai2_ship_killed, bool next_shot)
{
    BShip_Message message = {0};

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

    yyjson_mut_obj_add_bool(doc, root, NEXT_SHOT_KEY, next_shot);

    message.json = yyjson_mut_write(doc, 0, &message.length);

    yyjson_mut_doc_free(doc);
    return message;
}

BShip_Message BShip_Message_MatchOver_Create()
{
    BShip_Message message = {0};

    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    yyjson_mut_obj_add_int(doc, root, MESSAGE_TYPE_KEY, MESSAGE_MATCH_OVER);

    message.json = yyjson_mut_write(doc, 0, &message.length);

    yyjson_mut_doc_free(doc);
    return message;
}
