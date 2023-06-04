/**
 * @file game_logic.cpp
 * @author Matthew Getgen and Joey Gorski
 * @brief Battleships Game Logic for Battleships Contest
 * @date 2022-06-20
 */

#include "game_logic.h"
#include "logger.h"


/* ─────────────────────── *
 * CONTEST LOGIC FUNCTIONS *
 * ─────────────────────── */

// refactor of run contest
void run_contest_2(int board_size, int num_games, Connection connect, string socket_name, vector<tuple<string, string>> &execs, json &contest_log, int delay, bool display_now) {
    vector<ContestPlayer> all_players;      // used for the leaderboard.
    vector<ContestPlayer> alive_players;    // used to store players that are still in the contest, but aren't in a game currently.
    vector<ContestPlayer> dead_players;     // used to store players that no longer play the contest.
    
    make_contest_json(contest_log, board_size);

    // check players to make sure they are valid for the contest.
    initialize_players(execs, alive_players, dead_players, connect, socket_name);
    append_players_to_log(contest_log, alive_players);
    append_players_to_log(contest_log, dead_players);


    int alive_size = alive_players.size(), contest_round = 0;

    // main contest loop, ends when there is one person still standing
    while ( alive_size > 1 ) {
        append_new_contest_round(contest_log);

        vector<ContestMatch> matches;
        vector<json> match_logs; 
        
        // create all matches for this contest round.
        while ( alive_size > 1 ) {
            // get two random players to play a match.
            int idx1 = rand() % alive_size, idx2;
            do {
                idx2 = rand() % alive_size;
            } while ( idx1 == idx2 );

            ContestMatch match;
            json match_log;
            match.player1 = alive_players.at(idx1);
            match.player2 = alive_players.at(idx2);

            // remove the alive players from the back first
            if ( idx1 > idx2 ) {
                alive_players.erase(alive_players.begin() + idx1);
                alive_players.erase(alive_players.begin() + idx2);
            } else {
                alive_players.erase(alive_players.begin() + idx2);
                alive_players.erase(alive_players.begin() + idx1);
            }

            // add the future match to vectors to be handled later.
            matches.push_back(match);
            match_logs.push_back(match_log);
            alive_size = alive_players.size();
        }
        
        // play the matches, potentially in different threads in the future.
        int num_matches = matches.size();
        for (int i = 0; i < num_matches; i++) {
            json match_log = match_logs.at(i);
            ContestMatch cmatch = matches.at(i);
            Match match = handle_match_process(cmatch, match_log, board_size, num_games, connect, socket_name, delay, display_now);
            
            append_match_player_to_contest(cmatch.player1, match.player1);
            append_match_player_to_contest(cmatch.player2, match.player2);

            // check who won and change the values as a result.
            if ( match.player1.wins > match.player2.wins ) {    // player 1 won.
                cmatch.player2.lives--;
            } else if ( match.player1.wins < match.player2.wins ) { // player 2 won.
                cmatch.player1.lives--;
            } else {    // must have been a tie.
                cmatch.player1.lives--;
                cmatch.player2.lives--;
            }

            append_match_to_contest_log(contest_log, match_log, match, cmatch.player1, cmatch.player2, contest_round);

            if ( cmatch.player1.lives <= 0 ) dead_players.push_back(cmatch.player1);
            else                             alive_players.push_back(cmatch.player1);
            if ( cmatch.player2.lives <= 0 ) dead_players.push_back(cmatch.player2);
            else                             alive_players.push_back(cmatch.player2);
        }

        if ( display_now ) {
            all_players.insert(all_players.end(), alive_players.begin(), alive_players.end());
            all_players.insert(all_players.end(), dead_players.begin(), dead_players.end());
            display_leaderboard(all_players, false);    // display top 10 only.
            all_players.clear();

        }
        
        alive_size = alive_players.size();
        contest_round++;
    }

    // No matter what, display the final leaderboard
    all_players.insert(all_players.end(), alive_players.begin(), alive_players.end());
    all_players.insert(all_players.end(), dead_players.begin(), dead_players.end());
    display_leaderboard(all_players, true); // display all.

    if ( alive_size != 1 ) cout << "\nSomething unexpected happened... there are " << alive_size << " winners...\n";
    else                   cout << "\n" << all_players.at(0).ai_name << " won the contest!\n\n";
    
    return;
}

void initialize_players(vector<tuple<string, string>> &execs, vector<ContestPlayer> &alive_players, vector<ContestPlayer> &dead_players, Connection connect, string socket_name) {

    int num_execs = execs.size();
    ErrorNum status;
    
    // initialize the players in the contest
    for (int i = 0; i < num_execs; i++) {
        ContestPlayer player;
        player.exec = get<1>(execs.at(i)) + get<0>(execs.at(i));    // <1> is AI dir, <0> is file name
        memset(player.ai_name, 0, sizeof(player.ai_name));
        memset(player.author_name, 0, sizeof(player.author_name));
        player.lives = 3;
        player.total_wins = 0;
        player.total_losses = 0;
        player.total_ties = 0;
        player.idx = i;
        player.error = NO_ERR;
        status = wake_up_test(player, connect, socket_name);
        if ( status != NO_ERR ) {   // player failed basic test, remove them from contest.
            cout << "\nPlayer #" << i << " failed a basic test. They will not participate in the contest.\n\n";
            player.lives = 0;
            player.error = status;
            dead_players.push_back(player);
            continue;
        }
        alive_players.push_back(player);
    }

    return;
}

void run_contest(int board_size, int num_games, Connection connect, string socket_name, vector<tuple<string, string>> &execs, json &contest_log, int delay, bool display_now) {
    vector<ContestPlayer> all_players;
    vector<ContestPlayer> active_players;
    vector<ContestPlayer> passive_players;
    vector<ContestPlayer> dead_players;
    ErrorNum status;
    
    int num_execs = execs.size(), active_size, passive_size, contest_round = 0;

    make_contest_json(contest_log, board_size);

    // initialize the players in the contest
    for (int i = 0; i < num_execs; i++) {
        ContestPlayer player;
        player.exec = get<1>(execs.at(i)) + get<0>(execs.at(i));    // <1> is AI dir, <0> is file name
        memset(player.ai_name, 0, sizeof(player.ai_name));
        memset(player.author_name, 0, sizeof(player.author_name));
        player.lives = 3;
        player.total_wins = 0;
        player.total_losses = 0;
        player.total_ties = 0;
        player.idx = i;
        player.error = NO_ERR;
        status = wake_up_test(player, connect, socket_name);
        if ( status != NO_ERR ) {   // player failed basic test, remove them from contest.
            cout << "\nPlayer #" << i << " failed a basic test. They will not participate in the contest.\n\n";
            player.lives = 0;
            player.error = status;
            dead_players.push_back(player);
            continue;
        }
        append_player_to_log(contest_log, player);
        active_players.push_back(player);
    }

    active_size = active_players.size();
    passive_size = passive_players.size();

    // main contest loop, ends when there is one person still standing
    while ( (active_size + passive_size) > 1 ) {

        append_new_contest_round(contest_log);

        // play all active players
        while ( active_size > 1 ) {
            // get first two random players to play a match
            int idx1 = rand() % active_size, idx2;
            do {
                idx2 = rand() % active_size;
            } while ( idx1 == idx2 );

            // display in some way that the contest is still running...
            if ( !display_now ) cout << "\n" << active_players[idx1].ai_name << " VS " << active_players[idx2].ai_name << "\n";

            json match_log;

            // run the match
            Match match = run_match(board_size, num_games, connect, socket_name, active_players[idx1].exec, active_players[idx2].exec, match_log);
            merge_match_struct_to_log(match_log, match);

            if ( display_now ) {
                display_match(match_log, LAST, delay, false);
                sleep(SLEEP_TIME);
            }

            append_match_player_to_contest(active_players[idx1], match.player1);
            append_match_player_to_contest(active_players[idx2], match.player2);

            // check who won and change values as a result.
            if ( match.player1.wins > match.player2.wins ) {    // player 1 won
                active_players[idx2].lives--;
            } else if ( match.player1.wins < match.player2.wins ) { // player 2 won
                active_players[idx1].lives--;
            } else {    // must have been a tie
                // both lose a life
                active_players[idx1].lives--;
                active_players[idx2].lives--;
            }
            append_match_to_contest_log(contest_log, match_log, match, active_players[idx1], active_players[idx2], contest_round);

            // move the players
            move_player_by_state(&active_players, &passive_players, &dead_players, idx1);
            move_player_by_state(&active_players, &passive_players, &dead_players, idx2);

            // remove the active players for this contest round
            if ( idx1 > idx2 ) {
                active_players.erase(active_players.begin() + idx1);
                active_players.erase(active_players.begin() + idx2);
            } else {
                active_players.erase(active_players.begin() + idx2);
                active_players.erase(active_players.begin() + idx1);
            }
            active_size = active_players.size();
            passive_size = passive_players.size();
        }

        // move passive players into active
        active_players.insert(active_players.end(), passive_players.begin(), passive_players.end());
        passive_players.clear();

        // if display now, add to a total list of players and display the top 10 leaderboard.
        if ( display_now ) {
            all_players.insert(all_players.end(), active_players.begin(), active_players.end());
            all_players.insert(all_players.end(), dead_players.begin(), dead_players.end());

            display_leaderboard(all_players, false);    // display top 10 only.
            all_players.clear();
        }

        active_size = active_players.size();
        passive_size = passive_players.size();
        contest_round++;
    }

    // No matter what, display the final leaderboard
    all_players.insert(all_players.end(), active_players.begin(), active_players.end());
    all_players.insert(all_players.end(), passive_players.begin(), passive_players.end());
    all_players.insert(all_players.end(), dead_players.begin(), dead_players.end());

    display_leaderboard(all_players, true); // display all.
    all_players.clear();

    if ( active_size != 1 ) cout << "\nSomething unexpected happened... there are " << active_size << " winners...\n";
    else                    cout << "\n" << active_players[0].ai_name << " won the contest!\n\n";

    return;
}

ErrorNum wake_up_test(ContestPlayer &player, Connection connect, string socket_name) {
    ErrorNum status;
    char msg[MAX_MSG_SIZE];
    memset(msg, 0, MAX_MSG_SIZE);

    char *arg[] = { (char *)player.exec.c_str(), (char *)socket_name.c_str(), NULL };
    
    // start up the player
    status = run_player(arg[0], arg, &connect.pid1);
    if ( status != NO_ERR ) {
        if ( connect.pid1 != -1 ) kill_player(connect.pid1);
        return status;
    }

    // connect to the player
    status = accept_connection(connect.server_desc, &connect.player1_desc);
    if ( status != NO_ERR ) {
        kill_player(connect.pid1);
        close(connect.player1_desc);
        return status;
    }

    // recv message from player
    status = recv_message(connect.player1_desc, msg);
    if ( status != NO_ERR ) {
        kill_player(connect.pid1);
        close(connect.player1_desc);
        return status;
    }

    status = parse_hello_msg(msg, player.ai_name, player.author_name);
    kill_player(connect.pid1);
    close(connect.player1_desc);
    return status;
}

Match handle_match_process(ContestMatch &cmatch, json &match_log, int board_size, int num_games, Connection connect, string socket_name, int delay, bool display_now) {
    if ( !display_now ) cout << "\n" << cmatch.player1.ai_name << " VS " << cmatch.player2.ai_name << "\n";

    Match match = run_match(board_size, num_games, connect, socket_name, cmatch.player1.exec, cmatch.player2.exec, match_log);
    merge_match_struct_to_log(match_log, match);
    
    if ( display_now ) {
        display_match(match_log, LAST, delay, false);
        sleep(SLEEP_TIME);
    }

    return match;
}
        
void append_match_player_to_contest(ContestPlayer &cplayer, MatchPlayer mplayer) {
    // store total wins and losses
    cplayer.total_wins   += mplayer.wins;
    cplayer.total_losses += mplayer.losses;
    cplayer.total_ties   += mplayer.ties;

    // check if there were any errors, if so, boot them from the contest
    if ( mplayer.error != NO_ERR ) {  // player had an error, remove them from the contest
        cplayer.error = mplayer.error;
        cplayer.lives = 0;
    }
    return;
}

void move_player_by_state(vector<ContestPlayer> *active_players, vector<ContestPlayer> *passive_players, vector<ContestPlayer> *dead_players, int idx) {
    if ( active_players->at(idx).lives <= 0 ) { // player is dead
        dead_players->push_back(active_players->at(idx));
    } else {
        passive_players->push_back(active_players->at(idx));
    }
    return;
}


/* ───────────────────── *
 * MATCH LOGIC FUNCTIONS *
 * ───────────────────── */

Match run_match(int board_size, int num_games, Connection connect, string socket_name, string exec1, string exec2, json &match_log) {
    ErrorNum p1_status, p2_status;
    int status;

    make_match_json(match_log, board_size);

    Match match;
    memset(&match, 0, sizeof(Match));
    match.elapsed_time = 0;

    match.player1.error = NO_ERR;
    match.player2.error = NO_ERR;

    char msg1[MAX_MSG_SIZE];
    char msg2[MAX_MSG_SIZE];
    memset(msg1, 0, MAX_MSG_SIZE);
    memset(msg2, 0, MAX_MSG_SIZE);

    time_t start = time(NULL), end;

    /*      PLAYER STUFF        */
    char *arg1[] = { (char *)exec1.c_str(), (char *)socket_name.c_str(), NULL };
    char *arg2[] = { (char *)exec2.c_str(), (char *)socket_name.c_str(), NULL };

    // start and connect with player1
    p1_status = run_player(arg1[0], arg1, &connect.pid1);
    if ( p1_status != NO_ERR ) {
        if ( connect.pid1 != -1 ) kill_player(connect.pid1);
        match.player1.losses++;
        match.player2.wins++;
        match.player1.error = p1_status;
        return match;
    }
    p1_status = accept_connection(connect.server_desc, &connect.player1_desc);
    
    // start and connect with player2
    p2_status = run_player(arg2[0], arg2, &connect.pid2);
    if (p2_status != NO_ERR ) {
        kill_player(connect.pid1);
        close(connect.player1_desc);
        if ( connect.pid2 != -1 ) kill_player(connect.pid2);

        match.player1.wins++;
        match.player2.losses++;
        match.player2.error = p2_status;
        return match;
    }
    p2_status = accept_connection(connect.server_desc, &connect.player2_desc);

    // check connection status of both players
    status = check_return(&match, p1_status, p2_status);
    if (status) {   // players didn't connect, blame it on them (even if maybe it's your fault).
        kill_player(connect.pid1);
        kill_player(connect.pid2);
        close_player_sockets(&connect);
        return match;
    }

    /*      HELLO STUFF         */
    // recv hello message from player1 and player2
    p1_status = recv_message(connect.player1_desc, msg1);
    p2_status = recv_message(connect.player2_desc, msg2);
    status = check_return(&match, p1_status, p2_status);
    if (status) {
        kill_player(connect.pid1);
        kill_player(connect.pid2);
        close_player_sockets(&connect);
        return match;
    }
    
    // parse hello message from player1 and player2
    p1_status = parse_hello_msg(msg1, match.player1.ai_name, match.player1.author_name);
    p2_status = parse_hello_msg(msg2, match.player2.ai_name, match.player2.author_name);
    status = check_return(&match, p1_status, p2_status);
    if (status) {
        kill_player(connect.pid1);
        kill_player(connect.pid2);
        close_player_sockets(&connect);
        return match;
    }

    /*      MATCH STUFF         */
    // create setup_match message
    create_setup_match_msg(msg1, board_size, PLAYER_1);
    create_setup_match_msg(msg2, board_size, PLAYER_2);

    // send setup_match message
    p1_status = send_message(connect.player1_desc, msg1);
    p2_status = send_message(connect.player2_desc, msg2);
    status = check_return(&match, p1_status, p2_status);
    if (status) {
        kill_player(connect.pid1);
        kill_player(connect.pid2);
        close_player_sockets(&connect);
        return match;
    }

    /*      BOARD STUFF         */
    Board boards;
    boards.size = board_size;
    create_boards(boards);

    /*      SHIP STUFF          */
    int num_ships, max_ship_len, min_ship_len;
    // choose ship amount and sizes based on the size of the board
    switch (boards.size) {
        case 10:
            num_ships = 6;
            max_ship_len = 5;
            min_ship_len = 3;
            break;
        case 9:
            num_ships = 5;
            max_ship_len = 5;
            min_ship_len = 3;
            break;
        case 8:
            num_ships = 5;
            max_ship_len = 4;
            min_ship_len = 3;
            break;
        case 7:
            num_ships = 5;
            max_ship_len = 4;
            min_ship_len = 2;
            break;
        case 6:
            num_ships = 5;
            max_ship_len = 3;
            min_ship_len = 2;
            break;
        case 5:
            num_ships = 4;
            max_ship_len = 3;
            min_ship_len = 2;
            break;
        case 4:
            num_ships = 4;
            max_ship_len = 3;
            min_ship_len = 1;
            break;
        case 3:
            num_ships = 3;
            max_ship_len = 2;
            min_ship_len = 1;
            break;
        default:
            print_error(BOARD_SIZE_ERR, __FILE__, __LINE__);
            printf("Size of board received: %d\n", boards.size);
            delete_boards(boards);
            kill_player(connect.pid1);
            kill_player(connect.pid2);
            close_sockets(&connect);
            exit(-1);
    }

    /*      GAME STUFF          */
    for (int game = 0; game < num_games; game++) {
        json game_log = make_game_json();
        status = run_game(&match, game_log, &connect, boards, msg1, msg2, num_ships, max_ship_len, min_ship_len);
        match_log[GAMES_KEY].push_back(game_log);
        if (status) break;
    }

    /*      MATCH OVER STUFF        */
    create_match_over_msg(msg1);

    // send match_over message
    if ( status == -1 ) {           // player 1 failed, let player 2 know
        send_message(connect.player2_desc, msg1);
    } else if ( status == -2 ) {    // player 2 failed, let player 1 know
        send_message(connect.player1_desc, msg1);
    } else if ( status == 0 ) {     // neither failed, let both players know
        send_message(connect.player1_desc, msg1);
        send_message(connect.player2_desc, msg1);
    }

    end = time(NULL);
    match.elapsed_time = end-start;

    delete_boards(boards);
    kill_player(connect.pid1);
    kill_player(connect.pid2);
    close_player_sockets(&connect);

    return match;
}


/* ──────────────────── *
 * GAME LOGIC FUNCTIONS *
 * ──────────────────── */

int run_game(Match *match, json &game_log, Connection *connect, Board &boards, char *msg1, char *msg2, int num_ships, int max_ship_len, int min_ship_len) {
    ErrorNum p1_status, p2_status;
    int status;

    clear_boards(boards);

    /*      START GAME STUFF        */
    create_start_game_msg(msg1);

    // send start_round message
    p1_status = send_message(connect->player1_desc, msg1);
    p2_status = send_message(connect->player2_desc, msg1);
    status = check_return(match, p1_status, p2_status);
    if (status) return status;

    /*      SHIP STUFF       */
    // create ship vectors
    vector<Ship> ships1;
    vector<Ship> ships2;
    
    int random, ship_length;
    for (int i = 0; i < num_ships; i++) {
        Ship ship1, ship2;

        // store ships into vector
        ships1.push_back(ship1);
        ships2.push_back(ship2);

        // make a random ship length
        random = (rand() % 6);
        switch (random) {
            case 5:
                ship_length = max_ship_len;
                break;
            case 4:
            case 3:
                ship_length = (int)((max_ship_len + min_ship_len) / 2); // case 0-3 can be the same number in some ranges, it all depends.
                break;
            case 2:
            case 1:
            case 0:
            default:
                ship_length = min_ship_len;
                break;
        }

        // handle placing the ships
        status = handle_ship_placement(match, game_log, connect, boards, msg1, msg2, ships1[i], ships2[i], ship_length);
        if (status) return status;
    }

    /*      SHOT STUFF       */
    // go round by round
    for (int i = 0; i < MAX_ROUNDS; i++) {    
        status = handle_shot_placement(match, game_log, connect, boards, msg1, msg2);
        if (status) return status;

        // check all ships for player1
        status = check_ships_alive(match, game_log, connect, msg1, boards.board1, ships1, PLAYER_1);
        if (status) return status;

        // check all ships for player2
        status = check_ships_alive(match, game_log, connect, msg2, boards.board2, ships2, PLAYER_2);
        if (status) return status;

        // someone has won, find out!
        if ( ships1.empty() || ships2.empty() ) break;
    }

    // calculate winner of the game based on total ships dead
    calculate_winner(match, game_log, ships1, ships2);
    send_game_over_messages(connect, msg1);

    return 0;
}

int handle_ship_placement(Match *match, json &game_log, Connection *connect, Board &boards, char *msg1, char *msg2, Ship &ship1, Ship &ship2, int ship_length) {
    ErrorNum p1_status, p2_status;
    int status;

    create_place_ship_msg(msg1, ship_length);

    // send place_ship message
    p1_status = send_message(connect->player1_desc, msg1);
    p2_status = send_message(connect->player2_desc, msg1);
    status = check_return(match, p1_status, p2_status);
    if (status) return status;  // match-level error

    // recv ship_placed message from player1 and player2
    p1_status = recv_message(connect->player1_desc, msg1);
    p2_status = recv_message(connect->player2_desc, msg2);
    status = check_return(match, p1_status, p2_status);
    if (status) return status;  // player unresponsive

    // parse ship_placed messages.
    p1_status = parse_ship_placed_msg(msg1, ship1);
    p2_status = parse_ship_placed_msg(msg2, ship2);
    status = check_return(match, p1_status, p2_status);
    if (status) return status;  // invalid response

    // validate ship was placed in a good spot.
    p1_status = validate_ship_placement(boards.board1, boards.size, ship1, ship_length);
    p2_status = validate_ship_placement(boards.board2, boards.size, ship2, ship_length);
    status = check_return(match, p1_status, p2_status);
    if (status) return status;  // invalid ship placement

    // store ships into boards
    store_ship_to_board(ship1, boards.board1, SHIP);
    store_ship_to_board(ship2, boards.board2, SHIP);
    
    // store ships into JSON
    append_ship_to_json(game_log, ship1, PLAYER_1);
    append_ship_to_json(game_log, ship2, PLAYER_2);

    return 0;
}

ErrorNum validate_ship_placement(char **board, int board_size, Ship &ship, int ship_length) {
    enum ShipError {
        WRONG_LEN, NOT_ON_BOARD, INTERSECT,
    };

    try {
        if ( ship.len != ship_length ) throw WRONG_LEN;
        // check lower bound
        if ( ship.row < 0 || ship.col < 0 ) throw NOT_ON_BOARD;

        // check upper bound
        if ( (ship.dir == HORIZONTAL && ship.col + (ship.len-1) >= board_size)
          || (ship.dir == VERTICAL   && ship.row + (ship.len-1) >= board_size) ) throw NOT_ON_BOARD;

        // check if placed on empty water
        for (int i = 0; i < ship.len; i++) {
            if ( (ship.dir == HORIZONTAL && board[ship.row][ship.col+i] != WATER)
              || (ship.dir == VERTICAL   && board[ship.row+i][ship.col] != WATER) ) throw INTERSECT;
        }

    } catch (ShipError ship_error) {
        print_error(SHIP_PLACE_ERR, __FILE__, __LINE__);

        if ( ship_error == WRONG_LEN ) printf("Ship returned with length: %d, but expected length: %d\n", ship.len, ship_length);
        else if ( ship_error == NOT_ON_BOARD ) printf("Ship returned with\n\trow: %d\n\tcol: %d\n\tlength: %d\n\tdirection: %c\ndoesn't fit onto the board!\n", ship.row, ship.col, ship.len, (char)ship.dir);
        else if ( ship_error == INTERSECT ) printf("Ship returned with\n\trow: %d\n\tcol: %d\n\tlength: %d\n\tdirection: %c\nintersects with a ship already on the board!\n", ship.row, ship.col, ship.len, (char)ship.dir);
        return BAD_SHIP;
    }
    return NO_ERR;
}

int handle_shot_placement(Match *match, json &game_log, Connection *connect, Board &boards, char *msg1, char *msg2) {
    ErrorNum p1_status, p2_status;
    int status;

    Shot shot1, shot2;

    create_take_shot_msg(msg1);

    // send take_shot message
    p1_status = send_message(connect->player1_desc, msg1);
    p2_status = send_message(connect->player2_desc, msg1);
    status = check_return(match, p1_status, p2_status);
    if (status) return status;  // match-level error

    // recv shot_taken message from player1 and player2
    p1_status = recv_message(connect->player1_desc, msg1);
    p2_status = recv_message(connect->player2_desc, msg2);
    status = check_return(match, p1_status, p2_status);
    if (status) return status;  // player unresponsive

    // parse the shot message that was taken
    p1_status = parse_shot_taken_msg(msg1, shot1);
    p2_status = parse_shot_taken_msg(msg2, shot2);
    status = check_return(match, p1_status, p2_status);
    if (status) return status;  // invalid response

    // validate shot was a valid shot.
    p1_status = validate_shot_placement(boards.size, shot1);
    p2_status = validate_shot_placement(boards.size, shot2);
    status = check_return(match, p1_status, p2_status);
    if (status) return status;  // invalid shot placement

    // get shot1 value
    get_shot_value(boards.board2, shot1);
    // store shot1 in JSON value
    append_shot_to_json(game_log, shot1, PLAYER_1);
    // store shot1 value
    store_shot_to_board(shot1, boards.board2);

    // get shot2 value
    get_shot_value(boards.board1, shot2);
    // store shot2 into JSON value
    append_shot_to_json(game_log, shot2, PLAYER_2);
    // store shot2 value
    store_shot_to_board(shot2, boards.board1);

    create_shot_return_msg(msg1, PLAYER_1, shot1);
    create_shot_return_msg(msg2, PLAYER_2, shot2);

    // send shot_return message 1 to both players.
    p1_status = send_message(connect->player1_desc, msg1);
    p2_status = send_message(connect->player2_desc, msg1);
    status = check_return(match, p1_status, p2_status);
    if (status) return status;  // match-level error

    // send shot_return message 2 to both players.
    p1_status = send_message(connect->player1_desc, msg2);
    p2_status = send_message(connect->player2_desc, msg2);
    status = check_return(match, p1_status, p2_status);
    if (status) return status;  // match-level error
    
    return 0;
}

ErrorNum validate_shot_placement(int board_size, Shot &shot) {
    // check upper and lower bound
    if ( shot.row < 0 || shot.row >= board_size
      || shot.col < 0 || shot.col >= board_size ) {
        print_error(SHOT_PLACE_ERR, __FILE__, __LINE__);
        printf("Shot returned with\n\trow: %d\n\tcol: %d\ndoesn't fit onto the board!\n", shot.row, shot.col);
        return BAD_SHOT;
    }
    return NO_ERR;
}

void get_shot_value(char **opponent_board, Shot &shot) {
    BoardValue value = (BoardValue)opponent_board[shot.row][shot.col];
    // convert value to new value based on current board value
    if      ( value == WATER ) shot.value = MISS;
    else if ( value == SHIP  ) shot.value = HIT;
    else if ( value == HIT   ) shot.value = DUPLICATE_HIT;
    else if ( value == MISS  ) shot.value = DUPLICATE_MISS;
    else if ( value == KILL  ) shot.value = DUPLICATE_KILL;
    else if ( value == DUPLICATE_HIT  ) shot.value = DUPLICATE_HIT;
    else if ( value == DUPLICATE_MISS ) shot.value = DUPLICATE_MISS;
    else if ( value == DUPLICATE_KILL ) shot.value = DUPLICATE_KILL;
    return;
}


/* ─────────────────── *
 * GAME OVER FUNCTIONS *
 * ─────────────────── */

int check_ships_alive(Match *match, json &game_log, Connection *connect, char *msg, char **board, vector<Ship> &ships, PlayerNum player) {
    ErrorNum p1_status, p2_status;
    int status;
    int row = 0, col = 0, hit_count;

    // go through all ships available.
    int num_ships = (int)ships.size();
    for (int ship = 0; ship < num_ships; ship++) {
        hit_count = 0;
        // go through the length of the ship
        row = ships.at(ship).row;
        col = ships.at(ship).col;
        for (int len = 0; len < ships.at(ship).len; len++) {

            if      ( ships.at(ship).dir == HORIZONTAL ) col = ships.at(ship).col + len;
            else if ( ships.at(ship).dir == VERTICAL )   row = ships.at(ship).row + len;

            // check hits on the ships
            if ( board[row][col] == HIT || board[row][col] == DUPLICATE_HIT ) hit_count++;
        }
        // if each location on the ship is hit
        if ( hit_count == ships.at(ship).len ) {
            create_ship_dead_msg(msg, player, ships.at(ship));

            // send ship_dead message to both players.
            p1_status = send_message(connect->player1_desc, msg);
            p2_status = send_message(connect->player2_desc, msg);
            status = check_return(match, p1_status, p2_status);
            if (status) return status;  // match-level error

            store_dead_ship_to_json(game_log, ships.at(ship), player);

            // store dead ship
            store_ship_to_board(ships.at(ship), board, KILL);
            
            // remove ship from ship vector.
            ships.erase(ships.begin()+ship);
            break;
        }
    }
    return 0;
}

int check_return(Match *match, ErrorNum p1_status, ErrorNum p2_status) {
    int status = 0;
    if ( p1_status != NO_ERR ) {        // player1 is wrong
        if ( p2_status != NO_ERR ) {    // both players are wrong, tie
            match->player1.ties++;
            match->player2.ties++;
            status = -3;
        } else {                        // only player1 is wrong
            match->player1.losses++;
            match->player2.wins++;
            status = -1;
        }
    } else if ( p2_status != NO_ERR ) { // only player2 is wrong
        match->player1.wins++;
        match->player2.losses++;
        status = -2;
    }
    match->player1.error = p1_status;
    match->player2.error = p2_status;
    return status;
}

void calculate_winner(Match *match, json &game_log, vector<Ship> &ships1, vector<Ship> &ships2) {
    // check who has all dead ships
    GameResult outcome1, outcome2;
    if ( ships2.empty() ) {
        if ( ships1.empty() ) {    // tie (both ships dead)
            match->player1.ties++;
            match->player2.ties++;
            outcome1 = TIE, outcome2 = TIE;
        } else {                    // player1 won (all enemy ships dead)
            match->player1.wins++;
            match->player2.losses++;
            outcome1 = WIN, outcome2 = LOSS;
        }
    } else if ( ships1.empty() ) { // player2 won (all enemy ships dead)
        match->player1.losses++;
        match->player2.wins++;
        outcome1 = LOSS, outcome2 = WIN;
    } else {                            // none of the ships are empty, check by total ships alive
        // check who has the least ships
        int size1 = ships1.size();
        int size2 = ships2.size();
        if ( size1 > size2 ) {          // player1 won (more alive ships)
            match->player1.wins++;
            match->player2.losses++;
            outcome1 = WIN, outcome2 = LOSS;
        } else if ( size1 < size2 ) {   // player2 won (more alive ships)
            match->player1.losses++;
            match->player2.wins++;
            outcome1 = LOSS, outcome2 = WIN;
        } else {    // tie (both have the same amount of ships alive)
            match->player1.ties++;
            match->player2.ties++;
            outcome1 = TIE, outcome2 = TIE;
        }
    }
    store_game_result_to_json(game_log, outcome1, outcome2);
    return;
}

void send_game_over_messages(Connection *connect, char *msg) {
    create_game_over_msg(msg);
    // at this point, checking return values doen't matter.

    // send game_over message
    send_message(connect->player1_desc, msg);
    send_message(connect->player2_desc, msg);
    return;
}

