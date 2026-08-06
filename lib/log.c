/**
 * @file log.c
 * @author Matthew Getgen
 * @brief Battleships message logic
 * @date 2026-08-03
 */

#include "vendor/yyjson/src/yyjson.h"
#include "arena.c"

#define BOARD_SIZE_KEY   "bs"
#define EVENTS_KEY       "evt"
#define AI1_KEY          "ai1"
#define AI2_KEY          "ai2"
#define AI_NAME_KEY      "ai"
#define AUTHOR_NAMES_KEY "au"
#define ERROR_KEY        "err"
#define ERROR_TYPE_KEY   "ert"
#define EXIT_STATUS_KEY  "es"
#define SHIP_KEY         "sp"
#define SHOT_KEY         "st"
#define MESSAGE_KEY      "msg"

BSHIP_DEFINE_ARRAY_PUSH(BShip_EventArray, BShip_Event)
BSHIP_DEFINE_ARRAY_PUSH(BShip_U32Array, uint32_t)

void BShip_AIMatchData_Create(yyjson_mut_doc *doc, yyjson_mut_val *ai_root, BShip_AIMatchData match)
{
    yyjson_mut_obj_add_strn(doc, ai_root, AI_NAME_KEY, match.name, match.ai_name_length);
    yyjson_mut_obj_add_strn(doc, ai_root, AUTHOR_NAMES_KEY, match.authors, match.author_name_length);
    if (match.error.type != ERROR_SUCCESS)
    {
        yyjson_mut_val *err = yyjson_mut_obj_add_obj(doc, ai_root, ERROR_KEY);
        yyjson_mut_obj_add_uint(doc, err, ERROR_TYPE_KEY, match.error.type);
        yyjson_mut_obj_add_sint(doc, err, EXIT_STATUS_KEY, match.error.exit_status);
        switch (match.error.type)
        {
        case ERROR_SUCCESS:
        case ERROR_AI_PATH_ISSUE:
        case ERROR_PROCESS_FAILED:
        case ERROR_CONNECTION_FAILED:
        case ERROR_CONNECTION_TIMEOUT:
        case ERROR_SEND_FAILED:
        case ERROR_SEND_TIMEOUT:
        case ERROR_RECEIVE_FAILED:
        case ERROR_RECEIVE_TIMEOUT:
        case ERROR_RECEIVE_EMPTY_MESSAGE:
            break;
        case ERROR_MESSAGE_HELLO_INVALID:
        case ERROR_MESSAGE_SHIPS_PLACED_INVALID:
        case ERROR_MESSAGE_SHOT_TAKEN_INVALID:
            yyjson_mut_obj_add_strn(doc, err, MESSAGE_KEY,
                match.error.value.message.buffer, match.error.value.message.length);
            break;
        case ERROR_SHIP_LENGTH_INVALID:
        case ERROR_SHIP_OFF_BOARD:
        case ERROR_SHIP_OVERLAP:
        {
            yyjson_mut_val *ship = yyjson_mut_obj_add_arr(doc, err, SHIP_KEY);
            yyjson_mut_arr_add_uint(doc, ship, match.error.value.ship.row);
            yyjson_mut_arr_add_uint(doc, ship, match.error.value.ship.column);
            yyjson_mut_arr_add_uint(doc, ship, match.error.value.ship.length);
            yyjson_mut_arr_add_uint(doc, ship, match.error.value.ship.direction);
        } break;
        case ERROR_SHOT_OFF_BOARD:
        case ERROR_SHOT_DUPLICATE:
        {
            yyjson_mut_val *shot = yyjson_mut_obj_add_arr(doc, err, SHOT_KEY);
            yyjson_mut_arr_add_uint(doc, shot, match.error.value.shot.row);
            yyjson_mut_arr_add_uint(doc, shot, match.error.value.shot.column);
            yyjson_mut_arr_add_uint(doc, shot, match.error.value.shot.value);
        } break;
        }
    }
}

void BShip_Match_Log_Store(BShip_MatchData match, char *path)
{
    assert(path != NULL);
    assert(match.board_size >= BSHIP_BOARD_SIZE_MIN);
    assert(match.board_size <= BSHIP_BOARD_SIZE_MAX);
    assert(match.events.buffer != NULL);

    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    yyjson_mut_obj_add_uint(doc, root, BOARD_SIZE_KEY, match.board_size);
    yyjson_mut_val *ai1 = yyjson_mut_obj_add_obj(doc, root, AI1_KEY);
    yyjson_mut_val *ai2 = yyjson_mut_obj_add_obj(doc, root, AI2_KEY);

    BShip_AIMatchData_Create(doc, ai1, match.ai1);
    BShip_AIMatchData_Create(doc, ai2, match.ai2);

    yyjson_mut_val *events = yyjson_mut_obj_add_arr(doc, root, EVENTS_KEY);
    for (size_t i = 0; i < match.events.length; i++)
    {
        BShip_Event e = match.events.buffer[i];
        if (e.type == BSHIP_EVENT_NONE) continue;
        yyjson_mut_val *event = yyjson_mut_arr_add_arr(doc, events);
        yyjson_mut_arr_add_uint(doc, event, e.type);
        switch (e.type)
        {
        case BSHIP_EVENT_NONE:
        case BSHIP_EVENT_GAME_START:
            break;
        case BSHIP_EVENT_SHIP_PLACEMENT:
            yyjson_mut_arr_add_uint(doc, event, e.value.compact.ai1_ship);
            yyjson_mut_arr_add_uint(doc, event, e.value.compact.ai2_ship);
            break;
        case BSHIP_EVENT_SHOT_RESULT:
            yyjson_mut_arr_add_uint(doc, event, e.value.compact.ai1_shot);
            yyjson_mut_arr_add_uint(doc, event, e.value.compact.ai2_shot);
            if (e.value.compact.ai1_ship != 0 || e.value.compact.ai2_ship != 0)
            {
                yyjson_mut_arr_add_uint(doc, event, e.value.compact.ai1_ship);
                yyjson_mut_arr_add_uint(doc, event, e.value.compact.ai2_ship);
            }
            break;
        case BSHIP_EVENT_GAME_RESULT:
            yyjson_mut_arr_add_uint(doc, event, e.value.ai1_game_result);
            break;
        }
    }

    size_t length = 0;
    char *json = yyjson_mut_write(doc, 0, &length);

    BShip_Buffer buffer = {
        .buffer = (uint8_t *)json,
        .length = length,
        .capacity = length,
    };

    BShip_File_Write(path, &buffer);

    free(json);
    yyjson_mut_doc_free(doc);
}

bool BShip_AIMatchData_Parse(BShip_Arena *arena, yyjson_val *ai_root, BShip_AIMatchData *match)
{
    assert(arena != NULL);
    assert(match != NULL);
    assert(match->name != NULL);
    assert(match->authors != NULL);
    {
        yyjson_val *obj = yyjson_obj_get(ai_root, AI_NAME_KEY);
        if (!yyjson_is_str(obj)) goto on_error;
        size_t name_len = yyjson_get_len(obj);
        if (name_len > BSHIP_MESSAGE_NAME_SIZE_MAX)
        {
            name_len = BSHIP_MESSAGE_NAME_SIZE_MAX;
        }
        const char *name = yyjson_get_str(obj);
        strncpy(match->name, name, name_len);
    }
    {
        yyjson_val *obj = yyjson_obj_get(ai_root, AUTHOR_NAMES_KEY);
        if (!yyjson_is_str(obj)) goto on_error;
        size_t authors_len = yyjson_get_len(obj);
        if (authors_len > BSHIP_MESSAGE_NAME_SIZE_MAX)
        {
            authors_len = BSHIP_MESSAGE_NAME_SIZE_MAX;
        }
        const char *authors = yyjson_get_str(obj);
        strncpy(match->authors, authors, authors_len);
    }

    yyjson_val *err_obj = yyjson_obj_get(ai_root, ERROR_KEY);
    match->error.type = ERROR_SUCCESS;
    if (err_obj != NULL)
    {
        if (!yyjson_is_obj(err_obj)) goto on_error;
        {
            yyjson_val *obj = yyjson_obj_get(err_obj, ERROR_TYPE_KEY);
            if (!yyjson_is_uint(obj)) goto on_error;
            match->error.type = (BShip_ErrorType)yyjson_get_uint(obj);
        }
        {
            yyjson_val *obj = yyjson_obj_get(err_obj, EXIT_STATUS_KEY);
            if (!yyjson_is_sint(obj)) goto on_error;
            match->error.exit_status = yyjson_get_sint(obj);
        }
        switch (match->error.type)
        {
        case ERROR_SUCCESS:
        case ERROR_AI_PATH_ISSUE:
        case ERROR_PROCESS_FAILED:
        case ERROR_CONNECTION_FAILED:
        case ERROR_CONNECTION_TIMEOUT:
        case ERROR_SEND_FAILED:
        case ERROR_SEND_TIMEOUT:
        case ERROR_RECEIVE_FAILED:
        case ERROR_RECEIVE_TIMEOUT:
        case ERROR_RECEIVE_EMPTY_MESSAGE:
            break;
        case ERROR_MESSAGE_HELLO_INVALID:
        case ERROR_MESSAGE_SHIPS_PLACED_INVALID:
        case ERROR_MESSAGE_SHOT_TAKEN_INVALID:
        {
            yyjson_val *obj = yyjson_obj_get(err_obj, MESSAGE_KEY);
            if (!yyjson_is_str(obj)) goto on_error;
            size_t message_len = yyjson_get_len(obj);
            if (message_len > BSHIP_MESSAGE_SIZE)
            {
                message_len = BSHIP_MESSAGE_SIZE;
            }
            match->error.value.message.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_SIZE);
            if (match->error.value.message.buffer == NULL) goto on_error;
            memset(match->error.value.message.buffer, 0, BSHIP_MESSAGE_SIZE);
            const char *message = yyjson_get_str(obj);
            strncpy(match->error.value.message.buffer, message, message_len);
        } break;
        case ERROR_SHIP_LENGTH_INVALID:
        case ERROR_SHIP_OFF_BOARD:
        case ERROR_SHIP_OVERLAP:
        {
            yyjson_val *obj = yyjson_obj_get(err_obj, SHIP_KEY);
            if (!yyjson_is_arr(obj)) goto on_error;
            if (yyjson_arr_size(obj) != 4) goto on_error;
            
            yyjson_val *row = yyjson_arr_get(obj, 0);
            if (!yyjson_is_uint(row)) goto on_error;
            match->error.value.ship.row = yyjson_get_uint(row);
            
            yyjson_val *column = yyjson_arr_get(obj, 1);
            if (!yyjson_is_uint(column)) goto on_error;
            match->error.value.ship.column = yyjson_get_uint(column);
            
            yyjson_val *length = yyjson_arr_get(obj, 2);
            if (!yyjson_is_uint(length)) goto on_error;
            match->error.value.ship.length = yyjson_get_uint(length);
            
            yyjson_val *direction = yyjson_arr_get(obj, 3);
            if (!yyjson_is_uint(direction)) goto on_error;
            match->error.value.ship.direction = (BShip_Direction)yyjson_get_uint(direction);
        } break;
        case ERROR_SHOT_OFF_BOARD:
        case ERROR_SHOT_DUPLICATE:
        {
            yyjson_val *obj = yyjson_obj_get(err_obj, SHOT_KEY);
            if (!yyjson_is_arr(obj)) goto on_error;
            if (yyjson_arr_size(obj) != 3) goto on_error;
            
            yyjson_val *row = yyjson_arr_get(obj, 0);
            if (!yyjson_is_uint(row)) goto on_error;
            match->error.value.shot.row = yyjson_get_uint(row);
            
            yyjson_val *column = yyjson_arr_get(obj, 1);
            if (!yyjson_is_uint(column)) goto on_error;
            match->error.value.shot.column = yyjson_get_uint(column);
            
            yyjson_val *value = yyjson_arr_get(obj, 2);
            if (!yyjson_is_uint(value)) goto on_error;
            match->error.value.shot.value = (BShip_BoardValue)yyjson_get_uint(value);
        } break;
        }
    }

    return true;
on_error:
    return false;
}

bool BShip_Match_Log_Load(BShip_Arena *arena, BShip_MatchData *match, char *path)
{
    assert(arena != NULL);
    assert(match != NULL);
    assert(path != NULL);
    ssize_t file_size = BShip_File_GetSize(path);
    if (file_size == -1)
    {
        PRINT_ERROR_F("Match log not found at %s", path);
        return false;
    }

    BSHIP_ARENA_TEMP_BEGIN(arena);

    BShip_Buffer buffer = {
        .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint8_t, file_size),
        .capacity = file_size,
    };

    if (!BShip_File_Read(path, &buffer, file_size)) return false;

    yyjson_doc *doc = yyjson_read((const char *)buffer.buffer, buffer.length, 0);
    if (doc == NULL) goto on_error;
    BSHIP_ARENA_TEMP_END(arena);
    {
        match->ai1.name = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_NAME_SIZE_MAX);
        match->ai1.authors = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_NAME_SIZE_MAX);
        if (match->ai1.name == NULL || match->ai1.authors == NULL)
        {
            goto on_error;
        }
        memset(match->ai1.name, 0, BSHIP_MESSAGE_NAME_SIZE_MAX);
        memset(match->ai1.authors, 0, BSHIP_MESSAGE_NAME_SIZE_MAX);
    }
    {
        match->ai2.name = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_NAME_SIZE_MAX);
        match->ai2.authors = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_NAME_SIZE_MAX);
        if (match->ai2.name == NULL || match->ai2.authors == NULL)
        {
            goto on_error;
        }
        memset(match->ai2.name, 0, BSHIP_MESSAGE_NAME_SIZE_MAX);
        memset(match->ai2.authors, 0, BSHIP_MESSAGE_NAME_SIZE_MAX);
    }

    yyjson_val *root = yyjson_doc_get_root(doc);
    {
        yyjson_val *obj = yyjson_obj_get(root, BOARD_SIZE_KEY);
        if (!yyjson_is_uint(obj)) goto on_error;
        match->board_size = yyjson_get_uint(obj);
    }
    if (match->board_size < BSHIP_BOARD_SIZE_MIN || match->board_size > BSHIP_BOARD_SIZE_MAX) goto on_error;
    {
        yyjson_val *obj = yyjson_obj_get(root, AI1_KEY);
        if (!yyjson_is_obj(obj)) goto on_error;
        if (!BShip_AIMatchData_Parse(arena, obj, &match->ai1)) goto on_error;
    }
    {
        yyjson_val *obj = yyjson_obj_get(root, AI2_KEY);
        if (!yyjson_is_obj(obj)) goto on_error;
        if (!BShip_AIMatchData_Parse(arena, obj, &match->ai2)) goto on_error;
    }

    uint32_t games_per_match_max = BShip_GamesPerMatchMax_From_BoardSize(match->board_size);
    match->game_indexes.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint32_t, games_per_match_max);
    if (match->game_indexes.buffer == NULL) goto on_error;
    match->game_indexes.capacity = games_per_match_max;

    yyjson_val *events = yyjson_obj_get(root, EVENTS_KEY);
    if (!yyjson_is_arr(events)) goto on_error;

    size_t events_size = yyjson_arr_size(events);
    match->events.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_Event, events_size);
    if (match->events.buffer == NULL) goto on_error;
    match->events.capacity = events_size;

    for (size_t i = 0; i < events_size; i++)
    {
        yyjson_val *event = yyjson_arr_get(events, i);
        if (!yyjson_is_arr(event)) goto on_error;
        size_t event_size = yyjson_arr_size(event);
        if (event_size == 0 || event_size > 5) goto on_error;
        
        yyjson_val *event_type = yyjson_arr_get(event, 0);
        if (!yyjson_is_uint(event_type)) goto on_error;
        BShip_Event e = {
            .type = (BShip_EventType)yyjson_get_uint(event_type),
        };
        switch (e.type)
        {
        case BSHIP_EVENT_NONE:
        case BSHIP_EVENT_GAME_START:
            break;
        case BSHIP_EVENT_SHIP_PLACEMENT:
        {
            if (event_size != 3) goto on_error;
            yyjson_val *ai1_ship = yyjson_arr_get(event, 1);
            if (!yyjson_is_uint(ai1_ship)) goto on_error;
            e.value.compact.ai1_ship = yyjson_get_uint(ai1_ship);

            yyjson_val *ai2_ship = yyjson_arr_get(event, 2);
            if (!yyjson_is_uint(ai2_ship)) goto on_error;
            e.value.compact.ai2_ship = yyjson_get_uint(ai2_ship);
        } break;
        case BSHIP_EVENT_SHOT_RESULT:
        {
            if (event_size != 3 && event_size != 5) goto on_error;
            yyjson_val *ai1_shot = yyjson_arr_get(event, 1);
            if (!yyjson_is_uint(ai1_shot)) goto on_error;
            e.value.compact.ai1_shot = yyjson_get_uint(ai1_shot);

            yyjson_val *ai2_shot = yyjson_arr_get(event, 2);
            if (!yyjson_is_uint(ai2_shot)) goto on_error;
            e.value.compact.ai2_shot = yyjson_get_uint(ai2_shot);

            if (event_size == 5)
            {
                yyjson_val *ai1_ship = yyjson_arr_get(event, 3);
                if (!yyjson_is_uint(ai1_ship)) goto on_error;
                e.value.compact.ai1_ship = yyjson_get_uint(ai1_ship);

                yyjson_val *ai2_ship = yyjson_arr_get(event, 4);
                if (!yyjson_is_uint(ai2_ship)) goto on_error;
                e.value.compact.ai2_ship = yyjson_get_uint(ai2_ship);
            }
        } break;
        case BSHIP_EVENT_GAME_RESULT:
        {
            if (event_size != 2) goto on_error;

            yyjson_val *game_result = yyjson_arr_get(event, 1);
            if (!yyjson_is_uint(game_result)) goto on_error;
            e.value.ai1_game_result = (BShip_GameResult)yyjson_get_uint(game_result);

        } break;
        }
        if (e.type == BSHIP_EVENT_GAME_START)
        {
            BShip_U32Array_Push(&match->game_indexes, match->events.length);
        }
        BShip_EventArray_Push(&match->events, e);
    }


    yyjson_doc_free(doc);
    return true;
on_error:
    PRINT_ERROR_F("Invalid match log stored at %s", path);
    yyjson_doc_free(doc);
    return false;
}
