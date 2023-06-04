/**
 * @file display.cpp
 * @author Matthew Getgen
 * @brief Display Functionality for the Battleships Contest
 * @date 2022-04-20
 */

#include "display.h"
#include "conio.h"
#include "defines.h"
#include "json.hpp"
#include "questions.h"


/* ───────────────────────── *
 * DISPLAY CONTEST FUNCTIONS *
 * ───────────────────────── */

void replay_contest(int delay, string log_dir, bool leaderboard_only) {
	ifstream infile((log_dir + CONTEST_LOG).c_str());
	// make sure the file exists.
	if ( !infile.is_open() || infile.fail() ) {
		print_error("Failed opening contest log file.", __FILE__, __LINE__);
		infile.close();
		return;
	}
	json contest_log = json::parse(infile);
	infile.close();

	// make sure it is a valid contest log.
	if ( !validate_contest_log(contest_log) ) return;

	// display the leaderboard only
	if ( leaderboard_only ) {
		vector<ContestPlayer> players;
		players = get_contest_players(contest_log[PLAYERS_KEY], true);
		display_leaderboard(players, true);

	// display the contest normally
	} else display_contest(contest_log, delay);

	return;
}

void display_contest(json &contest_log, int delay) {
	vector<ContestPlayer> players = get_contest_players(contest_log[PLAYERS_KEY], false);

	Board boards;
	boards.size = (int)contest_log[BOARD_SIZE_KEY];
	create_boards(boards);

	// iterate through each contest round.
	int num_contest_rounds = (int)contest_log[MATCHES_KEY].size();
	for (int i = 0; i < num_contest_rounds; i++) {
		
		// iterate through each match in the round.
		int num_matches = (int)contest_log[MATCHES_KEY].at(i).size();
		for (int j = 0; j < num_matches; j++) {

			// get the correct index for the player in the players struct.
			int idx1 = (int)contest_log[MATCHES_KEY].at(i).at(j)[PLAYER_1_KEY][PLAYER_IDX_KEY];
			int idx2 = (int)contest_log[MATCHES_KEY].at(i).at(j)[PLAYER_2_KEY][PLAYER_IDX_KEY];
			idx1 = get_ai_num_by_idx(players, idx1);
			idx2 = get_ai_num_by_idx(players, idx2);

			Name names;
			if ( idx1 != -1 ) {
				names.p1 = players.at(idx1).ai_name;
				names.a1 = players.at(idx1).author_name;
			}
			if ( idx2 != -1 ) {
				names.p2 = players.at(idx2).ai_name;
				names.a2 = players.at(idx2).author_name;
			}
			// ai name is always clean, but author name isn't.
			clean_name(names.p1);
			clean_name(names.p2);

			display_game(contest_log[MATCHES_KEY].at(i).at(j)[LAST_GAME_KEY], boards, delay, -1, names);

			display_match_errors(contest_log[MATCHES_KEY].at(i).at(j), boards.size);

			display_match_results(contest_log[MATCHES_KEY].at(i).at(j), boards.size, names);

			// for each game, store the  wins, losses, and ties into their ContestPlayer struct.
			store_match_player_values(players.at(idx1), contest_log[MATCHES_KEY].at(i).at(j)[PLAYER_1_KEY]);
			store_match_player_values(players.at(idx2), contest_log[MATCHES_KEY].at(i).at(j)[PLAYER_2_KEY]);

			sleep(SLEEP_TIME);
		}

		// display Top 10 Leaderboard if it's not the last round.
		if ( i != num_contest_rounds-1 ) {
			display_leaderboard(players, false);
			sleep(SLEEP_TIME);
		}
	}
	// display the Final Leaderboard if it's the last round.
	display_leaderboard(players, true);

	delete_boards(boards);
	return;
}

vector<ContestPlayer> get_contest_players(json &contest_players, bool get_end_value) {
	vector<ContestPlayer> players;
	int num_players = (int)contest_players.size();

	for (int i = 0; i < num_players; i++) {
		ContestPlayer player;
		string ai = contest_players.at(i)[AI_NAME_KEY];
		string authors = contest_players.at(i)[AUTHOR_NAMES_KEY];
		memset(player.ai_name, 0, MAX_NAME_SIZE);
		memset(player.author_name, 0, MAX_NAME_SIZE);

		// make sure the AI and Authors names aren't too big.
		if (ai.size() > MAX_NAME_SIZE)      strncpy(player.ai_name, ai.c_str(), MAX_NAME_SIZE-1);
		else                                strncpy(player.ai_name, ai.c_str(), ai.size());
		if (authors.size() > MAX_NAME_SIZE)	strncpy(player.author_name, authors.c_str(), MAX_NAME_SIZE-1);
		else                                strncpy(player.author_name, authors.c_str(), authors.size());

		// Used when we just want to display the final leaderboard only.
		if ( get_end_value ) {
			player.total_wins 	= (int)contest_players.at(i)[TOTAL_WINS_KEY];
			player.total_losses = (int)contest_players.at(i)[TOTAL_LOSSES_KEY];
			player.total_ties	= (int)contest_players.at(i)[TOTAL_TIES_KEY];
		// Else, just start at 0 and add to it as the contest goes on.
		} else {
			player.total_wins = 0;
			player.total_losses = 0;
			player.total_ties = 0;
		}
		
		player.idx = (int)contest_players.at(i)[PLAYER_IDX_KEY];

		players.push_back(player);
	}

	return players;
}

int get_ai_num_by_idx(vector<ContestPlayer> &players, int idx) {
	int num_players = (int)players.size();
	for (int i = 0; i < num_players; i++) {
		if ( players.at(i).idx == idx ) return i;
	}
	return -1;
}

void store_match_player_values(ContestPlayer &player, json &contest_player) {
	player.total_wins += (int)contest_player[WINS_KEY];
	player.total_losses += (int)contest_player[LOSSES_KEY];
	player.total_ties += (int)contest_player[TIES_KEY];
	player.error = (ErrorNum)contest_player[ERROR_KEY];
	return;
}

void display_leaderboard(vector<ContestPlayer> &players, bool show_all) {
	string r = "Rank", auth = "Author(s)", ai = "AI Name", w = "Wins", l = "Losses", t = "Ties", author_line, ai_line;
	cout << clrscr() << flush;

	sort(players.begin(), players.end(), sort_players_by_winner);

	cout << gotoRowCol(TOP_ROW_OFFSET-10, BOARD_1_OFFSET);
	if ( show_all ) cout << "Final Leaderboard" << flush;
	else 			cout << "Top 10 Leaderboard" << flush; 

	int author_size = auth.size(), ai_size = ai.size(), num_players = players.size();
	
	// to display the top 10 AI
	if ( !show_all && num_players > 10 ) num_players = 10;

	for (int i = 0; i < num_players; i++) {
		// doing wizard magic to make sure the names are clean
		string author = players.at(i).author_name;
		string ai = players.at(i).ai_name;
		clean_name(author);
		clean_name(ai);

		// check the full size of the names after cleaning.
		int auth_s = author.size(), ai_s = ai.size();

		// get the largest name (which now should be 20, the max).
		if ( author_size < auth_s ) author_size = auth_s;
		if ( ai_size < ai_s ) 		ai_size = ai_s;
	}

	if ( author_size > MAX_NAME_LEN ) author_size = MAX_NAME_LEN;
	if ( ai_size > MAX_NAME_LEN ) ai_size = MAX_NAME_LEN;

	for (int i = 0; i < author_size; i++) {
		author_line.append("─");
	}
	for (int i = 0; i < ai_size; i++) {
		ai_line.append("─");
	}
	
	cout << gotoRowCol(TOP_ROW_OFFSET-8, BOARD_1_OFFSET);
	printf(" %s │ %*s │ %*s │ %6s │ %6s │ %6s │\n",
		r.c_str(),
		author_size, auth.c_str(),
		ai_size, ai.c_str(),
		w.c_str(),
		l.c_str(),
		t.c_str()
	);

	printf("──────┼─%s─┼─%s─┼────────┼────────┼────────┤\n", author_line.c_str(), ai_line.c_str());
	
	for (int i = 0, rank = 1; i < num_players; i++, rank++) {
		string author_name = players.at(i).author_name;
		string ai_name = players.at(i).ai_name;
		clean_name(author_name);
		clean_name(ai_name);
		printf(" %4d │ %*s │ %*s │ %6d │ %6d │ %6d │\n",
			rank,
			author_size, author_name.c_str(),
			ai_size, ai_name.c_str(),
			players.at(i).total_wins,
			players.at(i).total_losses,
			players.at(i).total_ties
		);
	}
	
	if ( !show_all ) reset_cursor();

	return;
}

bool sort_players_by_winner(const ContestPlayer &a, const ContestPlayer &b) {
	if ( a.total_wins == b.total_wins) {
		if ( a.total_ties == b.total_ties) {
			// return "a has less losses than b" if wins and ties are equal.
			return a.total_losses < b.total_losses;
		}
		// return "a has more ties than b" if wins are equal.
		return a.total_ties > b.total_ties;
	}
	// return "a has more wins than b" otherwise.
	return a.total_wins > b.total_wins;
}


/* ─────────────────────── *
 * DISPLAY MATCH FUNCTIONS *
 * ─────────────────────── */

void replay_match(DisplayType display, int delay, string log_dir, bool step_through) {
	ifstream infile((log_dir + MATCH_LOG).c_str());
	if ( !infile.is_open() || infile.fail() ) {
		print_error("Failed opening match log file.", __FILE__, __LINE__);
		infile.close();
		return;
	}
	json match_log = json::parse(infile);
	infile.close();

	if ( !validate_match_log(match_log) ) return;

	display_match(match_log, display, delay, step_through);
	return;
}

void display_match(json &match_log, DisplayType display, int delay, bool step_through) {
	Name names;
	names.p1 = match_log[PLAYER_1_KEY][AI_NAME_KEY];
	names.p2 = match_log[PLAYER_2_KEY][AI_NAME_KEY];
	names.a1 = match_log[PLAYER_1_KEY][AUTHOR_NAMES_KEY];
	names.a2 = match_log[PLAYER_2_KEY][AUTHOR_NAMES_KEY];

	// always clean ai name, dont always need to clean author name
	clean_name(names.p1);
	clean_name(names.p2);

	Board boards;
	boards.size = (int)match_log[BOARD_SIZE_KEY];
	create_boards(boards);

	int num_games = (int)match_log[GAMES_KEY].size();

	// This assumes that a game should have been played,
	// so display the unplayed game and potential errors.
	if ( num_games <= 0 ) {
		display_init_boards(boards.size, names, true);

		// display any match errros
		display_match_errors(match_log, boards.size);
		
		// display match results
		display_match_results(match_log, boards.size, names);
		delete_boards(boards);
		return;
	}
	
	vector<int> game_indexes;

	// this assumes the user doesn't want to see the game at all,
	// display the results intstead.
	if ( display == NONE ) {
		only_display_match_results(match_log, names);
		delete_boards(boards);
		return;

	} else if ( display == INCREMENT ) {
		int increment = ask_incremental_question(num_games);
		if ( increment == -1 ) exit_abruptly();
		for (int i = num_games-1; i >= 0; i -= increment) {
			game_indexes.push_back(i);
		}
	} else {
		int win_idx = -1, loss_idx = -1, tie_idx = -1;

		for (int i = 0; i < num_games; i++) {
			if ( display == ALL ) {
				game_indexes.push_back(i);
			} else if ( display == LAST ) {
				game_indexes.push_back(num_games-1);
				break;
			} else if ( display == EACH ) {
				GameResult outcome = (GameResult)match_log[GAMES_KEY].at(i)[PLAYER_1_KEY][OUTCOME_KEY];
				if 		( outcome == WIN )	win_idx	 = i;
				else if ( outcome == LOSS )	loss_idx = i;
				else if ( outcome == TIE )	tie_idx  = i;
			}
		}

		if ( display == EACH ) {
			if ( win_idx  != -1 ) game_indexes.push_back(win_idx);
			if ( loss_idx != -1 ) game_indexes.push_back(loss_idx);
			if ( tie_idx  != -1 ) game_indexes.push_back(tie_idx);
			// if there was an error, push the last game back (The last game is always an error).
			ErrorNum err1 = (ErrorNum)match_log[PLAYER_1_KEY][ERROR_KEY];
			ErrorNum err2 = (ErrorNum)match_log[PLAYER_2_KEY][ERROR_KEY];
			if ( err1 != NO_ERR || err2 != NO_ERR ) game_indexes.push_back(num_games-1);
		}
	}

	// sort vector
	sort(game_indexes.begin(), game_indexes.end());
	// remove duplicates, if they exist
	game_indexes.erase( unique(game_indexes.begin(), game_indexes.end()), game_indexes.end() );
	
	if ( step_through ) step_through_each_game(match_log, boards, game_indexes, names);
	else 				display_each_game(match_log, boards, game_indexes, delay, names);

	// num_games = game_indexes.size();
	// for (int i = 0; i < num_games; i++) {
	// 	if ( step_through ) step_through_game(match_log[GAMES_KEY].at(game_indexes.at(i)), boards, p1, p2, a1, a2, game_indexes.at(i));
	// 	else 				display_game(match_log[GAMES_KEY].at(game_indexes.at(i)), boards, delay, p1, p2, a1, a2, game_indexes.at(i));
	// 	if ( i < num_games-1 && !step_through ) sleep(SLEEP_TIME);
	// }

	// display any match errros
	display_match_errors(match_log, boards.size);
	
	// display match results
	display_match_results(match_log, boards.size, names);
	delete_boards(boards);
	return;
}

void display_each_game(json &match_log, Board &boards, vector<int> &game_indexes, int delay, Name &names) {
	int num_games = game_indexes.size();
	for (int i = 0; i < num_games; i++) {
		display_game(match_log[GAMES_KEY].at(game_indexes.at(i)), boards, delay, game_indexes.at(i), names);
		if ( i < num_games-1 ) sleep(SLEEP_TIME);
	}
	return;
}

void step_through_each_game(json &match_log, Board &boards, vector<int> &game_indexes, Name &names) {
	json step_through_log = get_step_through_match_log(match_log, game_indexes);
	
	step_through_game2(step_through_log, boards, game_indexes, names);
	return;
}

void display_match_errors(json &match_log, int board_size) {
	int result_offset = TOP_ROW_OFFSET + board_size + 1;
	ErrorNum err1 = (ErrorNum)match_log[PLAYER_1_KEY][ERROR_KEY];
	ErrorNum err2 = (ErrorNum)match_log[PLAYER_2_KEY][ERROR_KEY];
	display_error_by_code(err1, result_offset, BOARD_1_OFFSET);
	display_error_by_code(err2, result_offset, BOARD_2_OFFSET);
	return;
}

void display_error_by_code(ErrorNum error, int row_offset, int col_offset) {
	cout << gotoRowCol(row_offset, col_offset) << bgColor(WHITE) << fgColor(RED);
	switch (error) {
		case BAD_SOCK_CREATE:
			printf("Error: %s (code: %d)", SOCKET_CREATE_ERR, error);
			break;
		case BAD_SOCK_NAME:
			printf("Error: %s (code: %d)", SOCKET_NAME_ERR, error);
			break;
		case BAD_SOCK_BIND:
			printf("Error: %s (code: %d)", SOCKET_BIND_ERR, error);
			break;
		case BAD_SOCK_OPT:
			printf("Error: %s (code: %d)", SOCKET_OPT_ERR, error);
			break;
		case BAD_FORK:
			printf("Error: %s (code: %d)", PLAYER_FORK_ERR, error);
			break;
		case BAD_CONNECT:
			printf("Error: %s (code: %d)", SOCKET_CONNECT_ERR, error);
			break;
		case BAD_SEND:
			printf("Error: %s (code: %d)", SEND_MESSAGE_ERR, error);
			break;
		case BAD_RECV:
			printf("Error: %s (code: %d)", RECV_MESSAGE_ERR, error);
			break;
		case BAD_HELLO_MSG:
			printf("Error: %s (code: %d)", HELLO_MSG_ERR, error);
			break;
		case BAD_SHIP_PLACED_MSG:
			printf("Error: %s (code: %d)", SHIP_MSG_ERR, error);
			break;
		case BAD_SHOT_TAKEN_MSG:
			printf("Error: %s (code: %d)", SHOT_MSG_ERR, error);
			break;
		case BAD_SHIP:
			printf("Error: %s (code: %d)", SHIP_PLACE_ERR, error);
			break;
		case BAD_SHOT:
			printf("Error: %s (code: %d)", SHOT_PLACE_ERR, error);
			break;
		case NO_ERR:
		default:
			break;
	}
	cout << bgColor(RESET) << fgColor(RESET);
	return;
}

void only_display_match_results(json &match_log, Name &names) {
	cout << clrscr() << flush;
	cout << "Match Results" << endl;
	display_match_results(match_log, -TOP_ROW_OFFSET-1, names);	// -12 to display it on top of the screen
	return;
}

void display_match_results(json &match_log, int board_size, Name &names) {
	string ai = "AI Name", w = "Wins", l = "Losses", t = "Ties";
	int result_offset = TOP_ROW_OFFSET + board_size + 4;
	int p1_size, p2_size, longest = ai.size();

	p1_size = names.p1.size();
	p2_size = names.p2.size();

	if ( p1_size > longest ) longest = p1_size;
	if ( p2_size > longest ) longest = p2_size;

	string display_line = "──";

	for (int i = 0; i < longest; i++) {
		display_line.append("─");
	}
	display_line.append("┼────────┼────────┼────────┤");

	cout << gotoRowCol(result_offset, 1);
	printf(" %*s │ %6s │ %6s │ %6s │\n", longest, ai.c_str(), w.c_str(), l.c_str(), t.c_str());
	printf("%s\n", display_line.c_str());
	display_match_results_by_player(match_log[PLAYER_1_KEY], names.p1, longest);
	display_match_results_by_player(match_log[PLAYER_2_KEY], names.p2, longest);
	int seconds = (int) match_log[ELAPSED_TIME_KEY];
	cout << gotoRowCol(result_offset+5, 1);
	if ( seconds == 1 ) cout << "Time elapsed: " << seconds << " second";
	else cout << "Time elapsed: " << seconds << " seconds";
	reset_cursor();
	return;
}

void display_match_results_by_player(json &match_player, string &p, int display_offset) {
	int wins = 	 (int)match_player[WINS_KEY],
		losses = (int)match_player[LOSSES_KEY],
		ties = 	 (int)match_player[TIES_KEY];

	printf(" %*s │ %6d │ %6d │ %6d │\n", display_offset, p.c_str(), wins, losses, ties);
	return;
}


/* ────────────────────── *
 * DISPLAY GAME FUNCTIONS *
 * ────────────────────── */

void display_game(json &game_log, Board &boards, int delay, int game_num, Name &names) {
	clear_boards(boards);
	display_init_boards(boards.size, names, false);
	display_game_number(game_num);

	Ship ship1, ship2;

	int num_ships = (int)game_log[PLAYER_1_KEY][SHIPS_KEY].size();
	for (int i = 0; i < num_ships; i++) {
		move_ship_to_struct(ship1, game_log[PLAYER_1_KEY][SHIPS_KEY].at(i));
		move_ship_to_struct(ship2, game_log[PLAYER_2_KEY][SHIPS_KEY].at(i));

		store_ship_to_board(ship1, boards.board1, SHIP);
		store_ship_to_board(ship2, boards.board2, SHIP);
	}

	Shot shot1, shot2;

	int num_shots = (int)game_log[PLAYER_1_KEY][SHOTS_KEY].size();
	usleep(delay);
	json shot1_log, shot2_log;
	for (int i = 0; i < num_shots; i++) {
		shot1_log = game_log[PLAYER_1_KEY][SHOTS_KEY].at(i);
		shot2_log = game_log[PLAYER_2_KEY][SHOTS_KEY].at(i);

		move_shot_to_struct(shot1, shot1_log);
		move_shot_to_struct(shot2, shot2_log);

		handle_killed_ship(shot1, shot1_log, game_log[PLAYER_2_KEY][SHIPS_KEY], PLAYER_2, boards, true);
		handle_killed_ship(shot2, shot2_log, game_log[PLAYER_1_KEY][SHIPS_KEY], PLAYER_1, boards, true);

		store_shot_to_board(shot1, boards.board2);
		store_shot_to_board(shot2, boards.board1);

		display_board_value(PLAYER_1, shot2, true);
		display_board_value(PLAYER_2, shot1, true);
		display_shot_value(boards.size, PLAYER_1, shot2);
		display_shot_value(boards.size, PLAYER_2, shot1);
		reset_cursor();
		usleep(delay);
		display_board_value(PLAYER_1, shot2, false);
		display_board_value(PLAYER_2, shot1, false);
		
	}


	display_init_boards(boards.size, names, true);
	display_entire_board(boards);
	display_game_number(game_num);
	display_game_result(game_log, boards.size, names);
	reset_cursor();
	return;
}

void step_through_game2(json &step_through_log, Board &boards, vector<int> game_indexes, Name &names) {
	int input_offset = TOP_ROW_OFFSET + boards.size + 4;
	display_init_boards(boards.size, names, false);
	clear_boards(boards);

	// by default, start at the first game (0).
	int ystep = 0;
	// by default, start before there are any ships (-1).
	int xstep = -1;
	
	// trust the log over the vector, even though they're the same probably.
	int num_games = (int)step_through_log[GAMES_KEY].size();
	json game_step = step_through_log[GAMES_KEY].at(ystep);
	int num_actions = (int)game_step[PLAYER_1_KEY].size();
	display_game_number(game_indexes.at(ystep));

	cout << gotoRowCol(input_offset, BOARD_1_OFFSET) << "Please hit ENTER to start steping through: ";
	int ch = getchar();
	if ( ch != 10 ) exit_abruptly();

	BufferToggle bt;
	bt.off();
	do {
		cout << gotoRowCol(input_offset, BOARD_1_OFFSET) << "Please use  🠖  🠔  🠕  🠗  or ENTER key to end: ";
		ch = getchar();
		
		// === UP ===
		if ( ch == 65 && ystep > 0 ) {
			cout << " 🠕  ";
			ystep--;
			clear_boards(boards);

			game_step = step_through_log[GAMES_KEY].at(ystep);
			num_actions = (int)game_step[PLAYER_1_KEY].size();
			display_game_number(game_indexes.at(ystep));

			if ( xstep >= num_actions ) xstep = num_actions-1;

			// if it's still at the first step (-1), nothing will happen.
			for (int i = 0; i <= xstep; i++) {
				process_step(game_step, boards, i);
			}
			display_entire_board(boards);

		// === DOWN ===
		} else if ( ch == 66 && ystep < num_games-1 ) {
			cout << " 🠗  ";
			ystep++;
			clear_boards(boards);
			
			game_step = step_through_log[GAMES_KEY].at(ystep);
			num_actions = (int)game_step[PLAYER_1_KEY].size();
			display_game_number(game_indexes.at(ystep));

			if ( xstep >= num_actions ) xstep = num_actions-1;
			
			// if it's still at the first step (-1), nothing will happen.
			for (int i = 0; i <= xstep; i++) {
				process_step(game_step, boards, i);
			}
			display_entire_board(boards);

		// === RIGHT ===
		} else if ( ch == 67 && xstep < num_actions-1 ) {
			cout << " 🠖  ";
			xstep++;

			process_step(game_step, boards, xstep);
			display_entire_board(boards);

		// === LEFT ===
		} else if ( ch == 68 && xstep > -1 ) {
			cout << " 🠔  ";
			xstep--;
			clear_boards(boards);

			for (int i = 0; i <= xstep; i++) {
				process_step(game_step, boards, i);
			}
			display_entire_board(boards);
		
		// === NONE ===
		} else {
			cout << "    ";
		}

	} while ( ch != 10 );	// NEWLINE
	bt.on();

	return;
}

void step_through_game(json &game_log, Board &boards, int game_num, Name &names) {
	int input_offset = TOP_ROW_OFFSET + boards.size + 4;
	clear_boards(boards);
	display_init_boards(boards.size, names, false);
	display_game_number(game_num);

	json step_through_log = get_step_through_log(game_log);
	int num_actions = (int)step_through_log[PLAYER_1_KEY].size();

	BufferToggle bt;
	cout << gotoRowCol(input_offset, BOARD_1_OFFSET) << "Please hit ENTER to step through the game: ";
	int ch = getchar();
	if ( ch != 10 ) exit_abruptly();
	int step = -1;
	do {
		bt.off();
		cout << gotoRowCol(input_offset, BOARD_1_OFFSET) << "Please use ->, <-, or ENTER keys to end: ";
		ch = getchar();
		
		if ( ch == 67 && step < num_actions-1 ) {	// ->
			cout << " -> ";
			step++;
			process_step(step_through_log, boards, step);
			display_entire_board(boards);
		} else if ( ch == 68 && step > -1 ) {		// <-
			//						   -1 is the state before there are any ships.
			cout << " <- ";
			step--;
			clear_boards(boards);

			for (int i = 0; i <= step; i++) {
				process_step(step_through_log, boards, i);
				//display_forward_step(step_through_log, boards, i, false);
			}
			display_entire_board(boards);
		} else {	// neither
			cout << "    ";
		}

	}  while ( ch != 10 );	// NEWLINE
	bt.on();

	display_game_result(game_log, boards.size, names);
	return;
}

void process_step(json &step_log, Board &boards, int step) {
	json &step1 = step_log[PLAYER_1_KEY].at(step);
	json &step2 = step_log[PLAYER_2_KEY].at(step);

	if ( step1.contains(VALUE_KEY) ) {	// step is a shot.
		Shot shot1, shot2;

		move_shot_to_struct(shot1, step1);
		move_shot_to_struct(shot2, step2);

		store_shot_to_board(shot2, boards.board1);
		store_shot_to_board(shot1, boards.board2);

		handle_killed_ship(shot1, step1, step_log[PLAYER_2_KEY], PLAYER_2, boards, false);
		handle_killed_ship(shot2, step2, step_log[PLAYER_1_KEY], PLAYER_1, boards, false);
		
		display_shot_value(boards.size, PLAYER_1, shot2);
		display_shot_value(boards.size, PLAYER_2, shot1);

	} else {	// step is a ship.
		Ship ship1, ship2;

		move_ship_to_struct(ship1, step1);
		move_ship_to_struct(ship2, step2);
		
		store_ship_to_board(ship1, boards.board1, SHIP);
		store_ship_to_board(ship2, boards.board2, SHIP);

		display_ship_placement(boards.size, PLAYER_1, ship1);
		display_ship_placement(boards.size, PLAYER_2, ship2);
	}
	return;
}

void handle_killed_ship(Shot &shot, json &shot_log, json &ships, PlayerNum player, Board &boards, bool display) {
	if ( shot_log.contains(INDEX_SHIP_KEY) ) {
		Ship ship;
		int kill_id = (int)shot_log[INDEX_SHIP_KEY];

		// store ship
		move_ship_to_struct(ship, ships.at(kill_id));
		if ( player == PLAYER_1 ) store_ship_to_board(ship, boards.board1, KILL);
		else 					  store_ship_to_board(ship, boards.board2, KILL);

		if ( display ) display_ship(player, ship, KILL);
		shot.value = KILL;
	}
	return;
}

void display_game_number(int game_num) {
	cout << gotoRowCol(TOP_ROW_OFFSET-6, BOARD_1_OFFSET);
	if ( game_num == -1 ) cout << "Last Game";
	else cout << "Game #" << game_num + 1 << "   ";
	return;
}

void display_game_result(json &game_log, int board_size, Name &names) {
	int result_offset = TOP_ROW_OFFSET + board_size + 1;
	GameResult result1, result2;
	result1 = game_log[PLAYER_1_KEY][OUTCOME_KEY];
	result2 = game_log[PLAYER_2_KEY][OUTCOME_KEY];
	display_game_result_by_player(result1, names.p1, result_offset, BOARD_1_OFFSET);
	display_game_result_by_player(result2, names.p2, result_offset, BOARD_2_OFFSET);
	return;
}

void display_game_result_by_player(GameResult result, string &p, int row_offset, int col_offset) {
	cout << gotoRowCol(row_offset, col_offset);
	switch (result) {
		case WIN:
			cout << p << " won!";
			break;
		case LOSS:
			cout << p << " lost.";
			break;
		case TIE:
			cout << "A tie!";
			break;
		default:
			break;
	}
	return;
}


/* ─────────────────────── *
 * DISPLAY BOARD FUNCTIONS *
 * ─────────────────────── */

void display_init_boards(int board_size, Name &names, bool is_final) {
	string colNums = " │";
	string colLine = "─┼";
	string waterLine = "";

	cout << clrscr() << flush;

	// display author names.
	cout << gotoRowCol(TOP_ROW_OFFSET-10, BOARD_1_OFFSET) 
		 << names.a1 << "\n"
		 << "──── VS ────\n"
		 << names.a2 << "\n";

	// display AI names.
	if ( is_final ) {
		cout << gotoRowCol(TOP_ROW_OFFSET-4, BOARD_1_OFFSET) << "Final Status of " << names.p1 << "'s Board:"
			 << gotoRowCol(TOP_ROW_OFFSET-4, BOARD_2_OFFSET) << "Final Status of " << names.p2 << "'s Board:";
	} else {
		cout << gotoRowCol(TOP_ROW_OFFSET-4, BOARD_1_OFFSET) << names.p1 << "'s Board:"
			 << gotoRowCol(TOP_ROW_OFFSET-4, BOARD_2_OFFSET) << names.p2 << "'s Board:";

	}

	// get numbers, line, and water in separate long strings to be able to print efficiently.
	for (int i = 0; i < board_size; i++) {
		colNums.append(to_string(i));
		colLine.append("─");
		waterLine.push_back(WATER);
	}

	// print column numbers and line.
	cout << gotoRowCol(TOP_ROW_OFFSET-2, BOARD_1_OFFSET) << colNums 
		 << gotoRowCol(TOP_ROW_OFFSET-2, BOARD_2_OFFSET) << colNums;
	cout << gotoRowCol(TOP_ROW_OFFSET-1, BOARD_1_OFFSET) << colLine 
		 << gotoRowCol(TOP_ROW_OFFSET-1, BOARD_2_OFFSET) << colLine;

	// print row numbers, line, and water.
	for (int i = 0; i < board_size; i++) {
		cout << gotoRowCol(TOP_ROW_OFFSET+i, BOARD_1_OFFSET)
			 << bgColor(RESET) << fgColor(RESET) << i << "│"
			 << bgColor(LIGHT_CYAN) << fgColor(BLACK) << waterLine;

		cout << gotoRowCol(TOP_ROW_OFFSET+i, BOARD_2_OFFSET)
			 << bgColor(RESET) << fgColor(RESET) << i << "│"
			 << bgColor(LIGHT_CYAN) << fgColor(BLACK) << waterLine;
	}

	reset_cursor();
	return;
}

void display_entire_board(Board &boards) {
	Shot shot;
	for (shot.row = 0; shot.row < boards.size; shot.row++) {
		for (shot.col = 0; shot.col < boards.size; shot.col++) {
			shot.value = (BoardValue)boards.board1[shot.row][shot.col];
			display_board_value(PLAYER_1, shot, false);
			shot.value = (BoardValue)boards.board2[shot.row][shot.col];
			display_board_value(PLAYER_2, shot, false);
		}
	}
	return;
}

void display_board_value(PlayerNum player, Shot shot, bool is_current) {
	int offset = LEFT_COL_OFFSET;
	if 		( player == PLAYER_1 ) offset += BOARD_1_OFFSET;
	else if ( player == PLAYER_2 ) offset += BOARD_2_OFFSET;

	shot.row += TOP_ROW_OFFSET;
	shot.col += offset;

	Color bg, fg;
	BoardValue value;
	fg = BLACK;
	value = shot.value;
	switch (shot.value) {
		case WATER:
			bg = LIGHT_CYAN;
			break;
		case SHIP:
			bg = WHITE;
			break;
		case HIT:
			// was LIGHT_MAGENTA, but I like this better. Can change later.
			bg = LIGHT_YELLOW;
			break;
		case DUPLICATE_HIT:
			bg = LIGHT_YELLOW;
			value = DUPLICATE;
			break;
		case MISS:
			bg = GRAY;
			break;
		case DUPLICATE_MISS:
			bg = GRAY;
			value = DUPLICATE;
			break;
		case KILL:
			bg = LIGHT_RED;
			// Was BLACK, but I also like this better. Can change later as well.
			fg = WHITE;
			break;
		case DUPLICATE_KILL:
			bg = LIGHT_RED;
			fg = WHITE;
			value = DUPLICATE;
			break;
		default:
			bg = RED;
			break;
	}
	if ( is_current ) {
		fg = bg;
		bg = BLACK;
	}
	cout << gotoRowCol(shot.row, shot.col) 
		 << bgColor(bg) << fgColor(fg) 
		 << value
		 << bgColor(RESET) << fgColor(RESET) << flush;
	return;
}

void display_shot_value(int board_size, PlayerNum player, Shot &shot) {
	int value_offset = TOP_ROW_OFFSET + board_size + 1, offset = BOARD_1_OFFSET;
	if ( player == PLAYER_2 ) offset = BOARD_2_OFFSET;

	cout << gotoRowCol(value_offset, offset);

	switch (shot.value) {
		case HIT:
			cout << fgColor(LIGHT_YELLOW) << "HIT" << fgColor(RESET);
			break;
		case MISS:
			cout << fgColor(GRAY) << "MISS" << fgColor(RESET);
			break;
		case KILL:
			cout << fgColor(LIGHT_RED) << "KILL" << fgColor(RESET);
			break;
		case DUPLICATE_HIT:
		case DUPLICATE_MISS:
		case DUPLICATE_KILL:
			cout << bgColor(WHITE) << fgColor(RED) << "DUPLICATE" << bgColor(RESET) << fgColor(RESET);
			break;
		default:
			break;
	}
	printf(" @ [%d, %d]           ", shot.row, shot.col);
	return;
}

void display_ship(PlayerNum player, Ship &ship, BoardValue value) {
    Shot shot;
    shot.value = value;
    for (int i = 0; i < ship.len; i++) {
        if ( ship.dir == HORIZONTAL ) {
            shot.row = ship.row;
            shot.col = ship.col + i;
        } else {
            shot.row = ship.row + i;
            shot.col = ship.col;
        }
        display_board_value(player, shot, false);
    }
    return;
}

void display_ship_placement(int board_size, PlayerNum player, Ship &ship) {
	int value_offset = TOP_ROW_OFFSET + board_size + 1, offset = BOARD_1_OFFSET;
	if ( player == PLAYER_2 ) offset = BOARD_2_OFFSET;

	cout << gotoRowCol(value_offset, offset);
	string dir_str;
	if ( ship.dir == HORIZONTAL ) dir_str = "HORIZONTAL";
	else 						  dir_str = "VERTICAL";
	printf("%s @ [%d, %d]           ", dir_str.c_str(), ship.row, ship.col);
	return;
}

void clean_name(string &s) {
	if ( s.size() > MAX_NAME_LEN-3 ) {
		s = s.substr(0, MAX_NAME_LEN-3);
		s.append("...");
	}
	return;
}

void reset_cursor() {
	cout << gotoRowCol(40, 1) << resetAll() << "" << flush;
	return;
}

