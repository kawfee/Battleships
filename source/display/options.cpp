/**
 * @file options.cpp
 * @authors Matthew Getgen
 * @brief Runtime Options for the Battleships Controller.
 * @date 2023-08-31
 */

#include "options.h"


Options get_options(int &row, const string &system_dir) {
    Options options;
    const string options_file = system_dir + OPTIONS_FILE;
    row = print_start();

    json j_options = read_options_file(options_file);

    options.runtime = get_runtime(row, j_options);

    switch (options.runtime) {
    case RunMatch:
        options.match_options = get_match_options(row, system_dir, j_options[MATCH_OPTIONS_KEY]);
        break;
    case ReplayMatch:
        options.match_options = get_match_replay_options(row, system_dir, j_options[MATCH_OPTIONS_KEY]);
        break;
    case RunContest:
        options.contest_options = get_contest_options(row, system_dir, j_options[CONTEST_OPTIONS_KEY]);
        break;
    case ReplayContest:
        options.contest_options = get_contest_replay_options(row, system_dir, j_options[MATCH_OPTIONS_KEY]);
        break;
    }

    return options;
}

int print_start() {
    int row = 1;
    cout << conio::clearScreen();
    cout << conio::gotoRowCol(row, 1) << "Welcome to the Battleships AI Contest and Tester!";
    if (debug) {
        row++;
        cout << conio::gotoRowCol(row, 16) << conio::fgColor(conio::RED) << conio::setTextStyle(conio::BOLD) 
             << "## debug mode ##" << conio::resetAll();
    }

    row += 2;
    cout << conio::gotoRowCol(row, 1) << "Anything in \"[]\" is the default value. Hit enter to use the default." << flush;
    row += 2;
    return row;
}


/* ────────────────────────── *
 * RUNTIME QUESTION FUNCTIONS *
 * ────────────────────────── */

Runtime get_runtime(int &row, json &j_options) {
    Runtime runtime = RunMatch;
    string input = get_json_options_choice(j_options, RUNTIME_KEY);

    cout << conio::gotoRowCol(row, 1) 
         << "What would you like to run?\n"
         << "   [0]\tTest AI\n"
         << "    1\tRun Contest\n"
         << "    2\tReplay Test\n"
         << "    3\tReplay Contest\n"
         << "Please enter your choice: " << flush;
    if ( input == "" ) getline(cin, input);

    if (input == "" || input == "0" ) runtime = RunMatch;
    else if ( input == "1" ) runtime = RunContest;
    else if ( input == "2" ) runtime = ReplayMatch;
    else if ( input == "3" ) runtime = ReplayContest;
    else exit_abruptly();

    // clear question because it's clean and cool like that.
    cout << conio::gotoRowCol(row, 1) << conio::clearRow()
         << conio::gotoNextRow() << conio::clearRow()
         << conio::gotoNextRow() << conio::clearRow()
         << conio::gotoNextRow() << conio::clearRow()
         << conio::gotoNextRow() << conio::clearRow()
         << conio::gotoNextRow() << conio::clearRow();
    cout << conio::gotoRowCol(row, 1) << conio::setTextStyle(conio::BOLD);
    switch (runtime) {
    case RunMatch:
        cout << "Testing AI";
        break;
    case RunContest:
        cout << "Running Contest";
        break;
    case ReplayMatch:
        cout << "Replaying Match";
        break;
    case ReplayContest:
        cout << "Replaying Contest";
        break;
    }
    cout << conio::resetAll() << flush;
    row++;
    return runtime;
}


/* ──────────────────────── *
 * MATCH QUESTION FUNCTIONS *
 * ──────────────────────── */

MatchOptions get_match_options(int &row, const string &system_dir, json &j_options) {
    MatchOptions options;

    options.board_size = get_board_size(row, j_options);
    options.num_games = get_num_games(row, j_options);
    options.display_type = get_match_display_type(row, j_options);
    options.step_through = false;
    if ( options.display_type != NONE ) {
        options.step_through = get_step_through(row, j_options);
    }
    if ( options.display_type != NONE && !options.step_through ) {
        options.delay_time = get_delay_time(row, j_options);
    }

    vector<Executable> execs = get_all_execs(system_dir);
    options.exec1 = get_match_player(row, execs, PLAYER_1, j_options);
    options.exec2 = get_match_player(row, execs, PLAYER_2, j_options);
    
    return options;
}

MatchOptions get_match_replay_options(int &row, const string &system_dir, json &j_options) {
    MatchOptions options;

    const string match_log_file = system_dir + LOGS_DIR + MATCH_LOG;
    ifstream match_file(match_log_file.c_str());
    if ( !match_file.is_open() || match_file.fail() ) {
        print_error("Couldn't find match_log.json file!", __FILE__, __LINE__);
        exit_abruptly();
    }
    match_file.close();

    options.display_type = get_match_display_type(row, j_options);
    if ( options.display_type != NONE ) {
        options.step_through = get_step_through(row, j_options);
    } else {
        options.step_through = false;
    }
    if ( options.display_type != NONE && !options.step_through ) {
        options.delay_time = get_delay_time(row, j_options);
    }

    return options;
}

int get_board_size(int &row, json &j_options) {
    int board_size = 10;

    string input = get_json_options_choice(j_options, OPTIONS_BOARD_KEY);
    cout << conio::gotoRowCol(row, 1) << "Please enter a board size between 3-[10]: " << flush;
    if ( input == "" ) getline(cin, input);

    if ( input != "" ) {
        board_size = check_valid_int(input, 3, 10);
        if ( board_size == -1 ) exit_abruptly();
    }
    cout << conio::gotoRowCol(row, 1) << conio::clearRow() 
         << conio::setTextStyle(conio::BOLD)
         << "Board Size: " << board_size
         << conio::resetAll() << flush;
    row++;

    return board_size;
}

int get_num_games(int &row, json &j_options) {
    int num_games = 500;

    string input = get_json_options_choice(j_options, GAMES_PER_MATCH_KEY);
    cout << conio::gotoRowCol(row, 1) << "Please enter the number of games per match [500]: " << flush;
    if ( input == "" ) getline(cin, input);

    if ( input != "" ) {
        num_games = check_valid_int(input, 1, 10000);
        if ( num_games == -1 ) exit_abruptly();
    }
    cout << conio::gotoRowCol(row, 1) << conio::clearRow() 
         << conio::setTextStyle(conio::BOLD)
         << "Games per Match: " << num_games
         << conio::resetAll() << flush;
    row++;

    return num_games;
}

MatchDisplayType get_match_display_type(int &row, json &j_options) {
    MatchDisplayType type = LAST;

    string input = get_json_options_choice(j_options, DISPLAY_TYPE_KEY);
    cout << conio::gotoRowCol(row, 1) 
         << "How would you like to display the match?\n"
         << "   [0]\tLast Game\n"
         << "    1\tAll Games\n"
         << "    2\t1 win, loss, tie, or error (if present)\n"
         << "    3\tDisplay every Nth game\n"
         << "    4\tChoose from all games played\n"
         << "    5\tNone (stats only)\n"
         << "Please enter your choice: " << flush;
    if ( input == "" ) getline(cin, input);

    if (input == "" || input == "0" ) type = LAST;
    else if ( input == "1" ) type = ALL;
    else if ( input == "2" ) type = EACH_TYPE;
    else if ( input == "3" ) type = INCREMENT;
    else if ( input == "4" ) type = CHOICE;
    else if ( input == "5" ) type = NONE;
    else exit_abruptly();

    // clear question because it's clean and cool like that.
    cout << conio::gotoRowCol(row, 1) << conio::clearRow()
         << conio::gotoNextRow() << conio::clearRow()
         << conio::gotoNextRow() << conio::clearRow()
         << conio::gotoNextRow() << conio::clearRow()
         << conio::gotoNextRow() << conio::clearRow()
         << conio::gotoNextRow() << conio::clearRow()
         << conio::gotoNextRow() << conio::clearRow()
         << conio::gotoNextRow() << conio::clearRow();
    cout << conio::gotoRowCol(row, 1) << conio::setTextStyle(conio::BOLD);
    switch (type) {
    case LAST:
        cout << "Displaying last game";
        break;
    case ALL:
        cout << "Displaying all games";
        break;
    case EACH_TYPE:
        cout << "Displaying 1 of each type";
        break;
    case INCREMENT:
        cout << "Displaying every Nth game";
        break;
    case CHOICE:
        cout << "Choose game(s) to display";
        break;
    case NONE:
        cout << "Displaying stats only";
        break;
    }
    cout << conio::resetAll() << flush;
    row++;

    return type;
}

bool get_step_through(int &row, json &j_options) {
    bool step_through = false;

    string input = get_json_options_choice(j_options, STEP_THROUGH_KEY);
    cout << conio::gotoRowCol(row, 1) << "Would you like to step through the games? y/[N] ";
    if ( input == "" ) getline(cin, input);

    if ( input == "" || input.at(0) == 'n' || input.at(0) == 'N' ) {
        step_through = false;
    } else if ( input.at(0) == 'y' || input.at(0) == 'Y' ) {
        step_through = true;
    } else {
        exit_abruptly();
    }

    cout << conio::gotoRowCol(row, 1) << conio::clearRow()
         << conio::setTextStyle(conio::BOLD);
    if ( step_through ) {
        cout << "Stepping through display";
        row++;
    }
    cout << conio::resetAll() << flush;

    return step_through;
}

int get_delay_time(int &row, json &j_options) {
    // Delay is in microseconds, so 0.3 seconds.
    int delay_time = 300000;
    // used to request, and display
    float float_delay = 0.3;

    string input = get_json_options_choice(j_options, DELAY_TIME_KEY);
    cout << conio::gotoRowCol(row, 1) << "Please enter the delay time (in seconds) to display actions [0.3]: " << flush;
    if ( input == "" ) getline(cin, input);

    if ( input != "" ) {
        float_delay = check_valid_float(input, 0.0, 100.0);
        if ( float_delay < 0.0 ) exit_abruptly();
        delay_time = static_cast<int>(float_delay * 1000000);
    }
    cout << conio::gotoRowCol(row, 1) << conio::clearRow()
         << conio::setTextStyle(conio::BOLD)
         << "Delay Time: " << float_delay;
    if ( float_delay == 1.0 ) cout << " second";
    else cout << " seconds";
    cout << conio::resetAll() << flush;
    row++;

    return delay_time;
}

Executable get_match_player(int &row, vector<Executable> &execs, PlayerNum player, json &j_options) {
    Executable chosen;

    string input = "";
    if ( player == PLAYER_1 ) input = get_json_options_choice(j_options, OPTIONS_P1_KEY);
    else                      input = get_json_options_choice(j_options, OPTIONS_P2_KEY);

    size_t num_execs = execs.size();
    if ( num_execs == 0 ) {
        cout << "\nNo AI found.\n";
        exit_abruptly();
    }
    string word = "";
    if ( player == PLAYER_1 ) word = "first";
    else                      word = "second";
    cout << conio::gotoRowCol(row, 1) 
         << "Choose the " << word << " AI to test";
    for (int i = 0; i < (int)num_execs; i++) {
        cout << conio::gotoRowCol(row+i+1, 1) 
             << "    " << i << "\t" << execs.at(i).file_name;
    };
    cout << conio::gotoRowCol(row+num_execs+1, 1)
         << "Please enter a choice between 0-" << num_execs-1 << ": ";
    
    if ( input == "" ) getline(cin, input);

    if ( input == "" ) exit_abruptly();
    int choice = check_valid_int(input, 0, num_execs-1);
    if ( choice == -1 ) exit_abruptly();

    chosen = execs.at(choice);

    cout << conio::gotoRowCol(row, 1);
    for (int i = 0; i < (int)num_execs+2; i++) {
        cout << conio::clearRow() << conio::gotoNextRow();
    }
    cout << conio::gotoRowCol(row, 1)
         << conio::setTextStyle(conio::BOLD)
         << "Player " << player << ": #" << choice << " " << chosen.file_name
         << conio::resetAll() << flush;
    row++;

    return chosen;
}

/* ────────────────────────── *
 * CONTEST QUESTION FUNCTIONS *
 * ────────────────────────── */

ContestOptions get_contest_options(int &row, const string &system_dir, json &j_options) {
    ContestOptions options;

    options.board_size = get_board_size(row, j_options);
    options.num_games = get_num_games(row, j_options);
    options.display_type = get_contest_display_type(row, j_options);
    if ( options.display_type == NORMAL ) {
        options.delay_time = get_delay_time(row, j_options);
    }

    options.execs = get_all_execs(system_dir);
    ask_to_remove_player(row, options.execs);

    return options;
}

ContestOptions get_contest_replay_options(int &row, const string &system_dir, json &j_options) {
    ContestOptions options;

    const string contest_log_file = system_dir + LOGS_DIR + CONTEST_LOG;
    ifstream contest_file(contest_log_file.c_str());
    if ( !contest_file.is_open() || contest_file.fail() ) {
        print_error("Couldn't find contest_log.json file!", __FILE__, __LINE__);
        exit_abruptly();
    }
    contest_file.close();

    options.display_type = get_contest_display_type(row, j_options);
    if ( options.display_type == NORMAL ) {
        options.delay_time = get_delay_time(row, j_options);
    }

    return options;
}

ContestDisplayType get_contest_display_type(int &row, json &j_options) {
    ContestDisplayType type = NORMAL;

    string input = get_json_options_choice(j_options, DISPLAY_TYPE_KEY);
    cout << conio::gotoRowCol(row, 1) 
         << "How would you like to display the contest?\n"
         << "   [0]\tDisplay all matches, round results, and the final result\n"
         << "    1\tDisplay only the round results and final results\n"
         << "    2\tDisplay only the final result\n"
         << "Please enter your choice: " << flush;
    if ( input == "" ) getline(cin, input);

    if (input == "" || input == "0" ) type = NORMAL;
    else if ( input == "1" ) type = ROUNDS;
    else if ( input == "2" ) type = FINAL;
    else exit_abruptly();

    // clear question because it's clean and cool like that.
    cout << conio::gotoRowCol(row, 1) << conio::clearRow()
         << conio::gotoNextRow() << conio::clearRow()
         << conio::gotoNextRow() << conio::clearRow()
         << conio::gotoNextRow() << conio::clearRow()
         << conio::gotoNextRow() << conio::clearRow();
    cout << conio::gotoRowCol(row, 1) << conio::setTextStyle(conio::BOLD);
    switch (type) {
    case NORMAL:
        cout << "Displaying matches, rounds, and final";
        break;
    case ROUNDS:
        cout << "Displaying round results and final";
        break;
    case FINAL:
        cout << "Displaying final result";
        break;
    }
    cout << conio::resetAll() << flush;
    row++;

    return type;
}

void ask_to_remove_player(int &row, vector<Executable> &execs) {
    string input = "";

    do {
        input = "";
        int num_execs = (int)execs.size()-1;
        cout << conio::gotoRowCol(row, 1) << conio::clearRow();
        cout << "Would you like to remove any AI from the contest?\n";
        for (int i = 0; i < (int)execs.size(); i++) {
            cout << "    " << i << "\t" << execs.at(i).file_name << "\n";
        }
        cout << "Please enter an AI # between 0-" << num_execs << " (ENTER if none): ";
        getline(cin, input);

        if (input == "") {
            break;
        } else {
            int choice = check_valid_int(input, 0, num_execs);
            if ( choice == -1 ) exit_abruptly();

            cout << conio::gotoRowCol(row, 1) << conio::clearRow();
            for (int i = 0; i < num_execs+2; i++) {
                cout << conio::gotoNextRow() << conio::clearRow();
            }
            cout << conio::gotoRowCol(row, 1) << conio::setTextStyle(conio::BOLD)
                 << "Removed " << execs.at(choice).file_name << " from the contest"
                 << conio::resetAll();
            execs.erase(execs.begin()+choice);
            row++;
        }

    } while ( input != "" );
    return;
}


/* ────────────────────── *
 * OPTIONS JSON FUNCTIONS *
 * ────────────────────── */

json read_options_file(const string &options_file) {
    json json_options = json::object();

    ifstream infile(options_file.c_str());

    if ( !infile.is_open() || infile.fail() ) {
        // doesn't exist, that's okay.
        add_json_options_layout(json_options);
        return json_options;
    }
    if ( !json::accept(infile) ) {
        print_error("Invalid JSON found in options.json file!", __FILE__, __LINE__);
        exit_abruptly();
    }

    infile.seekg(0, ios::beg);

    json_options = json::parse(infile);
    infile.close();

    add_json_options_layout(json_options);

    return json_options;
}

void add_json_options_layout(json &j) {
    add_object_with_empty_choice(j, RUNTIME_KEY);

    add_object(j, MATCH_OPTIONS_KEY);
    json &j_match = j[MATCH_OPTIONS_KEY];

    add_object_with_empty_choice(j_match, OPTIONS_BOARD_KEY);
    add_object_with_empty_choice(j_match, GAMES_PER_MATCH_KEY);
    add_object_with_empty_choice(j_match, DISPLAY_TYPE_KEY);
    add_object_with_empty_choice(j_match, STEP_THROUGH_KEY);
    add_object_with_empty_choice(j_match, DELAY_TIME_KEY);
    add_object_with_empty_choice(j_match, OPTIONS_P1_KEY);
    add_object_with_empty_choice(j_match, OPTIONS_P2_KEY);

    add_object(j, CONTEST_OPTIONS_KEY);
    json &j_contest = j[CONTEST_OPTIONS_KEY];

    add_object_with_empty_choice(j_contest, OPTIONS_BOARD_KEY);
    add_object_with_empty_choice(j_contest, GAMES_PER_MATCH_KEY);
    add_object_with_empty_choice(j_contest, DISPLAY_TYPE_KEY);
    add_object_with_empty_choice(j_contest, DELAY_TIME_KEY);
    return;
}

void add_object_with_empty_choice(json &j, const char *key) {
    add_object(j, key);
    add_empty_choice(j[key]);
    return;
}

void add_object(json &j, const char *key) {
    if ( !check_object(j, key) ) {
        j[key] = json::object();
    }
    return;
}

void add_empty_choice(json &j) {
    if ( !check_string(j, CHOICE_KEY) ) {
        j[CHOICE_KEY] = "";
    }
    return;
}

string get_json_options_choice(json &j, const char *key) {
    string value = "";
    if ( check_object(j, key) && ( check_string(j[key], CHOICE_KEY) || j.at(key).at(CHOICE_KEY) != nullptr ) ) {
        value = j.at(key).at(CHOICE_KEY);
    }
    return value;
}


/* ────────────────────────────── *
 * MATCH DISPLAY OPTION FUNCTIONS *
 * ────────────────────────────── */

int ask_display_game_increment(int &row, int num_games) {
    string input = "";
    vector<int> options;
    vector<int> divisors = {2, 5, 10, 25, 50};

    for (int i = 0; i < (int)divisors.size(); i++) {
        int option = num_games / divisors.at(i);
        if ( option > 0 ) options.push_back(option);
    }

    int num_options = (int)options.size();
    if ( num_options <= 0 ) return -1;

    cout << conio::gotoRowCol(row, 1)
         << "Choose the rate to display the " << num_games << " games\n";
    for (int i = 0; i < num_options; i++) {
        cout << "    " << i << "\tEvery " << options.at(i) << "\n";
    }
    cout << conio::gotoRowCol(row+1+num_options, 1)
         << "Please enter a choice between 0-" << num_options-1 << ": ";
    getline(cin, input);
    row += 3 + num_options;

    if ( input == "" ) return -1;
    int choice = check_valid_int(input, 0, num_options-1);
    if ( choice == -1 ) return choice;
    else return options.at(choice);
}

int ask_display_game_choice(int &row, int min, int max) {
    string input = "";
    cout << conio::gotoRowCol(row, 1) 
         << "Please enter a game to display between " << min << "-" << max << " (ENTER to stop): ";
    getline(cin, input);
    row += 2;

    if ( input == "" ) return -1;
    int choice = check_valid_int(input, min, max);
    return choice;
}


/* ──────────────────── *
 * FILESYSTEM FUNCTIONS *
 * ──────────────────── */

vector<Executable> get_all_execs(const string &system_dir) {
    DIR *dir;
    dirent *dp;
    vector<Executable> execs;

    const string exec_dir = system_dir + EXEC_DIR;

    get_protected_execs(exec_dir, execs);
    // make sure the directory can be opened
    if ( (dir = opendir(exec_dir.c_str())) == NULL ) {
        print_error("Cannot open ai_files/ directory!", __FILE__, __LINE__);
        fprintf(stderr, "Directory path: %s\n", exec_dir.c_str());
        exit_abruptly();
    }

    struct stat sb;
    while ( (dp = readdir(dir)) != NULL ) {
        string full_path = exec_dir + dp->d_name;
        // if it is both a regular file, and an executable, add it.
        if ( stat(full_path.c_str(), &sb) == 0 && sb.st_mode & S_IXUSR && sb.st_mode & S_IFREG ) {
            Executable exec;
            exec.file_name = dp->d_name;
            exec.exec = full_path;
            execs.push_back(exec);
        }
    }
   
    // sort the players in alphabetical order
    sort(execs.begin(), execs.end(), sort_players_by_name);
    
    return execs;
}

void get_protected_execs(const string &exec_dir, vector<Executable> &execs) {
    DIR *dir;
    dirent *dp;
    string protect_dir = exec_dir + PROTECT_DIR;

    // make sure the directory can be opened.
    if ( (dir = opendir(protect_dir.c_str())) == NULL ) {
        print_error("Cannot open ai_files/protected/ directory!", __FILE__, __LINE__);
        fprintf(stderr, "Directory path: %s\n", protect_dir.c_str());
        exit_abruptly();
    }

    struct stat sb;
    // for each file in the directory
    while ( (dp = readdir(dir)) != NULL ) {
        string full_path = protect_dir + dp->d_name;
        // if it is both a regular file, and an executable, add it.
        if ( stat(full_path.c_str(), &sb) == 0 && sb.st_mode & S_IXUSR && sb.st_mode & S_IFREG ) {
            Executable exec;
            exec.file_name = dp->d_name;
            exec.exec = full_path;
            execs.push_back(exec);
        }
    }
    return;
}

bool sort_players_by_name(const Executable &a, const Executable &b) {
    return a.file_name < b.file_name;
}


/* ───────────────────── *
 * CHECK INPUT FUNCTIONS *
 * ───────────────────── */

int check_valid_int(string input, int min, int max) {
    int len = input.length();
    int value;

    if ( len == 0 ) return -1;
    for (int i = 0; i < len; i++) {
        if ( !isdigit(input.at(i)) ) return -1;
    }
    value = stoi(input);
    if (value < min || value > max) return -1;
    
    return value;
}

float check_valid_float(string input, float min, float max) {
    int len = input.length();
    float value;
    bool period_found = false;

    if ( len == 0 ) return -1.0;
    for (int i = 0; i < len; i++) {
        if ( !isdigit(input.at(i)) ) {
            if ( input.at(i) == '.' && period_found == false ) {
                period_found = true;
            } else return -1.0;
        }
    }
    value = stof(input);
    if ( value < min || value > max ) return -1.0;
    return value;
}

void exit_abruptly() {
    cout << "\nExiting.\n" << flush;
    exit(1);
}

