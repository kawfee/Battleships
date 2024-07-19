/**
 * @file match_logic.cpp
 * @author Matthew Getgen
 * @brief Battleships Match Logic
 * @date 2023-08-28
 */

#include "match_logic.h"

MatchLog run_match(Connection &connect, MatchOptions &options, const char *socket_name) {
    MatchLog match;
    match.board_size = options.board_size;
    match.elapsed_time = 0;
    match.player1.error.type = OK;
    match.player2.error.type = OK;
    memset(&match.player1.stats, 0, sizeof(MatchStats));
    memset(&match.player2.stats, 0, sizeof(MatchStats));
    match.player1.ai_name = options.exec1.file_name;
    match.player2.ai_name = options.exec2.file_name;
    int status = 0;

    timeval start, end;

    gettimeofday(&start, NULL);
    status = start_players(match, connect, options, socket_name);
    // start players handles all the nitty-gritty of killing players and such.
    if (status) return match;

    status = handle_start_match(match, connect, options);
    if (status) {
        handle_match_over(connect, status);
        return match;
    }

    ShipInfo info = handle_ship_sizes(connect, options.board_size);

    Board board = create_boards(options.board_size);

    for (int i = 0; i < options.num_games; i++) {
        GameLog game = run_game(connect, board, info);
        merge_game_and_match_player(match.player1, game.player1);
        merge_game_and_match_player(match.player2, game.player2);
        match.games.push_back(game);
        
        status = check_match_errors(match);
        if (status) break;
    }

    handle_match_over(connect, status);
    gettimeofday(&end, NULL);
    store_elapsed_time(match, start, end);

    delete_boards(board);
    return match;
}

int start_players(MatchLog &match, Connection &connect, MatchOptions &options, const char *socket_name) {
    match.player1.error.type = start_player(connect.player1, connect.server_desc, options.exec1.exec.c_str(), socket_name);
    match.player2.error.type = start_player(connect.player2, connect.server_desc, options.exec2.exec.c_str(), socket_name);
    int status = check_match_errors_save_result(match);
    switch (status) {
    case -3:
        // both failed, and should already be dead.
        break;
    case -2:
        // Player 2 failed to start (and was killed). Kill player 1.
        kill_player(connect.player1.pid);
        break;
    case -1:
        // Player 1 failed to start (and was killed). Kill player 2.
        kill_player(connect.player2.pid);
        break;
    }
    return status;
}

int handle_start_match(MatchLog &match, Connection &connect, MatchOptions &options) {
    int status = 0;
    
    match.player1.error.type = recv_msg(connect.player1.desc, connect.player1.msg);
    match.player2.error.type = recv_msg(connect.player2.desc, connect.player2.msg);
    status = check_match_errors_save_result(match);
    if (status) return status;

    match.player1.error = parse_hello_msg(connect.player1.msg, match.player1.ai_name, match.player1.author_name);
    match.player2.error = parse_hello_msg(connect.player2.msg, match.player2.ai_name, match.player2.author_name);
    status = check_match_errors_save_result(match);
    if (status) return status;

    create_setup_match_msg(connect.player1.msg, options.board_size, PLAYER_1);
    create_setup_match_msg(connect.player2.msg, options.board_size, PLAYER_2);

    match.player1.error.type = send_msg(connect.player1.desc, connect.player1.msg);
    match.player2.error.type = send_msg(connect.player2.desc, connect.player2.msg);
    status = check_match_errors_save_result(match);

    return status;
}

ShipInfo handle_ship_sizes(Connection &connect, int board_size) {
    ShipInfo info;

    switch (board_size) {
    case 10:
        info.num_ships = 6;
        info.max_len = 5;
        info.min_len = 3;
        break;
    case 9:
        info.num_ships = 5;
        info.max_len = 5;
        info.min_len = 3;
        break;
    case 8:
        info.num_ships = 5;
        info.max_len = 4;
        info.min_len = 3;
        break;
    case 7:
        info.num_ships = 5;
        info.max_len = 4;
        info.min_len = 2;
        break;
    case 6:
        info.num_ships = 5;
        info.max_len = 3;
        info.min_len = 2;
        break;
    case 5:
        info.num_ships = 4;
        info.max_len = 3;
        info.min_len = 2;
        break;
    case 4:
        info.num_ships = 4;
        info.max_len = 3;
        info.min_len = 1;
        break;
    case 3:
        info.num_ships = 3;
        info.max_len = 2;
        info.min_len = 1;
        break;
    default:
        print_error(BOARD_SIZE_ERR, __FILE__, __LINE__);
        printf("Size of board received: %d\n", board_size);
        kill_player(connect.player1.desc);
        kill_player(connect.player2.desc);
        close_sockets(connect);
        printf("\nExiting.\n");
        exit(1);
    }
    return info;
}

void merge_game_and_match_player(MatchPlayer &match, GamePlayer &game) {
    match.error = game.error;
    match.stats.total_num_board_shot += game.stats.num_board_shot;
    match.stats.total_hits += game.stats.hits;
    match.stats.total_misses += game.stats.misses;
    match.stats.total_duplicates += game.stats.duplicates;
    match.stats.total_ships_killed += game.stats.ships_killed;

    switch (game.stats.result) {
    case WIN:
        match.stats.wins++;
        break;
    case LOSS:
        match.stats.losses++;
        break;
    case TIE:
        match.stats.ties++;
        break;
    }
    return;
}

void handle_match_over(Connection &connect, int last_status) {
    create_match_over_msg(connect.player1.msg);
    create_match_over_msg(connect.player2.msg);

    switch (last_status) {
    case -3:
        // kill both players. Both are tweaking.
        kill_player(connect.player1.pid);
        kill_player(connect.player2.pid);
        break;
    case -2:
        // Send message to player 1 to let it peacefully exit.
        send_msg(connect.player1.desc, connect.player1.msg);
        wait_player(connect.player1);
        // kill the troublemaker.
        kill_player(connect.player2.pid);
        break;
    case -1:
        // kill the troublemaker.
        kill_player(connect.player1.pid);
        // Send message to player 2 to let it peacefully exit.
        send_msg(connect.player2.desc, connect.player2.msg);
        wait_player(connect.player2);
        break;
    case 0:
    default:
        // Let both players peacefully exit.
        send_msg(connect.player1.desc, connect.player1.msg);
        send_msg(connect.player2.desc, connect.player2.msg);
        wait_player(connect.player1);
        wait_player(connect.player2);
        break;
    }
    close_player_sockets(connect);
    return;
}

int check_match_errors_save_result(MatchLog &match) {
    int status = check_match_errors(match);
    switch (status) {
    case -3:
        match.player1.stats.ties++;
        match.player2.stats.ties++;
        break;
    case -2:
        match.player1.stats.wins++;
        match.player2.stats.losses++;
        break;
    case -1:
        match.player1.stats.losses++;
        match.player2.stats.wins++;
        break;
    default:
        break;
    }
    return status;
}

int check_match_errors(MatchLog &match) {
    int status = 0;
    if ( match.player1.error.type != OK ) status -= 1;
    if ( match.player2.error.type != OK ) status -= 2;

    return status;
}

void store_elapsed_time(MatchLog &match, timeval &start, timeval &end) {
    long seconds = end.tv_sec - start.tv_sec;
    // without floating-point precision because I don't care about microsecond granularity.
    long milliseconds = (end.tv_usec - start.tv_usec) / 1000;

    // We just want the difference, don't worry if one is bigger than the other.
    if ( seconds < 0 ) seconds *= -1;
    if ( milliseconds < 0 ) milliseconds *= -1;

    // with floating-point precision because I the millisecond granularity.
    match.elapsed_time = (float)seconds + ((float)milliseconds / 1000.0);
    return;
}

