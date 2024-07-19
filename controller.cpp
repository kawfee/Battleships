/**
 * @file controller.cpp
 * @authors Matthew Getgen & Joey Gorski
 * @brief Battleships Controller to run the Battleships Contest and Match
 * @date 2023-08-28
 */

#include <unistd.h>

#include "source/defines.h"
#include "source/display/options.h"
#include "source/logic/contest_logic.h"
#include "source/display/display_contest.h"


bool debug = false;

void print_error(const string error, const char *file_name, int line) {
    cerr << endl
         << conio::setTextStyle(conio::BOLD) << conio::fgColor(conio::RED)
         << file_name << " Error: " << conio::resetAll()
         << error << " (line: " << line << ")" << endl;
    return;
}

int main(int argc, char *argv[]) { 

    // check if -d or --debug was applied as an argument (debug mode)
    if ( argc >= 2 ) {
        for (int i = 0; i < argc; i++ ) {
            if ( strncmp(argv[i], "-d", 3) == 0 
              || strncmp(argv[i], "--debug", 8) == 0 ) {
                debug = true;
            }
        }
    }

    // initial setup.
    srand(getpid());
    signal(SIGTSTP, SIG_IGN);   // Ignore CTRL-Z

    const string 
        system_dir = get_current_dir_name(),
        socket_name = system_dir + SOCKET_NAME;

    int row;
    Options options = get_options(row, system_dir);
    Connection connect;
    ContestLog contest;
    MatchLog match;

    switch (options.runtime) {
    case RunMatch:
        signal(SIGINT, SIG_IGN);    // Ignore CTRL-C

        connect = create_socket(socket_name.c_str());

        match = run_match(connect, options.match_options, socket_name.c_str());

        close_sockets(connect);
        save_match_log(match, system_dir);
        
        signal(SIGINT, SIG_DFL);    // Listen to CTRL-C again

        display_match_with_options(match, options.match_options, row);
        break;
    case ReplayMatch:
        match = open_match_log(system_dir);
        display_match_with_options(match, options.match_options, row);
        break;
    case RunContest:
        signal(SIGINT, SIG_IGN); // Ignore CTRL-C

        connect = create_socket(socket_name.c_str());

        contest = run_contest(connect, options.contest_options, socket_name.c_str());

        close_sockets(connect);
        save_contest_log(contest, system_dir);

        signal(SIGINT, SIG_DFL); // Listen to CTRL-C again

        display_contest_with_options(contest, options.contest_options); 
        break;
    case ReplayContest:
        contest = open_contest_log(system_dir);
        display_contest_with_options(contest, options.contest_options);
        break;
    }


    cout << "\nGoodbye!\n" << flush;
    return 0;
}
