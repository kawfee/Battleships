/**
 * @file options.h
 * @authors Matthew Getgen
 * @brief Runtime Options for the Battleships Controller.
 * @date 2023-08-31
 */

#ifndef OPTIONS_H
#define OPTIONS_H

#include <algorithm>
#include <bits/stdc++.h>
#include <dirent.h>
#include <limits>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "conio.h"
#include "../json.hpp"
#include "../logic/logger.h"
#include "../defines.h"

using json = nlohmann::json;

#define RUNTIME_KEY         "runtime"
#define MATCH_OPTIONS_KEY   "match"
#define CONTEST_OPTIONS_KEY "contest"
#define CHOICE_KEY          "choice"

#define OPTIONS_BOARD_KEY   "board_size"
#define GAMES_PER_MATCH_KEY "games_per_match"
#define DISPLAY_TYPE_KEY    "display_type"
#define STEP_THROUGH_KEY    "step_through"
#define DELAY_TIME_KEY      "delay_time"
#define OPTIONS_P1_KEY      "player_1"
#define OPTIONS_P2_KEY      "player_2"



/// @brief Gets all options from either user input or options.json file.
/// @param row Row to display questions at.
/// @param system_dir Current working directory to get the default options from.
/// @return Options struct from input. 
Options get_options(int &row, const string &system_dir);

/// @brief Prints the starting message.
/// @return row value to print next values at.
int print_start();


/* ────────────────────────── *
 * RUNTIME QUESTION FUNCTIONS *
 * ────────────────────────── */

/// @brief Gets the runtime choice from the user or input.
/// @param row Row to display the question at.
/// @param j_options Options JSON struct to get the default options from.
/// @return Runtime choice from input.
Runtime get_runtime(int &row, json &j_options);


/* ──────────────────────── *
 * MATCH QUESTION FUNCTIONS *
 * ──────────────────────── */

/// @brief Gets the match runtime options from the user or options.
/// Asks all questions necessary to run a new match.
/// @param row Row to display the questions at.
/// @param system_dir Current working directory to find players at.
/// @param j_options Options JSON struct to get the default options from.
/// @return MatchOptions from input.
MatchOptions get_match_options(int &row, const string &system_dir, json &j_options);

/// @brief Gets the match replay runtime options from the user or options.
/// Doesn't ask questions that were already determined when the match initially ran.
/// @param row Row to display the questions at.
/// @param system_dir Current working directory to find the log file at.
/// @param j_options Options JSON struct to get the default options from.
/// @return MatchOptions from input.
MatchOptions get_match_replay_options(int &row, const string &system_dir, json &j_options);

/// @brief Gets the size of the board for the match from the user or input.
/// @param row Row to display the question at.
/// @param j_options Options JSON struct to get the default options from.
/// @return size of the board from input
int get_board_size(int &row, json &j_options);

/// @brief Gets the number of games to be played in a match from the user or options.
/// @param row Row to display the question at.
/// @param j_options Options JSON struct to get the default options from.
/// @return number of games per match from input.
int get_num_games(int &row, json &j_options);

/// @brief Gets the match display type from the user or options.
/// @param row Row to display the question at.
/// @param j_options Options JSON struct to get the default options from.
/// @return display type from input.
MatchDisplayType get_match_display_type(int &row, json &j_options);

/// @brief Gets the step through choice from the user or options.
/// @param row Row to display the question at.
/// @param j_options Options JSON struct to get the default options from.
/// @return true if step through, false if not.
bool get_step_through(int &row, json &j_options);

/// @brief Gets the delay time in Microseconds between each action on the board.
/// A longer delay time means the actions appear for longer.
/// @param row Row to display question at.
/// @param j_options Options JSON struct to get the default options from.
/// @return delay time in microseconds from input (as a float).
int get_delay_time(int &row, json &j_options);

/// @brief Gets either player1 or player2 for the match.
/// @param row Row to display question at.
/// @param execs List of executables to choose a player from.
/// @param player Player Number of either player1 or player2.
/// @param j_options Options JSON struct to get the default options from.
/// @return Executable of player chosen from input.
Executable get_match_player(int &row, vector<Executable> &execs, PlayerNum player, json &j_options);

/* ────────────────────────── *
 * CONTEST QUESTION FUNCTIONS *
 * ────────────────────────── */

/// @brief Gets the contest runtime options from the user or options.
/// Asks all questions necessary to run a new contest.
/// @param row Row to display questions at.
/// @param system_dir Current working directory to find players at.
/// @param j_options Options JSON struct that stores the default options.
/// @return ContestOptions from input.
ContestOptions get_contest_options(int &row, const string &system_dir, json &j_options);

/// @brief Gets the contest replay runtime options from the user or options.
/// Doesn't ask questions that were already determined when the contest initially ran.
/// @param row Row to display questions at.
/// @param system_dir Current working directory to find the log directory at.
/// @param j_options Options JSON struct that stores the default options.
/// @return ContestOptions from input.
ContestOptions get_contest_replay_options(int &row, const string &system_dir, json &j_options);

/// @brief Gets the contest display type from the user or options.
/// @param row Row to display question at.
/// @param j_options Options JSON struct that stores the default options.
/// @return ContestDisplayType from input.
ContestDisplayType get_contest_display_type(int &row, json &j_options);

/// @brief Asks if a player should be removed from a contest.
/// @param row Row to display question at.
/// @param execs List of executables to remove players from if necessary.
void ask_to_remove_player(int &row, vector<Executable> &execs);


/* ────────────────────── *
 * OPTIONS JSON FUNCTIONS *
 * ────────────────────── */

/// @brief Reads and parses the options.json file and sets the default layout.
/// @param options_file Path to the options file.
/// @return JSON struct of options file.
json read_options_file(const string &options_file);

/// @brief Defines the layout of the options.json struct and how it should be laid out if it doesn't already exist.
/// @param j JSON struct to store layout to.
void add_json_options_layout(json &j);

/// @brief Adds both an object and a choice field at that key to properly set up the JSON if not initialized correctly.
/// @param j JSON struct to store object to.
/// @param key Key to store object and choice at in JSON struct.
void add_object_with_empty_choice(json &j, const char *key);

/// @brief Adds an object to JSON struct if it doesn't exist.
/// @param j JSON struct to store object to.
/// @param key Key to store object at in JSON struct.
void add_object(json &j, const char *key);

/// @brief Adds an empty choice value to struct if it doesn't exist.
/// @param j JSON struct to store choice to.
void add_empty_choice(json &j);

/// @brief Returns the string value of a choice in the JSON options struct.
/// @param j JSON struct for options.
/// @param key Key for value that contains the choice.
/// @return value as string, or "" if not found.
string get_json_options_choice(json &j, const char *key);


/* ────────────────────────────── *
 * MATCH DISPLAY OPTION FUNCTIONS *
 * ────────────────────────────── */

/// @brief Asks about the rate to display games out of the total number of games.
/// Calculated after all games have run, and uses the total number of games to decide what a valid increment is.
/// @param row Row to display the question at.
/// @param num_games Number of games played in the match.
/// @return Rate of games to display chosen by the user.
int ask_display_game_increment(int &row, int num_games);

/// @brief Asks for a choice from all games play for which one to replay.
/// If something invalid is input, or nothing at all, it will return -1 which will cause the display to stop.
/// @param row Row to display the question at.
/// @param min Minimum num game to choose.
/// @param max Maximum num game to choose.
/// @return -1 if invalid input, or game number if valid.
int ask_display_game_choice(int &row, int min, int max);


/* ──────────────────── *
 * FILESYSTEM FUNCTIONS *
 * ──────────────────── */

/// @brief Reads all executables at players path, sorts them by name, and stores them in an Executable list.
/// @param system_dir Path to working directory. Appends "/ai_files/" to it.
/// @return Executable list from directory.
vector<Executable> get_all_execs(const string &system_dir);

/// @brief Reads all executables at protected players path and appends them to list.
/// @param exec_dir Path to ai files directory. Appends "/protected/" to it.
/// @param execs Executable list.
void get_protected_execs(const string &exec_dir, vector<Executable> &execs);

/// @brief Sorts players by their name, to be clean.
/// @param a Executable a to compare with b.
/// @param b Executable b to compare with a.
/// @return Boolean as to whether they are in order or not.
bool sort_players_by_name(const Executable &a, const Executable &b);


/* ───────────────────── *
 * CHECK INPUT FUNCTIONS *
 * ───────────────────── */

/// @brief Checks if the input string is a valid integer, and in range.
/// @param input Input string from user.
/// @param min Minimum allowed value of integer.
/// @param max Maximum allowed value of integer.
/// @return -1 if invalid, correctly parsed value if valid.
int check_valid_int(string input, int min, int max);

/// @brief Checks if the input string is a valid float, and in range.
/// @param input Input string from user.
/// @param min Minimum allowed value of float.
/// @param max Maximum allowed value of float.
/// @return -1.0 if invalid, correctly parsed value if valid.
float check_valid_float(string input, float min, float max);

/// @brief Exits from program abruptly and says "Exiting.".
void exit_abruptly();

#endif

