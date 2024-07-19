/**
 * @file Player.cpp
 * @author Matthew Getgen
 * @brief The Base Class for Player AIs.
 * @date 2022-11-22
 */

#include "Player.h"
#include "defines.h"


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
    int board_size = (int)j[BOARD_SIZE_KEY];
    PlayerNum player = (PlayerNum)j[PLAYER_NUM_KEY];

    char my_key[3], their_key[3];
    memset(my_key, 0, sizeof(my_key));
    memset(their_key, 0, sizeof(their_key));
    if (player == PLAYER_1) {
        strncpy(my_key, PLAYER_1_KEY, sizeof(my_key));
        strncpy(their_key, PLAYER_2_KEY, sizeof(their_key));
    } else {
        strncpy(my_key, PLAYER_2_KEY, sizeof(my_key));
        strncpy(their_key, PLAYER_1_KEY, sizeof(their_key));

    }

    handle_setup_match(player, board_size);

    Ship ship1, ship2;
    Shot shot1, shot2;
    
    int rv = 0, ship_length;
    bool next_shot;
    
    do {
        next_shot = false;
        rv = recv_msg();
        if ( rv != 0 ) break;
        j = json::parse(this->msg);
        mt = (MessageType)j[MESSAGE_TYPE_KEY];

        switch (mt) {
            case start_game:
                handle_start_game();
                break;
            case place_ship:
                ship_length = (int)j[LEN_KEY];

                ship1 = choose_ship_place(ship_length);
                create_ship_placed_msg(ship1);
                if ( send_msg() == -1 ) return -1;

                break;
            case take_shot:
                shot1 = choose_shot();
                create_shot_taken_msg(shot1);
                if ( send_msg() == -1 ) return -1;
                break;
            case shot_return:
                shot1.row = (int)j[my_key][SHOT_KEY][ROW_KEY];
                shot1.col = (int)j[my_key][SHOT_KEY][COL_KEY];
                shot1.value = (BoardValue)j[my_key][SHOT_KEY][VALUE_KEY];

                shot2.row = (int)j[their_key][SHOT_KEY][ROW_KEY];
                shot2.col = (int)j[their_key][SHOT_KEY][COL_KEY];
                shot2.value = (BoardValue)j[their_key][SHOT_KEY][VALUE_KEY];

                next_shot = (bool)j[NEXT_SHOT_KEY];

                if (player == PLAYER_1) {
                    handle_shot_return(PLAYER_1, shot1);
                    handle_shot_return(PLAYER_2, shot2);
                } else {
                    handle_shot_return(PLAYER_2, shot1);
                    handle_shot_return(PLAYER_1, shot2);
                }

                if (j[my_key].contains(SHIP_KEY)) {
                    ship1.row = (int)j[my_key][SHIP_KEY][ROW_KEY];
                    ship1.col = (int)j[my_key][SHIP_KEY][COL_KEY];
                    ship1.len = (int)j[my_key][SHIP_KEY][LEN_KEY];
                    ship1.dir = (Direction)j[my_key][SHIP_KEY][DIR_KEY];

                    if (player == PLAYER_1)
                        handle_ship_dead(PLAYER_1, ship1);
                    else
                        handle_ship_dead(PLAYER_2, ship1);
                }
                if (j[their_key].contains(SHIP_KEY)) {
                    ship2.row = (int)j[their_key][SHIP_KEY][ROW_KEY];
                    ship2.col = (int)j[their_key][SHIP_KEY][COL_KEY];
                    ship2.len = (int)j[their_key][SHIP_KEY][LEN_KEY];
                    ship2.dir = (Direction)j[their_key][SHIP_KEY][DIR_KEY];

                    if (player == PLAYER_1)
                        handle_ship_dead(PLAYER_2, ship2);
                    else
                        handle_ship_dead(PLAYER_1, ship2);
                }

                if (next_shot) {
                    shot1 = choose_shot();
                    create_shot_taken_msg(shot1);
                    if ( send_msg() == -1 ) return -1;
                }

                break;
            case game_over:
                game_stats.result = (GameResult)j[GAME_RESULT_KEY];
                game_stats.num_board_shot = (int)j[NUM_BOARD_SHOT_KEY];
                game_stats.hits = (int)j[NUM_HITS_KEY];
                game_stats.misses = (int)j[NUM_MISSES_KEY];
                game_stats.duplicates = (int)j[NUM_DUPLICATES_KEY];
                game_stats.ships_killed = (int)j[SHIPS_KILLED_KEY];
                
                switch (game_stats.result) {
                    case WIN:
                        match_stats.wins++;
                        break;
                    case LOSS:
                        match_stats.losses++;
                        break;
                    case TIE:
                        match_stats.ties++;
                        break;
                }
                match_stats.total_num_board_shot += game_stats.num_board_shot;
                match_stats.total_hits += game_stats.hits;
                match_stats.total_misses += game_stats.misses;
                match_stats.total_duplicates += game_stats.duplicates;
                match_stats.total_ships_killed += game_stats.ships_killed;
                handle_game_over();
                break;
            case match_over:
                handle_match_over();
                break;
            default:
                handle_match_over();
                break;
        }
    } while ( rv != -1 && mt != match_over );

    return rv;
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

