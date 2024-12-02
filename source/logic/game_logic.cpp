/**
 * @file game_logic.cpp
 * @author Matthew Getgen
 * @brief Battleships Game Logic
 * @date 2023-08-28
 */

#include "game_logic.h"


GameLog run_game(Connection &connect, Board &board, ShipInfo &ship_info) {
    GameLog game;
    game.player1.error.type = OK;
    game.player2.error.type = OK;
    memset(&game.player1.stats, 0, sizeof(GameStats));
    memset(&game.player2.stats, 0, sizeof(GameStats));

    int status;
    clear_boards(board);

    create_start_game_msg(connect.player1.msg);

    game.player1.error.type = send_msg(connect.player1.desc, connect.player1.msg);
    game.player2.error.type = send_msg(connect.player2.desc, connect.player1.msg);
    status = check_game_errors(game);
    if (status) return game;

    status = handle_ships(game, connect, board, ship_info);
    if (status) return game;

    create_take_shot_msg(connect.player1.msg);

    game.player1.error.type = send_msg(connect.player1.desc, connect.player1.msg);
    game.player2.error.type = send_msg(connect.player2.desc, connect.player1.msg);
    status = check_game_errors(game);
    if (status) return game;

    bool next_shot = true;

    int max_num_shots = board.size * board.size;
    // gives every AI the chance to hit at every location once. This should be enough.
    for (int i = 0; i < max_num_shots; i++) {
        if (i == (max_num_shots-1)) next_shot = false;
        status = handle_shots(game, connect, board, next_shot);
        if (status) return game;
        if (!next_shot) break;
    }

    calculate_winner(game);
    handle_game_over(game, connect);

    return game;
}

int handle_ships(GameLog &game, Connection &connect, Board &board, ShipInfo &ship_info) {
    int status = 0, random, length;
    for (int i = 0; i < ship_info.num_ships; i++) {
        random = (rand() % 6);
        switch (random) {
        case 5:
            length = ship_info.max_len;
            break;
        case 4:
        case 3:
            length = (int)((ship_info.max_len + ship_info.min_len) / 2);
            break;
        case 2:
        case 1:
        case 0:
        default:
            length = ship_info.min_len;
            break;
        }

        status = handle_ship_placement(game, connect, board, length);
        if (status) return status;
    }
    return status;
}

int handle_ship_placement(GameLog &game, Connection &connect, Board &board, int length) {
    int status = 0;
    Ship ship1, ship2;

    create_place_ship_msg(connect.player1.msg, length);

    game.player1.error.type = send_msg(connect.player1.desc, connect.player1.msg);
    game.player2.error.type = send_msg(connect.player2.desc, connect.player1.msg);
    status = check_game_errors(game);
    if (status) return status;

    game.player1.error.type = recv_msg(connect.player1.desc, connect.player1.msg);
    game.player2.error.type = recv_msg(connect.player2.desc, connect.player2.msg);
    status = check_game_errors(game);
    if (status) return status;

    game.player1.error = parse_ship_placed_msg(connect.player1.msg, ship1);
    game.player2.error = parse_ship_placed_msg(connect.player2.msg, ship2);
    status = check_game_errors(game);
    if (status) return status;

    game.player1.error = validate_ship_placement(board, PLAYER_1, ship1, length);
    game.player2.error = validate_ship_placement(board, PLAYER_2, ship2, length);
    status = check_game_errors(game);
    if (status) return status;

    store_ship_board_value(board, PLAYER_1, ship1, SHIP);
    store_ship_board_value(board, PLAYER_2, ship2, SHIP);

    ship1.alive = true;
    ship2.alive = true;

    game.player1.ships.push_back(ship1);
    game.player2.ships.push_back(ship2);

    return status;
}

Error validate_ship_placement(
    Board &board,
    PlayerNum num,
    Ship &ship,
    int expected_length
) {
    Error error;
    error.type = OK;

    if ( ship.len != expected_length ) {
        error.type = ErrShipLength;
        error.ship = ship;
        print_error(SHIP_PLACE_ERR, __FILE__, __LINE__);
        printf("Ship returned with length: %d, but expected length: %d\n",
            ship.len, expected_length
        );
        return error;
    }

    int front = 0, end = board.size, check = 0;
    if ( ship.dir == HORIZONTAL ) {
        front = ship.col;
        end = ship.col + (ship.len - 1);
        check = ship.row;
    } else if ( ship.dir == VERTICAL ) {
        front = ship.row;
        end = ship.row + (ship.len - 1);
        check = ship.col;
    }

    if ( front < 0 || end >= board.size || check < 0 || check >= board.size ) {
        error.type = ErrShipOffBoard;
        error.ship = ship;
        print_error(SHIP_PLACE_ERR, __FILE__, __LINE__);
        printf("Ship returned with\n\trow: %d\n\tcol: %d\n\tlength: %d\n\tdirection: %c\ndoesn't fit onto the board!\n", ship.row, ship.col, ship.len, (char)ship.dir);
        return error;
    }

    int r = ship.row, c = ship.col;
    int rm = ship.dir == VERTICAL, cm = ship.dir == HORIZONTAL;
    Shot shot;
    for (int l = 0; l < ship.len; l++) {
        shot.row = r + (l * rm);
        shot.col = c + (l * cm);
        if (get_shot_board_value(board, num, shot) != WATER) {
            error.type = ErrShipIntersect;
            error.ship = ship;
            print_error(SHIP_PLACE_ERR, __FILE__, __LINE__);
            printf("Ship returned with\n\trow: %d\n\tcol: %d\n\tlength: %d\n\tdirection: %c\nintersects with a ship already on the board!\n", ship.row, ship.col, ship.len, (char)ship.dir);
            return error;
        }
    }

    return error;
}

int handle_shots(
    GameLog &game,
    Connection &connect,
    Board &board,
    bool &next_shot
) {
    int status = 0;
    Shot shot1, shot2;
    shot1.ship_sunk_idx = -1;
    shot2.ship_sunk_idx = -1;

    status = handle_shot_placement(game, connect, board, shot1, shot2);
    if (status) return status;

    // Different players because it's the opponent's board.
    calculate_shot_value(game.player1.stats, shot1, PLAYER_2, board);    
    calculate_shot_value(game.player2.stats, shot2, PLAYER_1, board);    

    shot1.ship_sunk_idx = find_dead_ship(game.player2, PLAYER_2, board);
    shot2.ship_sunk_idx = find_dead_ship(game.player1, PLAYER_1, board);

    if ( shot1.ship_sunk_idx != -1 ) game.player1.stats.ships_killed++;
    if ( shot2.ship_sunk_idx != -1 ) game.player2.stats.ships_killed++;

    if ( count_alive_ships(game.player1) == 0 || 
         count_alive_ships(game.player2) == 0 ) next_shot = false;
    
    create_shot_return_msg(connect.player1.msg, shot1, shot2, game, next_shot);

    game.player1.shots.push_back(shot1);
    game.player2.shots.push_back(shot2);

    game.player1.error.type = send_msg(connect.player1.desc, connect.player1.msg);
    game.player2.error.type = send_msg(connect.player2.desc, connect.player1.msg);
    status = check_game_errors(game);

    return status;
}

int handle_shot_placement(
    GameLog &game,
    Connection &connect,
    Board &board,
    Shot &shot1, Shot &shot2
) {
    int status = 0;

    game.player1.error.type = recv_msg(connect.player1.desc, connect.player1.msg);
    game.player2.error.type = recv_msg(connect.player2.desc, connect.player2.msg);
    status = check_game_errors(game);
    if (status) return status;

    game.player1.error = parse_shot_taken_msg(connect.player1.msg, shot1);
    game.player2.error = parse_shot_taken_msg(connect.player2.msg, shot2);
    status = check_game_errors(game);
    if (status) return status;

    game.player1.error = validate_shot_placement(board.size, shot1);
    game.player2.error = validate_shot_placement(board.size, shot2);
    status = check_game_errors(game);

    return status;
}

Error validate_shot_placement(int size, Shot &shot) {
    Error error;
    error.type = OK;
    // check upper and lower bound
    if ( shot.row < 0 || shot.row >= size
      || shot.col < 0 || shot.col >= size ) {
        print_error(SHOT_PLACE_ERR, __FILE__, __LINE__);
        printf("Shot returned with\n\trow: %d\n\tcol: %d\ndoesn't fit onto the board!\n", shot.row, shot.col);
        error.type = ErrShotOffBoard;
        error.shot = shot;
    }
    return error;
}

void calculate_shot_value(
    GameStats &stats,
    Shot &shot,
    PlayerNum opponent,
    Board &board
) {
    BoardValue value = get_shot_board_value(board, opponent, shot);

    switch (value) {
    case SHIP:
        shot.value = HIT;
        stats.hits++;
        stats.num_board_shot++;
        break;
    case WATER:
        shot.value = MISS;
        stats.misses++;
        stats.num_board_shot++;
        break;
    case HIT:
    case DUPLICATE_HIT:
        shot.value = DUPLICATE_HIT;
        stats.duplicates++;
        break;
    case MISS:
    case DUPLICATE_MISS:
        shot.value = DUPLICATE_MISS;
        stats.duplicates++;
        break;
    case KILL:
    case DUPLICATE_KILL:
        shot.value = DUPLICATE_KILL;
        stats.duplicates++;
        break;
    }
    store_shot_board_value(board, opponent, shot);
    return;
}

int find_dead_ship(GamePlayer &player, PlayerNum num, Board &board) {
    int dead_index = -1;

    for (int s = 0; s < (int)player.ships.size(); s++) {
        if ( !player.ships.at(s).alive ) continue;

        Ship &ship = player.ships.at(s);
        
        if (board_ship_died(board, num, ship)) {
            dead_index = s;
            ship.alive = false;
            store_ship_board_value(board, num, ship, KILL);
            break;
        }
    }
    return dead_index;
}

int count_alive_ships(GamePlayer &player) {
    int num_alive = 0;
    for (int s = 0; s < (int)player.ships.size(); s++) {
        if ( player.ships.at(s).alive ) num_alive++;
    }
    return num_alive;
}

void calculate_winner(GameLog &game) {
    int alive1 = count_alive_ships(game.player1);
    int alive2 = count_alive_ships(game.player2);

    if ( alive1 == 0 && alive2 == 0 ) {
        game.player1.stats.result = TIE;
        game.player2.stats.result = TIE;
    } else if ( alive1 > 0 && alive2 == 0 ) {
        game.player1.stats.result = WIN;
        game.player2.stats.result = LOSS;
    } else if ( alive1 == 0 && alive2 > 0 ) {
        game.player1.stats.result = LOSS;
        game.player2.stats.result = WIN;
    } else {
        // both have alive ships, time for tiebreaker.
        if ( alive1 > alive2 ) {
            game.player1.stats.result = WIN;
            game.player2.stats.result = LOSS;
        } else if ( alive1 < alive2 ) {
            game.player1.stats.result = LOSS;
            game.player2.stats.result = WIN;
        } else {
            game.player1.stats.result = TIE;
            game.player2.stats.result = TIE;
        }
    }
    return;
}

int handle_game_over(GameLog &game, Connection &connect) {
    int status = 0;
    create_game_over_msg(connect.player1.msg, game.player1.stats);
    create_game_over_msg(connect.player2.msg, game.player2.stats);

    game.player1.error.type = send_msg(connect.player1.desc, connect.player1.msg);
    game.player2.error.type = send_msg(connect.player2.desc, connect.player2.msg);
    status = check_game_errors(game);
    return status;
}

int check_game_errors(GameLog &game) {
    int status = 0;
    if ( game.player1.error.type != OK ) status -= 1;
    if ( game.player2.error.type != OK ) status -= 2;

    switch (status) {
    case -3:
        game.player1.stats.result = TIE;
        game.player2.stats.result = TIE;
        break;
    case -2:
        game.player1.stats.result = WIN;
        game.player2.stats.result = LOSS;
        break;
    case -1:
        game.player1.stats.result = LOSS;
        game.player2.stats.result = WIN;
        break;
    }
    return status;
}

