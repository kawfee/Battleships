/**
 * @file questions.cpp
 * @authors Matthew Getgen
 * @brief Runtime Questions for the Battleships Controller.
 * @date 2022-12-03
 */

#include "questions.h"
#include "conio.h"
#include <sys/stat.h>


/* ────────────────────────── *
 * GENERAL QUESTION FUNCTIONS *
 * ────────────────────────── */

void exit_abruptly() {
    cout << "\nExiting.\n" << flush;
    exit(-1);
}

bool is_int(string str) {
    int len = str.length();
    if ( len == 0 ) return false;
    for (int i = 0; i < len; i++) {
        if ( !isdigit(str[i]) ) return false;
    }
    return true;
}

bool is_float(string str) {
    int len = str.length();
    if ( len == 0 ) return false;
    int period = 0;
    for (int i = 0; i < len; i++) {
        if ( !isdigit(str[i]) ) {
            if ( str[i] == '.' && period == 0 ) {
                // all is good, as long as there is only one period.
                period = 1;
            } else return false;
        }
    }
    return true;
}

int get_runtime_type() {
    string input = "";

    // get runtime questions
    cout << "What would you like to run?\n";
    cout << "   [0]\tTest AI (default)\n"
         << "   [1]\tRun Contest\n"
         << "   [2]\tReplay Logs\n";
    cout << "Please enter your choice [(0)-2] " << flush;
    getline(cin, input);

    if ( input == "" || input == "0" ) return 0;
    else if ( input == "1" ) return 1;
    else if (input == "2" ) return 2;
    else return -1;
}

vector<tuple<string, string>> get_all_players(string &system_dir) {
    DIR *dir;
    dirent *dp;
    vector<tuple<string, string>> execs;

    string exec_dir = system_dir + EXEC_DIR;

    get_protected_players(exec_dir, execs);
    // make sure the directory can be opened
    if ( (dir = opendir(exec_dir.c_str())) == NULL ) {
        print_error("Cannot open AI_Files/ directory!", __FILE__, __LINE__);
        fprintf(stderr, "Directory path: %s\n", exec_dir.c_str());
        exit_abruptly();
    }

    struct stat sb;
    while ( (dp = readdir(dir)) != NULL ) {
        string file_name = exec_dir + dp->d_name;
        // if it is both a regular file, and an executable, add it.
        if ( stat(file_name.c_str(), &sb) == 0 && sb.st_mode & S_IXUSR && sb.st_mode & S_IFREG ) {
            execs.push_back(make_tuple(dp->d_name, exec_dir));
        }
    }
   
    // sort the players in alphabetical order
    sort(execs.begin(), execs.end(), sort_players_by_name);

    return execs;
}

void get_protected_players(string &exec_dir, vector<tuple<string, string>> &execs) {
    DIR *dir;
    dirent *dp;
    string protect_dir = exec_dir + PROTECT_DIR;

    // make sure the directory can be opened.
    if ( (dir = opendir(protect_dir.c_str())) == NULL ) {
        print_error("Cannot open AI_Files/protected/ directory!", __FILE__, __LINE__);
        fprintf(stderr, "Directory path: %s\n", protect_dir.c_str());
        exit_abruptly();
    }

    struct stat sb;
    // for each file in the directory
    while ( (dp = readdir(dir)) != NULL ) {
        string file_name = protect_dir + dp->d_name;
        // if it is both a regular file, and an executable, add it.
        if ( stat(file_name.c_str(), &sb) == 0 && sb.st_mode & S_IXUSR && sb.st_mode & S_IFREG ) {
            execs.push_back(make_tuple(dp->d_name, protect_dir));
        }
    }
    return;
}

bool sort_players_by_name(const tuple<string, string> &a, const tuple<string, string> &b) {
    return get<0>(a) < get<0>(b);
}

void ask_game_questions(int &board_size, int &num_games) {
    string input = "";
    cout << "\nHow many games should the AI play? (500) " << flush;
    getline(cin, input);
    if ( input != "" ) {
        if   ( !is_int(input) ) exit_abruptly();
        num_games = stoi(input);
        if ( num_games < 0 || num_games >= INT16_MAX) exit_abruptly();
    }
    cout << "\nWhat size of the board would you like? [3-(10)] " << flush;
    getline(cin, input);
    if ( input != "" ) {    // don't take the default, convert it
        if   ( !is_int(input) ) exit_abruptly();
        board_size = stoi(input);
        if ( board_size < 3 || board_size > 10 ) exit_abruptly();
    }
    return;
}

void ask_delay_question(int &delay) {
    float f_delay;
    string input = "";
    cout << "\nHow long should the delay between actions be? [(0.3), 0.5, 1, etc.] " << flush;
    getline(cin, input);
    if ( input != "" ) {    // don't take the default, convert it
        if ( !is_float(input) ) exit_abruptly();
        f_delay = stof(input);
        if ( f_delay < 0.0 ) exit_abruptly();
        delay = static_cast<int>(f_delay * 1000000);
    }
    return;
}


/* ────────────────────────── *
 * CONTEST QUESTION FUNCTIONS *
 * ────────────────────────── */

void ask_contest_display_questions(DisplayType &contest_display, int &delay) {
    string input = "";
    cout << "\nHow would you like to display the contest games?\n";
    cout << "   [0]\tDisplay games after contest finishes (default)\n"
         << "   [1]\tDisplay games while running\n"
         << "   [2]\tLog only, don't display\n";
    cout << "Please enter your choice [(0)-2] " << flush;
    getline(cin, input);

    if      ( input == "" || input == "0" ) contest_display = AFTER;
    else if ( input == "1" ) contest_display = DURING;
    else if ( input == "2" ) contest_display = NONE;
    else exit_abruptly();

    if ( contest_display != NONE ) ask_delay_question(delay);
    return;
}

void check_ai_list(vector<tuple<string, string>> &execs) {
    string input = "";

    // make sure there are AI to play the contest.
    int choice, num_execs = execs.size();
    if ( num_execs < 1 ) {
        cout << "\nNo AI to play a contest.\n";
        exit_abruptly();
    }

    do {
        cout << "\nThis is the list of AIs entered in the contest:\n" << flush;

        // display all AI in the contest
        for (int i = 0; i < num_execs; i++) {
            cout << "   [" << i << "]\t" << get<0>(execs.at(i)) << "\n" << flush;
        }

        // ask for any AI to remove from the contest
        cout << "Would you like to remove any from the contest? [Enter for no, AI # if so] " << flush;
        getline(cin, input);

        if ( input == "" ) break;   // no AI to remove, continue
        if ( !is_int(input) ) exit_abruptly();    // if they input non-int

        choice = stoi(input);
        if ( choice < 0 || choice >= num_execs ) exit_abruptly();   // input not in range

        execs.erase(execs.begin()+choice);
        num_execs = execs.size();

    } while ( input != "" && num_execs-1 >= 2 );
    return;
}

int ask_display_leaderboard() {
    string input = "";
    cout << "\nHow would you like to replay the contest?\n";
    cout << "   [0]\tReplay entire contest (default)\n"
         << "   [1]\tShow Final Leaderboard only\n";
    cout << "Please enter your choice [(0)-1] ";
    getline(cin, input);

    if ( input == "" || input == "0" ) return 0;
    else if ( input == "1" ) return 1;
    else return -1;
}


/* ──────────────────────── *
 * MATCH QUESTION FUNCTIONS *
 * ─────────────────────────*/

void ask_display_questions(DisplayType &display, int &delay, bool &step_through, bool is_replay) {
    int display_choice;
    string input = "";
    vector<string> options {
		"Last (default)",
		"All",
		"None",
		"1 win, loss, tie, or error (if present)",
	};
	if ( is_replay ) {
        options.push_back("Every Nth game");
    }

    cout << "\nHow would you like to display the games?\n";
    int num_options = options.size();
    for (int i = 0; i < num_options; i++) {
        printf("   [%d]\t%s\n", i, options.at(i).c_str());
    }
    printf("Please enter your choice [(0)-%d] ", num_options-1);
    getline(cin, input);

    if ( input == "" ) display = LAST;
    else {
        if ( !is_int(input) ) exit_abruptly();
        display_choice = stoi(input);
        if ( display_choice < 0 || display_choice >= num_options ) exit_abruptly();

        switch (display_choice) {
            case 0:
                display = LAST;
                break;
            case 1:
                display = ALL;
                break;
            case 2:
                display = NONE;
                break;
            case 3:
                display = EACH;
                break;
            case 4:
                display = INCREMENT;
                break;
            default:
                exit_abruptly();
        }
    }

    if ( display != NONE ) {
        if ( is_replay ) ask_step_through_question(step_through);
        if ( !step_through ) ask_delay_question(delay);
    }

    return;
}

void ask_step_through_question(bool &step_through) {
    string input = "";
    cout << "\nWould you like to step through the games to display? [y or (N)] ";
    getline(cin, input);
    transform(input.begin(), input.end(), input.begin(), ::tolower);

    if ( input == "y" || input == "yes" ) step_through = true;
    else step_through = false;
    return;
}

void ask_which_player(vector<tuple<string,string>> &execs, const char *word, string &ai_exec) {
    string input = "";
    int choice, num_execs;

    // make sure there are AI to play the match.
    num_execs = execs.size();
    if ( num_execs < 1 ) {
        cout << "\nNo AI to play a match.\n";
        exit_abruptly();
    }

    // word is either "first" or "second"
	printf("\nChoose the %s AI to battle from the following:\n", word);

    // display all AI options
    for (int i = 0; i < num_execs; i++) {
		printf("   [%d]\t%s\n", i, get<0>(execs.at(i)).c_str());
    }

    // ask for AI option input
	printf("Please enter your choice [0-%d] ", num_execs-1);
    getline(cin, input);

    if ( input == "" )    exit_abruptly();    // if they input empty string
    if ( !is_int(input) ) exit_abruptly();    // if they input non-int
    
    choice = stoi(input);
    if ( choice < 0 || choice >= num_execs ) exit_abruptly();   // if input is not in range

    ai_exec = get<1>(execs.at(choice)) + get<0>(execs.at(choice));  // else, store the executable with path <1> and file name <0>
    return;
}

int ask_incremental_question(int num_games) {
    vector<int> options;
    string input = "";
    int divisors[] = { 2, 5, 10, 25, 50};
    int size_of_divisors = sizeof(divisors)/sizeof(divisors[0]);
    
    for (int i = 0; i < size_of_divisors; i++) {
        int option = num_games/divisors[i];
        if ( option > 0 ) options.push_back(option);
    }
    
    int num_options = options.size();
    // if empty, don't display incrementally.
    if ( num_options <= 0 ) return -1;

    printf("\n\nChoose the rate to display the %d games:\n", num_games);
    for (int i = 0; i < num_options; i++) {
        printf("   [%d]\tEvery %d\n", i, options.at(i));
    }
    printf("Please enter your choice [0-%d] ", num_options-1);
    fflush(stdout);
    fflush(stdin);
    getline(cin, input);

    if ( !is_int(input) ) return -1;

    int option = stoi(input);
    return options.at(option);
}


/* ────────────────────── *
 * LOG QUESTION FUNCTIONS *
 * ────────────────────── */

int find_log_files(string &log_dir) {
    DIR *dir;
    dirent *dp;

    if ( (dir = opendir(log_dir.c_str())) == NULL ) {
        print_error("Cannot open log directory!", __FILE__, __LINE__);
        printf("Log directory path: %s\n", log_dir.c_str());
        return -1;
    }

    bool has_match = false, has_contest = false;
    while ( (dp = readdir(dir)) != NULL ) {
        if ( strncmp(dp->d_name, ".", 1) != 0 ) {
            if ( strncmp(dp->d_name, MATCH_LOG, sizeof(MATCH_LOG)) == 0 ) has_match = true;
            if ( strncmp(dp->d_name, CONTEST_LOG, sizeof(CONTEST_LOG)) == 0 ) has_contest = true;
        }
    }

    if ( has_match && has_contest ) {
        string input = "";

        cout << "\nWhich log would you like to replay?\n";
        cout << "   [0]\tMatch Log (default)\n"
             << "   [1]\tContest Log\n";
        cout << "Please enter your choice [(0)-1] ";
        getline(cin, input);

        if ( input == "" || input == "0" ) return 0;
        else if ( input == "1" ) return 1;
        else return -1;

    } else if ( has_match ) {
        cout << "\nMatch Log found!\n";
        return 0;
    } else if ( has_contest ) {
        cout << "\nContest Log found!\n";
        return 1;
    } else {
        cout << "\nNo Logs found!\n";
        return -1;
    }
}

