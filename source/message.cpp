/**
 * @file messages.cpp
 * @author Matthew Getgen
 * @brief Battleships Server Message Functions
 * @date 2022-06-14
 */

#include "message.h"


/* ──────────────────────── *
 * CREATE MESSAGE FUNCTIONS *
 * ──────────────────────── */

void append_json_to_msg(char *msg, json j) {
    string m;
    m = j.dump();
    memset(msg, 0, MAX_MSG_SIZE);
    strncat(msg, m.c_str(), m.size());
    return;
}

void create_setup_match_msg(char *msg, int boardSize, PlayerNum num) {
    json j;
    j[MESSAGE_TYPE_KEY] = setup_match;
    j[BOARD_SIZE_KEY] = boardSize;
    j[PLAYER_NUM_KEY] = num;
    append_json_to_msg(msg, j);
    return;
}

void create_start_game_msg(char *msg) {
    json j;
    j[MESSAGE_TYPE_KEY] = start_game;
    append_json_to_msg(msg, j);
    return;
}

void create_place_ship_msg(char *msg, int len) {
    json j;
    j[MESSAGE_TYPE_KEY] = place_ship;
    j[LEN_KEY] = len;
    append_json_to_msg(msg, j);
    return;
}

void create_take_shot_msg(char *msg) {
    json j;
    j[MESSAGE_TYPE_KEY] = take_shot;
    append_json_to_msg(msg, j);
    return;
}

void create_shot_return_msg(char *msg, PlayerNum num, Shot shot) {
    json j;
    j[MESSAGE_TYPE_KEY] = shot_return;
    j[PLAYER_NUM_KEY] = num;
    j[ROW_KEY] = shot.row;
    j[COL_KEY] = shot.col;
    j[VALUE_KEY] = shot.value;
    append_json_to_msg(msg, j);
    return;
}

void create_ship_dead_msg(char *msg, PlayerNum num, Ship ship) {
    json j;
    j[MESSAGE_TYPE_KEY] = ship_dead;
    j[PLAYER_NUM_KEY] = num;
    j[ROW_KEY] = ship.row;
    j[COL_KEY] = ship.col;
    j[LEN_KEY] = ship.len;
    j[DIR_KEY] = ship.dir;
    append_json_to_msg(msg, j);
    return;
}

void create_game_over_msg(char *msg) {
    json j;
    j[MESSAGE_TYPE_KEY] = game_over;
    append_json_to_msg(msg, j);
    return;
}

void create_match_over_msg(char *msg) {
    json j;
    j[MESSAGE_TYPE_KEY] = match_over;
    append_json_to_msg(msg, j);
    return;
}


/* ─────────────────────── *
 * PARSE MESSAGE FUNCTIONS *
 * ─────────────────────── */

ErrorNum parse_hello_msg(char *msg, char *ai_name, char *author_names) {
    json j;

    try {
        // check if JSON.
        if ( !json::accept(msg) ) throw BAD_HELLO_MSG;
        j = json::parse(msg, nullptr, false);
        // check if has correct keys.
        if ( !j.contains(MESSAGE_TYPE_KEY) ||
             !j.contains(AI_NAME_KEY) ||
             !j.contains(AUTHOR_NAMES_KEY) ) throw BAD_HELLO_MSG;
        // check if correct types.
        if ( !j[MESSAGE_TYPE_KEY].is_number_integer() ||
             !j[AI_NAME_KEY].is_string() ||
             !j[AUTHOR_NAMES_KEY].is_string() ) throw BAD_HELLO_MSG;
        // check if correct message type.
        if ( j[MESSAGE_TYPE_KEY] != hello ) throw BAD_HELLO_MSG;
        

    } catch (...) {
        print_error(HELLO_MSG_ERR, __FILE__, __LINE__);
        printf("Message received: <%s>\n", msg);
        return BAD_HELLO_MSG;
    }
    
    string ai = j[AI_NAME_KEY];
    string authors = j[AUTHOR_NAMES_KEY];
    if (ai.size() > MAX_NAME_SIZE)      strncpy(ai_name, ai.c_str(), MAX_NAME_SIZE-1);
    else                                strncpy(ai_name, ai.c_str(), ai.size());
    if (authors.size() > MAX_NAME_SIZE) strncpy(author_names, authors.c_str(), MAX_NAME_SIZE-1);
    else                                strncpy(author_names, authors.c_str(), authors.size());
    return NO_ERR;
}

ErrorNum parse_ship_placed_msg(char *msg, Ship &ship) {
    json j;
    
    try {
        // check if JSON.
        if ( !json::accept(msg) ) throw BAD_SHIP_PLACED_MSG;
        j = json::parse(msg, nullptr, false);
        // check if has correct keys.
        if ( !j.contains(MESSAGE_TYPE_KEY) ||
             !j.contains(ROW_KEY) ||
             !j.contains(COL_KEY) ||
             !j.contains(LEN_KEY) || 
             !j.contains(DIR_KEY) ) throw BAD_SHIP_PLACED_MSG;
        // check if correct types.
        if ( !j[MESSAGE_TYPE_KEY].is_number_integer() ||
             !j[ROW_KEY].is_number_integer() ||
             !j[COL_KEY].is_number_integer() ||
             !j[LEN_KEY].is_number_integer() ||
             !j[DIR_KEY].is_number_integer() ) throw BAD_SHIP_PLACED_MSG;
        // check if Direction values are correct.
        if ( j[DIR_KEY] != HORIZONTAL && j[DIR_KEY] != VERTICAL ) throw BAD_SHIP_PLACED_MSG;
        // check if correct message type.
        if ( j[MESSAGE_TYPE_KEY] != ship_placed ) throw BAD_SHIP_PLACED_MSG;

    } catch (...) {
        print_error(SHIP_MSG_ERR, __FILE__, __LINE__);
        printf("Message received: <%s>\n", msg);
        return BAD_SHIP_PLACED_MSG;
    }

    ship.row = j[ROW_KEY];
    ship.col = j[COL_KEY];
    ship.len = j[LEN_KEY];
    ship.dir = j[DIR_KEY];
    return NO_ERR;
}

ErrorNum parse_shot_taken_msg(char *msg, Shot &shot) {
    json j;
    
    try {
        // check if JSON.
        if ( !json::accept(msg) ) throw BAD_SHOT_TAKEN_MSG;
        j = json::parse(msg, nullptr, false);
        // check if has correct keys.
        if ( !j.contains(MESSAGE_TYPE_KEY) || 
             !j.contains(ROW_KEY) || 
             !j.contains(COL_KEY) ) throw BAD_SHOT_TAKEN_MSG;
        // check if correct types.
        if ( !j[MESSAGE_TYPE_KEY].is_number_integer() ||
             !j[ROW_KEY].is_number_integer() ||
             !j[COL_KEY].is_number_integer() ) throw BAD_SHOT_TAKEN_MSG;
        // check if correct message type.
        if ( j[MESSAGE_TYPE_KEY] != shot_taken ) throw BAD_SHOT_TAKEN_MSG;

    } catch (...) {
        print_error(SHOT_MSG_ERR, __FILE__, __LINE__);
        printf("Message received: <%s>\n", msg);
        return BAD_SHOT_TAKEN_MSG;
    }

    shot.row = j[ROW_KEY];
    shot.col = j[COL_KEY];
    return NO_ERR;
}

