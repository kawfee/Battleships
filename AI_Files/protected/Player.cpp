/**
 * @file Player.cpp
 * @author Matthew Getgen
 * @brief The Base Class for Player AIs.
 * @date 2022-11-22
 */

#include "Player.h"


int Player::play_match(char *socket_path, const char *ai_name, const char *author_names) {
    if ( connect_to_socket(socket_path) == -1 ) return -1;

    // send hello message
    create_hello_msg(ai_name, author_names);
    if ( send_msg() == -1 ) return -1;

    // recv setup_match message
    if ( recv_msg() == -1 ) return -1;

    json j = json::parse(this->msg);
    MessageType mt = (MessageType)j[MESSAGE_TYPE_KEY];
    if ( mt != setup_match ) {
        printf("Player Error: Incorrect setup_match message! (line: %d)\n", __LINE__);
        return -1;
    }
    PlayerNum player = (PlayerNum)j[PLAYER_NUM_KEY];
    int board_size = (int)j[BOARD_SIZE_KEY];

    handle_setup_match(player, board_size);

    Ship ship;
    Shot shot;
    int rv;
    
    do {
        rv = recv_msg();
        j = json::parse(this->msg);
        mt = (MessageType)j[MESSAGE_TYPE_KEY];

        if ( mt == start_game ) {
            handle_start_game();
            continue;
        } else if ( mt == place_ship ) {
            // get ship length
            int ship_length = (int)j[LEN_KEY];

            // choose a ship to place
            ship = choose_ship_place(ship_length);

            // send response
            create_ship_placed_msg(ship);
            if ( send_msg() == -1 ) return -1;
            continue;
        } else if ( mt == take_shot ) {
            // chose a place to shoot
            shot = choose_shot();

            // send response
            create_shot_taken_msg(shot);
            if ( send_msg() == -1 ) return -1;
            continue;
        } else if ( mt == shot_return ) {
            shot.row = (int)j[ROW_KEY];
            shot.col = (int)j[COL_KEY];
            shot.value = (BoardValue)j[VALUE_KEY];
            PlayerNum num = (PlayerNum)j[PLAYER_NUM_KEY];

            handle_shot_return(num, shot);
            continue;
        } else if ( mt == ship_dead ) {
            ship.row = (int)j[ROW_KEY];
            ship.col = (int)j[COL_KEY];
            ship.len = (int)j[LEN_KEY];
            ship.dir = (Direction)j[DIR_KEY];
            PlayerNum num = (PlayerNum)j[PLAYER_NUM_KEY];

            handle_ship_dead(num, ship);
            continue;
        } else if ( mt == game_over ) {
            handle_game_over();
            continue;
        } else {    // whether match or not.
            handle_match_over();
            break;
        }
    } while ( rv != -1 && mt != match_over );


    return 0;
}

int Player::connect_to_socket(char *socket_path) {
    sockaddr_un server_sock;
    socklen_t len;
    size_t socket_len;

    // use Unix Domain Socket.
    this->socket_desc = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( this->socket_desc == -1 ) return -1;

    // bind socket values to the socket file.
    server_sock.sun_family = AF_UNIX;
    memset(server_sock.sun_path, 0, sizeof(server_sock.sun_path));
    socket_len = strlen(socket_path);
    if ( socket_len > sizeof(server_sock.sun_path) - 1 ) {
        printf("Player Error: Server socket path too long (line: %d)\n", __LINE__);
        return -1;
    }
    memcpy(server_sock.sun_path, socket_path, socket_len+1);
    
    // connect to the socket, create a socket descriptor.
    len = strlen(server_sock.sun_path) + sizeof(server_sock.sun_family);
    // if connection fails, return -1;
    if ( connect(this->socket_desc, (sockaddr *)&server_sock, len) == -1) {
        printf("Player Error: %s (line: %d)\n", strerror(errno), __LINE__);
        return -1;
    }

    // return 0, successful connection.
    return 0;
}

int Player::send_msg() {
    if ( send(this->socket_desc, this->msg, MAX_MSG_SIZE, 0) == -1 ) {
        printf("Player Error: %s (line: %d)\n", strerror(errno), __LINE__);
        return -1;
    }
    return 0;
}

int Player::recv_msg() {
    memset(this->msg, 0, MAX_MSG_SIZE);
    if ( recv(this->socket_desc, this->msg, MAX_MSG_SIZE, 0) == -1 ) {
        printf("Player Error: %s (line: %d)\n", strerror(errno), __LINE__);
    }
    return 0;
}

void Player::create_hello_msg(const char *ai_name, const char *author_names) {
    json j;
    char ai[MAX_NAME_SIZE], authors[MAX_NAME_SIZE];
    memset(ai, 0, MAX_NAME_SIZE);
    memset(authors, 0, MAX_NAME_SIZE);
    int ai_len = strlen(ai_name), auth_len = strlen(author_names);

    // make sure the player doesn't make a big name.
    if ( ai_len >= MAX_NAME_SIZE )  memcpy(ai, ai_name, sizeof(ai)-1);
    else                            memcpy(ai, ai_name, ai_len);
    if ( auth_len >= MAX_NAME_SIZE )memcpy(authors, author_names, sizeof(authors)-1);
    else                            memcpy(authors, author_names, auth_len);
    j[MESSAGE_TYPE_KEY] = hello;
    j[AI_NAME_KEY]      = ai;
    j[AUTHOR_NAMES_KEY] = authors;
    append_json_to_msg(j);
    return;
}

void Player::create_ship_placed_msg(Ship &ship) {
    json j;
    j[MESSAGE_TYPE_KEY] = ship_placed;
    j[ROW_KEY] = ship.row;
    j[COL_KEY] = ship.col;
    j[LEN_KEY] = ship.len;
    j[DIR_KEY] = ship.dir;
    append_json_to_msg(j);
    return;
}

void Player::create_shot_taken_msg(Shot &shot) {
    json j;
    j[MESSAGE_TYPE_KEY] = shot_taken;
    j[ROW_KEY] = shot.row;
    j[COL_KEY] = shot.col;
    append_json_to_msg(j);
    return;
}

void Player::append_json_to_msg(json &j) {
    string m;
    m = j.dump();
    memset(this->msg, 0, MAX_MSG_SIZE);
    // if the message is larger than this, then it is probably an invalid message and the server will handle that.
    if ( m.size() < MAX_MSG_SIZE ) strncat(this->msg, m.c_str(), m.size());
    else                           strncat(this->msg, m.c_str(), MAX_MSG_SIZE-1);
    return;
}