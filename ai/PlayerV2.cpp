/**
 * @file PlayerV2.cpp
 * @author Matthew Getgen
 * @brief The Implementation of the Base Class for the PlayerV2 AIs.
 * @date 2026-05-28
 */

#include "PlayerV2.h"
#include "json.hpp"

using json = nlohmann::json;

Shot get_shot_from_shot_result_message(json j, PlayerNum player) {
    Shot shot = {};
    int index = player == PLAYER_1 ? 0 : 1;
    shot.row = (int)j[SHOT_KEY].at(index).at(0);
    shot.col = (int)j[SHOT_KEY].at(index).at(1);
    shot.value = (BoardValue)j[SHOT_KEY].at(index).at(2);
    return shot;
}

bool get_ship_from_shot_result_message(json j, PlayerNum player, Ship &ship) {
    int index = player == PLAYER_1 ? 0 : 1;
    if (j[SHIP_KEY].at(index).is_null()) {
        return false;
    }
    ship.row = (int)j[SHIP_KEY].at(index).at(0);
    ship.col = (int)j[SHIP_KEY].at(index).at(1);
    ship.len = (int)j[SHIP_KEY].at(index).at(2);
    ship.dir = (Direction)j[SHIP_KEY].at(index).at(3);
    return true;
}

bool PlayerV2::play_match(char *socket_path, const char *ai_name, const char *author_names) {
    if (!connect_to_socket(socket_path)) return false;

    // hello code
    message_hello_create(ai_name, author_names);
    if (!message_send()) return false;

    // setup match code
    {
        if (!message_receive()) return false;

        json j = json::parse(this->message);
        MessageType type = (MessageType)j[MESSAGE_TYPE_KEY];
        if (type != MESSAGE_SETUP_MATCH) {
            PRINT_ERROR("Incorrect \"SETUP_MATCH\" message!");
            return false;
        }

        PlayerNum player = (PlayerNum)j[PLAYER_NUM_KEY];
        int board_size = (int)j[BOARD_SIZE_KEY];

        handle_setup_match(player, board_size);
    }

    // all other messages
    MessageType type = MESSAGE_SETUP_MATCH;
    while (type != MESSAGE_MATCH_OVER) {
        if (!message_receive()) {
            handle_match_over();
            return false;
        }

        json j = json::parse(this->message);
        type = (MessageType)j[MESSAGE_TYPE_KEY];

        vector<int> ship_lengths = {};
        vector<Ship> ships = {};
        Shot shot1 = {}, shot2 = {};
        Ship ship1 = {}, ship2 = {};
        bool has_ship1 = false, has_ship2 = false, next_shot = false;
        switch (type) {
        case MESSAGE_HELLO:
        case MESSAGE_SETUP_MATCH:
        case MESSAGE_SHIPS_PLACED:
        case MESSAGE_SHOT_TAKEN:
            PRINT_ERROR_F("Invalid message received: %s", this->message.c_str());
            return false;
            break;
        case MESSAGE_PLACE_SHIPS:
            handle_start_game();

            for (int i = 0; i < (int)j[LENGTH_KEY].size(); i++) {
                ship_lengths.push_back((int)j[LENGTH_KEY].at(i));
            }
            ships = choose_ship_placements(ship_lengths);
            message_ships_placed_create(ships);
            if (!message_send()) return false;

            shot1 = choose_next_shot();
            message_shot_taken_create(shot1);
            if (!message_send()) return false;
            break;
        case MESSAGE_SHOT_RESULT:
            shot1 = get_shot_from_shot_result_message(j, PLAYER_1);
            shot2 = get_shot_from_shot_result_message(j, PLAYER_2);

            handle_shot_result(PLAYER_1, shot1);
            handle_shot_result(PLAYER_2, shot2);

            if (j.contains(SHIP_KEY)) {
                has_ship1 = get_ship_from_shot_result_message(j, PLAYER_1, ship1);
                has_ship2 = get_ship_from_shot_result_message(j, PLAYER_2, ship2);
                if (has_ship1) {
                    handle_ship_dead(PLAYER_1, ship1);
                }
                if (has_ship2) {
                    handle_ship_dead(PLAYER_2, ship2);
                }
            }

            next_shot = (bool)j[NEXT_SHOT_KEY];
            if (next_shot) {
                shot1 = choose_next_shot();
                message_shot_taken_create(shot1);
                if (!message_send()) return false;
            } else {
                handle_game_over();
            }
            break;
        case MESSAGE_MATCH_OVER:
            handle_match_over();
            break;
        }
    }
    return true;
}

bool PlayerV2::connect_to_socket(char *socket_path) {
    sockaddr_un server_sock;
    socklen_t len;
    size_t socket_len;

    this->socket_desc = socket(AF_UNIX, SOCK_STREAM, 0);
    if (this->socket_desc == -1) {
        PRINT_ERROR(strerror(errno));
        return false;
    }

    server_sock.sun_family = AF_UNIX;
    memset(server_sock.sun_path, 0, sizeof(server_sock.sun_path));
    socket_len = strlen(socket_path);
    if (socket_len > sizeof(server_sock.sun_path)-1) {
        PRINT_ERROR("Server socket path is too long");
        return false;
    }
    memcpy(server_sock.sun_path, socket_path, socket_len+1);

    len = strlen(server_sock.sun_path) + sizeof(server_sock.sun_family);
    if (connect(this->socket_desc, (sockaddr *)&server_sock, len) == -1) {
        PRINT_ERROR(strerror(errno));
        return false;
    }

    return true;
}

bool PlayerV2::message_send() {
    if (this->message.size() > MAX_MESSAGE_SIZE) {
        this->message.resize(MAX_MESSAGE_SIZE);
    }
    if (send(this->socket_desc, this->message.c_str(), this->message.size(), 0) == -1) {
        PRINT_ERROR(strerror(errno));
        return false;
    }
    return true;
}

bool PlayerV2::message_receive() {
    this->message.clear();
    char message_buffer[MAX_MESSAGE_SIZE+1] = {};
    if (recv(this->socket_desc, message_buffer, MAX_MESSAGE_SIZE, 0) == -1) {
        PRINT_ERROR(strerror(errno));
        return false;
    }
    this->message = message_buffer;
    return true;
}

void PlayerV2::message_hello_create(const char *ai_name, const char *author_names) {
    char ai[MAX_NAME_SIZE+1] = {}, authors[MAX_NAME_SIZE+1] = {};
    int ai_len = strlen(ai_name), authors_len = strlen(author_names);
    ai_len = ai_len < MAX_NAME_SIZE ? ai_len : MAX_NAME_SIZE;
    authors_len = authors_len < MAX_NAME_SIZE ? authors_len : MAX_NAME_SIZE;
    memcpy(ai, ai_name, ai_len);
    memcpy(authors, author_names, authors_len);
    json j = {
        {MESSAGE_TYPE_KEY, MESSAGE_HELLO},
        {AI_NAME_KEY, ai},
        {AUTHOR_NAMES_KEY, authors},
    };
    this->message = j.dump();
}

void PlayerV2::message_ships_placed_create(vector<Ship> &ships) {
    json j = {
        {MESSAGE_TYPE_KEY, MESSAGE_SHIPS_PLACED},
        {SHIP_KEY, json::array()},
    };

    for (unsigned int i = 0; i < ships.size(); i++) {
        Ship ship = ships.at(i);
        json nested = json::array();
        nested.push_back(ship.row);
        nested.push_back(ship.col);
        nested.push_back(ship.len);
        nested.push_back(ship.dir);
        j[SHIP_KEY].push_back(nested);
    }
    this->message = j.dump();
}

void PlayerV2::message_shot_taken_create(Shot shot) {
    json j = {
        {MESSAGE_TYPE_KEY, MESSAGE_SHIPS_PLACED},
        {ROW_KEY, shot.row},
        {COLUMN_KEY, shot.col},
    };
    this->message = j.dump();
}

