/**
 * @file message.cpp
 * @author Matthew Getgen
 * @brief Battleships Server Message Functions
 * @date 2023-08-27
 *
 * JSON C++ Library from [nlohmann](https://github.com/nlohmann/json)
 */

#include "message.h"


/* ──────────────────────── *
 * CREATE MESSAGE FUNCTIONS *
 * ──────────────────────── */

void append_json_to_msg(char *msg, json &j) {
    string m;
    m = j.dump();
    memset(msg, 0, MAX_MSG_SIZE);
    if (m.size() < MAX_MSG_SIZE) strncat(msg, m.c_str(), m.size());
    else                         strncat(msg, m.c_str(), MAX_MSG_SIZE);
    return;
}

void create_setup_match_msg(char *msg, int board_size, PlayerNum num) {
    json j = json::object();
    j[MESSAGE_TYPE_KEY] = SetupMatch;
    j[BOARD_SIZE_KEY] = board_size;
    j[PLAYER_NUM_KEY] = num;
    append_json_to_msg(msg, j);
    return;
}

void create_start_game_msg(char *msg) {
    json j = json::object();
    j[MESSAGE_TYPE_KEY] = StartGame;
    append_json_to_msg(msg, j);
    return;
}

void create_place_ship_msg(char *msg, int length) {
    json j = json::object();
    j[MESSAGE_TYPE_KEY] = PlaceShip;
    j[LEN_KEY] = length;
    append_json_to_msg(msg, j);
    return;
}

void create_take_shot_msg(char *msg) {
    json j = json::object();
    j[MESSAGE_TYPE_KEY] = TakeShot;
    append_json_to_msg(msg, j);
    return;
}

void create_shot_return_msg(char *msg, Shot &shot1, Shot &shot2, GameLog &game, bool next_shot) {
    json j = json::object();
    j[MESSAGE_TYPE_KEY] = ShotReturn;

    j[PLAYER_1_KEY] = json::object();

    j[PLAYER_1_KEY][SHOT_KEY] = json::object(); 
    j[PLAYER_1_KEY][SHOT_KEY][ROW_KEY] = shot1.row;
    j[PLAYER_1_KEY][SHOT_KEY][COL_KEY] = shot1.col;
    j[PLAYER_1_KEY][SHOT_KEY][VALUE_KEY] = shot1.value;

    j[PLAYER_2_KEY] = json::object();

    j[PLAYER_2_KEY][SHOT_KEY] = json::object(); 
    j[PLAYER_2_KEY][SHOT_KEY][ROW_KEY] = shot2.row;
    j[PLAYER_2_KEY][SHOT_KEY][COL_KEY] = shot2.col;
    j[PLAYER_2_KEY][SHOT_KEY][VALUE_KEY] = shot2.value;

    // player1 killed a ship!
    if ( shot1.ship_sunk_idx != -1) {
        j[PLAYER_2_KEY][SHIP_KEY] = json::object();
        j[PLAYER_2_KEY][SHIP_KEY][ROW_KEY] = game.player2.ships.at(shot1.ship_sunk_idx).row;
        j[PLAYER_2_KEY][SHIP_KEY][COL_KEY] = game.player2.ships.at(shot1.ship_sunk_idx).col;
        j[PLAYER_2_KEY][SHIP_KEY][LEN_KEY] = game.player2.ships.at(shot1.ship_sunk_idx).len;
        j[PLAYER_2_KEY][SHIP_KEY][DIR_KEY] = game.player2.ships.at(shot1.ship_sunk_idx).dir;
    }
    
    // player2 killed a ship!
    if ( shot2.ship_sunk_idx != -1) {
        j[PLAYER_1_KEY][SHIP_KEY] = json::object();
        j[PLAYER_1_KEY][SHIP_KEY][ROW_KEY] = game.player1.ships.at(shot2.ship_sunk_idx).row;
        j[PLAYER_1_KEY][SHIP_KEY][COL_KEY] = game.player1.ships.at(shot2.ship_sunk_idx).col;
        j[PLAYER_1_KEY][SHIP_KEY][LEN_KEY] = game.player1.ships.at(shot2.ship_sunk_idx).len;
        j[PLAYER_1_KEY][SHIP_KEY][DIR_KEY] = game.player1.ships.at(shot2.ship_sunk_idx).dir;
    }

    j[NEXT_SHOT_KEY] = next_shot;
    append_json_to_msg(msg, j);
    return;
}

void create_game_over_msg(char *msg, GameStats &stats) {
    json j;
    j[MESSAGE_TYPE_KEY] = GameOver;
    j[GAME_RESULT_KEY] = stats.result;
    j[NUM_BOARD_SHOT_KEY] = stats.num_board_shot;
    j[NUM_HITS_KEY] = stats.hits;
    j[NUM_MISSES_KEY] = stats.misses;
    j[NUM_DUPLICATES_KEY] = stats.duplicates;
    j[SHIPS_KILLED_KEY] = stats.ships_killed;
    append_json_to_msg(msg, j);
    return;
}

void create_match_over_msg(char *msg) {
    json j;
    j[MESSAGE_TYPE_KEY] = MatchOver;
    append_json_to_msg(msg, j);
    return;
}


/* ─────────────────────── *
 * PARSE MESSAGE FUNCTIONS *
 * ─────────────────────── */

Error parse_hello_msg(char *msg, string &ai_name, string &author_name) {
    Error error;
    error.type = OK;
    json j;

    if ( !validate_hello_msg(msg, j) ) {
        print_error(HELLO_MESSAGE_ERR, __FILE__, __LINE__);
        printf("Message received: <%s>\n", msg);
        error.type = ErrHelloMessage;
        error.message = msg;
        return error;
    }
    
    string ai = j[AI_NAME_KEY];
    string authors = j[AUTHOR_NAMES_KEY];
    if (ai.size() > MAX_NAME_SIZE) ai_name = ai.substr(0, MAX_NAME_SIZE);
    else                           ai_name = ai;
    if (authors.size() > MAX_NAME_SIZE) author_name = authors.substr(0, MAX_NAME_SIZE);
    else                                author_name = authors;
    return error;
}

bool validate_hello_msg(char *msg, json &j) {
    bool valid = false;

    // check if JSON.
    valid = json::accept(msg);
    if ( !valid ) return false;

    j = json::parse(msg, nullptr, false);
    // check contains right keys.
    valid = j.contains(MESSAGE_TYPE_KEY) &&
            j.contains(AI_NAME_KEY) &&
            j.contains(AUTHOR_NAMES_KEY);
    if ( !valid ) return false;
    
    // check contains right types.
    valid = j[MESSAGE_TYPE_KEY].is_number_integer() &&
            j[AI_NAME_KEY].is_string() &&
            j[AUTHOR_NAMES_KEY].is_string();
    if ( !valid ) return false;

    // check right message type.
    valid = (j[MESSAGE_TYPE_KEY] == Hello);
    return valid;
}

Error parse_ship_placed_msg(char *msg, Ship &ship) {
    Error error;
    error.type = OK;
    json j;
    
    if ( !validate_ship_placed_msg(msg, j) ) {
        print_error(SHIP_MESSAGE_ERR, __FILE__, __LINE__);
        printf("Message received: <%s>\n", msg);
        error.type = ErrShipPlacedMessage;
        error.message = msg;
        return error;
    }

    ship.row = j[ROW_KEY];
    ship.col = j[COL_KEY];
    ship.len = j[LEN_KEY];
    ship.dir = (Direction)j[DIR_KEY];
    return error;
}

bool validate_ship_placed_msg(char *msg, json &j) {
    bool valid = false;
    
    // check if JSON.
    valid = json::accept(msg);
    if ( !valid ) return false;

    j = json::parse(msg, nullptr, false);
    // check contains right keys.
    valid = j.contains(MESSAGE_TYPE_KEY) &&
            j.contains(ROW_KEY) &&
            j.contains(COL_KEY) &&
            j.contains(LEN_KEY) &&
            j.contains(DIR_KEY);
    if ( !valid ) return false;
    
    // check contains right types.
    valid = j[MESSAGE_TYPE_KEY].is_number_integer() &&
            j[ROW_KEY].is_number_integer() &&
            j[COL_KEY].is_number_integer() &&
            j[LEN_KEY].is_number_integer() &&
            j[DIR_KEY].is_number_integer();
    if ( !valid ) return false;

    // check right message type.
    valid = (j[MESSAGE_TYPE_KEY] == ShipPlaced);
    if ( !valid ) return false;

    // check valid Direction values.
    valid = (j[DIR_KEY] == HORIZONTAL || j[DIR_KEY] == VERTICAL);
    return valid;
}

Error parse_shot_taken_msg(char *msg, Shot &shot) {
    Error error;
    error.type = OK;
    json j;
    
    if ( !validate_shot_taken_msg(msg, j) ) {
        print_error(SHOT_MESSAGE_ERR, __FILE__, __LINE__);
        printf("Message received: <%s>\n", msg);
        error.type = ErrShotTakenMessage;
        error.message = msg;
        return error;
    }

    shot.row = j[ROW_KEY];
    shot.col = j[COL_KEY];
    return error;
}

bool validate_shot_taken_msg(char *msg, json &j) {
    bool valid = false;
    
    // check if JSON.
    valid = json::accept(msg);
    if ( !valid ) return false;

    j = json::parse(msg, nullptr, false);
    // check contains right keys.
    valid = j.contains(MESSAGE_TYPE_KEY) &&
            j.contains(ROW_KEY) &&
            j.contains(COL_KEY);
    if ( !valid ) return false;
    
    // check contains right types.
    valid = j[MESSAGE_TYPE_KEY].is_number_integer() &&
            j[ROW_KEY].is_number_integer() &&
            j[COL_KEY].is_number_integer();
    if ( !valid ) return false;

    // check right message type.
    valid = (j[MESSAGE_TYPE_KEY] == ShotTaken);
    return valid;
}

