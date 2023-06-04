/**
 * @file controller.cpp
 * @authors Matthew Getgen and Joey Gorski
 * @brief Battleships Controller to run the Battleships Contest
 * @date 2022-04-19
 */

#include "source/defines.h"
#include "source/game_logic.h"

#define SOCKET_NAME "/battleships.socket"

bool debug = false;

void print_error(const char *error, const char *file_name, int line) {
    fprintf(stderr, "\n%s Error: %s (line: %d)\n", file_name, error, line);
    return;
}

Connection create_socket(const char *socket_name) {
    Connection connect;
    
    // create and bind the socket
    if ( bind_socket(&connect, (char *)socket_name) != NO_ERR ) {
        close_sockets(&connect);
        exit_abruptly();   // critical failure, not a complete match.
    }
    return connect;
}

int main(int argc, char *argv[]) {

    // handle signals
    signal(SIGINT, SIG_IGN);    // CTRL-C
    signal(SIGTSTP, SIG_IGN);   // CTRL-Z
    cout << clrscr();

    // check if a 1 was applied as an argument (debug mode)
    if ( argc == 2 && strncmp(argv[1], "1", 2) == 0 ) {
        debug = true;
        cout << "\n\t~~debug mode~~\n";
    }
    cout << gotoRowCol(2, 1) << "Welcome to the Battleships AI Contest and Tester!\n\n";
    
    string system_dir, ai_dir, log_dir, socket_name, exec1, exec2;

    int board_size = 10, num_games = 500;
    DisplayType display = NONE, contest_display = DURING;
    int delay = 300000;
    bool step_through = false;
    
    srand(time(NULL));

    // get the absolute path for the program
    system_dir = get_current_dir_name();
    log_dir = system_dir + LOGS_DIR;
    socket_name = system_dir + SOCKET_NAME;


    int runtime = get_runtime_type();
    
    // test AI
    if ( runtime == 0 ) {
        ask_game_questions(board_size, num_games);
        ask_display_questions(display, delay, step_through, false);

        vector<tuple<string, string>> execs = get_all_players(system_dir);
        ask_which_player(execs, "first", exec1);
        ask_which_player(execs, "second", exec2);

        Connection connect = create_socket(socket_name.c_str());

        json match_log;

        Match match = run_match(board_size, num_games, connect,  socket_name, exec1, exec2, match_log);
        close_sockets(&connect);

        merge_match_struct_to_log(match_log, match);
        log_match(match_log, log_dir);
        
        display_match(match_log, display, delay, step_through);

    // run contest
    } else if ( runtime == 1 ) {
        ask_game_questions(board_size, num_games);
        ask_contest_display_questions(contest_display, delay);

        vector<tuple<string, string>> execs = get_all_players(system_dir);
        check_ai_list(execs);

        Connection connect = create_socket(socket_name.c_str());

        json contest_log;

        if ( contest_display == DURING ) run_contest(board_size, num_games, connect, socket_name, execs, contest_log, delay, true);
        else                             run_contest(board_size, num_games, connect, socket_name, execs, contest_log, delay, false);
        close_sockets(&connect);
        
        log_contest(contest_log, log_dir);
        
        // display the contest at the end.
        if ( contest_display == AFTER ) display_contest(contest_log, delay);

    // replay logs
    } else if ( runtime == 2 ) {
        int log_type = find_log_files(log_dir);

        if ( log_type == -1 ) exit_abruptly();
        else if ( log_type == 0 ) {  // match log
            ask_display_questions(display, delay, step_through, true);
            replay_match(display, delay, log_dir, step_through);

        }  else if ( log_type == 1 ) {  // contest log
            int rv = ask_display_leaderboard();
            bool display_leaderboard = false;
            if ( rv == -1 ) exit_abruptly();
            else if ( rv == 0 ) display_leaderboard = false;
            else if ( rv == 1 ) display_leaderboard = true;
            if ( !display_leaderboard ) ask_delay_question(delay);
            replay_contest(delay, log_dir, display_leaderboard);
        }
    } else exit_abruptly();
    
    cout << "\nGoodbye!\n" << flush;
    return 0;
}

