/**
 * @file questions.h
 * @authors Matthew Getgen
 * @brief Runtime Questions for the Battleships Controller.
 * @date 2022-12-03
 */

#ifndef QUESTIONS_H
#define QUESTIONS_H


#include <algorithm>
#include <bits/stdc++.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "defines.h"
#include "conio.h"

#define EXEC_DIR "/AI_Files/"
#define PROTECT_DIR "/protected/"

using namespace conio;


/* ────────────────────────── *
 * GENERAL QUESTION FUNCTIONS *
 * ────────────────────────── */

/**
 * @brief
 */
void exit_abruptly();

/**
 * @brief
 */
bool is_int(string str);

/**
 * @brief
 */
bool is_float(string str);

/**
 * @brief Decides what type of runtime the user wants to perform.
 * @return 0 for a match, 1 for a contest, 2 to replay a log, or -1 for none.
 */
int get_runtime_type();

/**
 * @brief
 */
vector<tuple<string, string>> get_all_players(string &system_dir);

/**
 * @brief
 */
void get_protected_players(string &exec_dir, vector<tuple<string, string>> &execs);

/**
 * @brief
 */
bool sort_players_by_name(const tuple<string, string> &a, const tuple<string, string> &b);

/**
 * @brief
 */
void ask_game_questions(int &board_size, int &num_games);

/**
 * @brief
 */
void ask_delay_question(int &delay);


/* ────────────────────────── *
 * CONTEST QUESTION FUNCTIONS *
 * ────────────────────────── */

/**
 * @brief
 */
void ask_contest_display_questions(DisplayType &contest_display, int &delay);

/**
 * @brief
 */
void check_ai_list(vector<tuple<string, string>> &execs);

/**
 * @brief Decides if you would like to display the entire contest, or just the leaderboard.
 * @return 0 to display the entire contest, 1 to display only the leaderboard, -1 for none.
*/
int ask_display_leaderboard();


/* ──────────────────────── *
 * MATCH QUESTION FUNCTIONS *
 * ─────────────────────────*/

/**
 * @brief
 */
void ask_display_questions(DisplayType &display, int &delay, bool &step_through, bool is_replay);

/**
 * @brief
 */
void ask_step_through_question(bool &step_through);

/**
 * @brief
 */
void ask_which_player(vector<tuple<string,string>> &execs, const char *word, string &ai_exec);

/**
 * @brief Finds incremental 
 */
int ask_incremental_question(int num_games);


/* ────────────────────── *
 * LOG QUESTION FUNCTIONS *
 * ────────────────────── */

/**
 * @brief Finds log files and decides which one to use.
 * @param log_dir Directory to check for log files.
 * @return 0 for match_log, 1 for contest_log, -1 for none.
 */
int find_log_files(string &log_dir);


#endif

