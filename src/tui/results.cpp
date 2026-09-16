/**
 * @file options.cpp
 * @author Matthew Getgen
 * @brief Battleships TUI output results.
 * @date 2026-07-04
 */

#include <cmath>
#include <iomanip>
#include <math.h>
#include <time.h>

#include "shared.cpp"
#include "../../lib/battleshipslib.h"

#define MIN_SET(value1, value2) (value1 = value1 < value2 ? value1 : value2)
#define MAX_SET(value1, value2) (value1 = value1 > value2 ? value1 : value2)
#define MIN_MAX_AVG_SET(value_stat, index, ai1_value, ai2_value) \
    do { \
        if (index == 0) \
        { \
            value_stat.ai1.fff.f1 = ai1_value; \
            value_stat.ai2.fff.f1 = ai1_value; \
        } \
        MIN_SET(value_stat.ai1.fff.f1, ai1_value); \
        MIN_SET(value_stat.ai2.fff.f1, ai2_value); \
        MAX_SET(value_stat.ai1.fff.f2, ai1_value); \
        MAX_SET(value_stat.ai2.fff.f2, ai2_value); \
        value_stat.ai1.fff.f3 += ai1_value; \
        value_stat.ai2.fff.f3 += ai2_value; \
    } while (0)

const Color PLAYER_1_COLOR = BLUE, PLAYER_2_COLOR = YELLOW;

Color TUI_Player_Color_Get(BShip_PlayerNum player)
{
    return player == BSHIP_PLAYER_1 ? PLAYER_1_COLOR : PLAYER_2_COLOR;
}

void TUI_Board_Add(vector<TUI_TextGroup> &group, const string &name, BShip_Board board, BShip_PlayerNum player)
{
    group.push_back(TUI_TextGroup_Default(TUI_Text_New(name, { BOLD }, TUI_Player_Color_Get(player), RESET)));
    {
        string numbered_row = " |";
        for (size_t i = 0; i < board.size; i++)
        {
            if (i < 10) numbered_row += to_string(i);
            // NOTE(mattg): ASCII A starts at 65, and this branch assumes 10+
            else numbered_row += (char)(i + 55);
        }
        group.push_back(TUI_TextGroup_Default(TUI_Text_Default(numbered_row)));
    }
    {
        string line_row = "-+";
        for (size_t i = 0; i < board.size; i++)
        {
            line_row += "-";
        }
        group.push_back(TUI_TextGroup_Default(TUI_Text_Default(line_row)));
    }
    for (size_t i = 0; i < board.size; i++)
    {
        string board_row = "";
        if (i < 10) board_row += to_string(i);
        // NOTE(mattg): ASCII A starts at 65, and this branch assumes 10+
        else board_row += (char)(i + 55);
        board_row += "|";
        TUI_TextGroup row_group = TUI_TextGroup_Default(TUI_Text_Default(board_row));

        // NOTE(mattg): Doing a dynamic string color algo like this prevents the naive approach where
        // each board cell has a separate fg/bg ESC code, improving terminal rendering performance.
        string row = "";
        Color prev_fg = RESET;
        Color prev_bg = RESET;
        size_t j = 0;
        while (j < board.size)
        {
            char c = '\0';
            Color fg = RESET;
            Color bg = RESET;

            BShip_BoardValue value = BShip_Board_Get(board, i, j);
            switch (value)
            {
            case BSHIP_WATER:
                c = '~';
                bg = LIGHT_CYAN;
                fg = BLACK;
                break;
            case BSHIP_SHIP:
                c = 'S';
                fg = BLACK;
                bg = WHITE;
                break;
            case BSHIP_HIT:
                c = 'X';
                fg = BLACK;
                bg = LIGHT_YELLOW;
                break;
            case BSHIP_MISS:
                c = '*';
                fg = BLACK;
                bg = GRAY;
                break;
            case BSHIP_KILL:
                c = 'K';
                fg = WHITE;
                bg = LIGHT_RED;
                break;
            case BSHIP_DUPLICATE_HIT:
                c = '!';
                fg = BLACK;
                bg = LIGHT_YELLOW;
                break;
            case BSHIP_DUPLICATE_MISS:
                c = '!';
                fg = BLACK;
                bg = GRAY;
                break;
            case BSHIP_DUPLICATE_KILL:
                c = '!';
                fg = WHITE;
                bg = LIGHT_RED;
                break;
            }

            if (fg == prev_fg && bg == prev_bg)
            {
                row += c;
            }
            else
            {
                TUI_TextGroup_Add(&row_group, TUI_Text_New(row, {}, prev_fg, prev_bg));
                row = "";
                row += c;
                prev_fg = fg;
                prev_bg = bg;
            }

            j++;
        }
        TUI_TextGroup_Add(&row_group, TUI_Text_New(row, {}, prev_fg, prev_bg));
        group.push_back(row_group);
    }
}

void TUI_Store_Ship(BShip_Board board, BShip_Ship ship, BShip_BoardValue value)
{
    uint8_t row_multiplier = ship.direction == BSHIP_VERTICAL;
    uint8_t column_multiplier = ship.direction == BSHIP_HORIZONTAL;
    for (uint8_t i = 0; i < ship.length; i++)
    {
        uint8_t row = ship.row + (i * row_multiplier);
        uint8_t column = ship.column + (i * column_multiplier);
        BShip_Board_Set(board, row, column, value);
    }
}

typedef struct {
    vector<uint32_t> display_game_indexes;
    uint32_t display_game_index;
    uint32_t event_offset;
    bool game_stepping_over;
} TUI_GameStepState;

uint32_t TUI_GameStepState_GameIndex_Get(TUI_GameStepState *state)
{
    assert(state != NULL);
    if (state->display_game_indexes.size() > 0)
    {
        assert(state->display_game_index < state->display_game_indexes.size());
    }
    return state->display_game_indexes.at(state->display_game_index);
}

uint32_t TUI_GameStepState_Index_Start(TUI_GameStepState *state, BShip_MatchData match)
{
    assert(state != NULL);
    assert(match.game_indexes.buffer != NULL);
    uint32_t game_index = TUI_GameStepState_GameIndex_Get(state);
    assert(game_index < match.game_indexes.length);
    return match.game_indexes.buffer[game_index];
}

uint32_t TUI_GameStepState_Index_End(TUI_GameStepState *state, BShip_MatchData match)
{
    assert(state != NULL);
    assert(match.game_indexes.buffer != NULL);
    uint32_t game_index = TUI_GameStepState_GameIndex_Get(state);
    assert(game_index < match.game_indexes.length);
    return game_index == match.game_indexes.length-1
        ? match.events.length-1
        : (match.game_indexes.buffer[game_index+1])-1;
}

BShip_Event TUI_GameStepState_Event_Get(TUI_GameStepState *state, BShip_MatchData match)
{
    assert(state != NULL);
    assert(match.events.buffer != NULL);
    uint32_t game_event_index_start = TUI_GameStepState_Index_Start(state, match);
    uint32_t game_event_index_end = TUI_GameStepState_Index_End(state, match);
    uint32_t event_index = game_event_index_start + state->event_offset;
    if (event_index > game_event_index_end)
    {
        event_index = game_event_index_end;
    }
    assert(event_index < match.events.length);
    return match.events.buffer[event_index];
}

void TUI_GameStepState_NextGame(TUI_GameStepState *state)
{
    assert(state != NULL);
    if (state->display_game_indexes.size() == 0)
    {
        state->display_game_index = 0;
        return;
    }
    else if (state->display_game_index >= state->display_game_indexes.size())
    {
        state->display_game_index = state->display_game_indexes.size()-1;
    }
    else if (state->display_game_index < state->display_game_indexes.size()-1)
    {
        state->display_game_index++;
    }
}

void TUI_GameStepState_NextStep(TUI_GameStepState *state, BShip_MatchData match)
{
    assert(state != NULL);
    assert(match.events.buffer != NULL);
    uint32_t game_event_index_start = TUI_GameStepState_Index_Start(state, match);
    uint32_t game_event_index_end = TUI_GameStepState_Index_End(state, match);
    uint32_t event_index = game_event_index_start + state->event_offset;
    if (event_index < game_event_index_end)
    {
        state->event_offset++;
        return;
    }
    if (state->display_game_index < state->display_game_indexes.size()-1)
    {
        TUI_GameStepState_NextGame(state);
        state->event_offset = 0;
    }
}

void TUI_GameStepState_PreviousGame(TUI_GameStepState *state)
{
    assert(state != NULL);
    if (state->display_game_indexes.size() == 0)
    {
        state->display_game_index = 0;
        return;
    }
    else if (state->display_game_index >= state->display_game_indexes.size())
    {
        state->display_game_index = state->display_game_indexes.size()-1;
    }
    // NOTE(mattg): This is not an else if because we want this to compute every time
    // we do the upper bounds check.
    if (state->display_game_index > 0)
    {
        state->display_game_index--;
    }
}

void TUI_GameStepState_PreviousStep(TUI_GameStepState *state, BShip_MatchData match)
{
    assert(state != NULL);
    assert(match.events.buffer != NULL);
    assert(match.game_indexes.buffer != NULL);
    uint32_t game_event_index_start = TUI_GameStepState_Index_Start(state, match);
    uint32_t game_event_index_end = TUI_GameStepState_Index_End(state, match);
    uint32_t event_offset_bounds = game_event_index_end - game_event_index_start;
    if (state->event_offset > event_offset_bounds)
    {
        state->event_offset = event_offset_bounds;
    }
    if (state->event_offset > 0)
    {
        state->event_offset--;
        return;
    }
    if (state->display_game_index > 0)
    {
        TUI_GameStepState_PreviousGame(state);
        game_event_index_start = TUI_GameStepState_Index_Start(state, match);
        game_event_index_end = TUI_GameStepState_Index_End(state, match);
        state->event_offset = game_event_index_end - game_event_index_start;
    }
}

void TUI_GameStepState_Apply(TUI_GameStepState *state, BShip_MatchData match,
    BShip_Board ai1_board, BShip_Board ai2_board)
{
    assert(state != NULL);
    assert(match.game_indexes.buffer != NULL);
    assert(match.events.buffer != NULL);
    assert(ai1_board.buffer != NULL);
    assert(ai2_board.buffer != NULL);

    uint32_t game_event_index_start = TUI_GameStepState_Index_Start(state, match);
    uint32_t game_event_index_end = TUI_GameStepState_Index_End(state, match);
    uint32_t event_index = game_event_index_start + state->event_offset;

    memset(ai1_board.buffer, BSHIP_WATER, ai1_board.size * ai1_board.size);
    memset(ai2_board.buffer, BSHIP_WATER, ai2_board.size * ai2_board.size);

    uint32_t i = game_event_index_start;
    while (i <= event_index && i <= game_event_index_end && i < match.events.length)
    {
        BShip_Event event = match.events.buffer[i];
        if (event.type == BSHIP_EVENT_SHIP_PLACEMENT)
        {
            BShip_Ship ai1_ship = BShip_Ship_From_CompactShip(event.value.compact.ai1_ship);
            BShip_Ship ai2_ship = BShip_Ship_From_CompactShip(event.value.compact.ai2_ship);
            TUI_Store_Ship(ai1_board, ai1_ship, BSHIP_SHIP);
            TUI_Store_Ship(ai2_board, ai2_ship, BSHIP_SHIP);
        }
        else if (event.type == BSHIP_EVENT_SHOT_RESULT)
        {
            BShip_Shot ai1_shot = BShip_Shot_From_CompactShot(event.value.compact.ai1_shot);
            BShip_Shot ai2_shot = BShip_Shot_From_CompactShot(event.value.compact.ai2_shot);
            BShip_Board_Set(ai1_board, ai2_shot.row, ai2_shot.column, ai2_shot.value);
            BShip_Board_Set(ai2_board, ai1_shot.row, ai1_shot.column, ai1_shot.value);

            if (event.value.compact.ai1_ship > 0)
            {
                BShip_Ship ai1_dead_ship = BShip_Ship_From_CompactShip(event.value.compact.ai1_ship);
                TUI_Store_Ship(ai1_board, ai1_dead_ship, BSHIP_KILL);
            }
            if (event.value.compact.ai2_ship > 0)
            {
                BShip_Ship ai2_dead_ship = BShip_Ship_From_CompactShip(event.value.compact.ai2_ship);
                TUI_Store_Ship(ai2_board, ai2_dead_ship, BSHIP_KILL);
            }
        }
        i++;
    }
}

string TUI_Coordinates_Get(uint32_t row, uint32_t column)
{
    string coordinates = " @ [";
    if (row < 10)
    {
        coordinates += to_string(row);
    }
    else coordinates += (char)(row + 55);

    coordinates += ",";
    if (column < 10)
    {
        coordinates += to_string(column);
    }
    else coordinates += (char)(column + 55);
    coordinates += "]";
    return coordinates;
}

TUI_TextGroup TUI_TextGroup_Make_ShipPlacementEvent(BShip_Ship ship, BShip_PlayerNum player)
{
    Color player_color = player == BSHIP_PLAYER_1 ? PLAYER_1_COLOR : PLAYER_2_COLOR;
    string direction_string = ship.direction == BSHIP_HORIZONTAL ? "HORIZONTAL" : "VERTICAL";
    TUI_TextGroup group = TUI_TextGroup_Default(TUI_Text_New(direction_string, { BOLD }, player_color, RESET));

    string ship_event_string = TUI_Coordinates_Get(ship.row, ship.column) + " x " + to_string(ship.length);
    TUI_TextGroup_Add(&group, TUI_Text_New(ship_event_string, {}, player_color, RESET));
    return group;
}

TUI_TextGroup TUI_TextGroup_Make_ShotPlacementEvent(BShip_Shot shot, BShip_PlayerNum player)
{
    Color player_color = player == BSHIP_PLAYER_1 ? PLAYER_1_COLOR : PLAYER_2_COLOR;
    string shot_value_string = "";
    switch (shot.value)
    {
    case BSHIP_WATER:
    case BSHIP_SHIP:
        assert(false);
        break;
    case BSHIP_HIT:
        shot_value_string = "HIT";
        break;
    case BSHIP_DUPLICATE_HIT:
        shot_value_string = "DUPLICATE HIT";
        break;
    case BSHIP_MISS:
        shot_value_string = "MISS";
        break;
    case BSHIP_DUPLICATE_MISS:
        shot_value_string = "DUPLICATE MISS";
        break;
    case BSHIP_KILL:
        shot_value_string = "KILL";
        break;
    case BSHIP_DUPLICATE_KILL:
        shot_value_string = "DUPLICATE KILL";
        break;
    }
    TUI_TextGroup group = TUI_TextGroup_Default(TUI_Text_New(shot_value_string, { BOLD }, player_color, RESET));

    string shot_event_string = TUI_Coordinates_Get(shot.row, shot.column);
    TUI_TextGroup_Add(&group, TUI_Text_New(shot_event_string, {}, player_color, RESET));
    return group;
}

TUI_TextGroup TUI_TextGroup_Make_GameResultEvent(BShip_GameResult result, const string &name,
    BShip_PlayerNum player)
{
    Color player_color = player == BSHIP_PLAYER_1 ? PLAYER_1_COLOR : PLAYER_2_COLOR;
    TUI_TextGroup group = TUI_TextGroup_Default(TUI_Text_New(name, { BOLD }, player_color, RESET));
    string result_string = " ";
    switch (result)
    {
    case BSHIP_WIN:
        result_string += "won!";
        break;
    case BSHIP_LOSS:
        result_string += "lost";
        break;
    case BSHIP_TIE:
        result_string += "tied";
        break;
    }
    TUI_TextGroup_Add(&group, TUI_Text_Default(result_string));
    return group;
}

void TUI_TextGroups_Add_EventDescriptions(vector<TUI_TextGroup> &ai1_group, vector<TUI_TextGroup> &ai2_group,
    BShip_MatchData match, BShip_Event event)
{
    BShip_Ship ai1_ship = {}, ai2_ship = {};
    BShip_Shot ai1_shot = {}, ai2_shot = {};
    BShip_GameResult ai1_result = BSHIP_WIN, ai2_result = BSHIP_WIN;
    switch (event.type)
    {
    case BSHIP_EVENT_NONE:
    case BSHIP_EVENT_GAME_START:
        // NOTE(mattg): These events display nothing
        break;
    case BSHIP_EVENT_SHIP_PLACEMENT:
        ai1_group.push_back(TUI_TextGroup_Default(TUI_Text_Default("")));
        ai2_group.push_back(TUI_TextGroup_Default(TUI_Text_Default("")));
        ai1_ship = BShip_Ship_From_CompactShip(event.value.compact.ai1_ship);
        ai2_ship = BShip_Ship_From_CompactShip(event.value.compact.ai2_ship);
        ai1_group.push_back(TUI_TextGroup_Make_ShipPlacementEvent(ai1_ship, BSHIP_PLAYER_1));
        ai2_group.push_back(TUI_TextGroup_Make_ShipPlacementEvent(ai2_ship, BSHIP_PLAYER_2));
        break;
    case BSHIP_EVENT_SHOT_RESULT:
        ai1_group.push_back(TUI_TextGroup_Default(TUI_Text_Default("")));
        ai2_group.push_back(TUI_TextGroup_Default(TUI_Text_Default("")));
        ai1_shot = BShip_Shot_From_CompactShot(event.value.compact.ai1_shot);
        ai2_shot = BShip_Shot_From_CompactShot(event.value.compact.ai2_shot);
        if (event.value.compact.ai1_ship > 0)
        {
            ai2_shot.value = BSHIP_KILL;
        }
        if (event.value.compact.ai2_ship > 0)
        {
            ai1_shot.value = BSHIP_KILL;
        }
        ai1_group.push_back(TUI_TextGroup_Make_ShotPlacementEvent(ai2_shot, BSHIP_PLAYER_2));
        ai2_group.push_back(TUI_TextGroup_Make_ShotPlacementEvent(ai1_shot, BSHIP_PLAYER_1));
        break;
    case BSHIP_EVENT_GAME_RESULT:
        ai1_group.push_back(TUI_TextGroup_Default(TUI_Text_Default("")));
        ai2_group.push_back(TUI_TextGroup_Default(TUI_Text_Default("")));
        ai1_result = event.value.ai1_game_result;
        ai2_result = ai1_result == BSHIP_TIE ? BSHIP_TIE :
            ai1_result == BSHIP_WIN ? BSHIP_LOSS : BSHIP_WIN;
        ai1_group.push_back(TUI_TextGroup_Make_GameResultEvent(ai1_result, match.ai1.name, BSHIP_PLAYER_1));
        ai2_group.push_back(TUI_TextGroup_Make_GameResultEvent(ai2_result, match.ai2.name, BSHIP_PLAYER_2));
        break;
    }
}
typedef enum {
    STATS_TEXT,
    STATS_RESULT,
    STATS_PERCENT,
    STATS_RATIO,
} TUI_StatsType;

typedef struct {
    TUI_Text v1;
    TUI_Text v2;
    TUI_Text v3;
} TUI_AIGameStat;

typedef struct {
    TUI_Text key;
    TUI_AIGameStat ai1;
    TUI_AIGameStat ai2;
    TUI_StatsType type;
} TUI_GameStat;

typedef struct {
    vector<TUI_GameStat> stats;
    uint32_t widest_key;
    uint32_t widest_v1;
    uint32_t widest_v2;
    uint32_t widest_v3;
} TUI_GameStats;

typedef struct {
    TUI_Text v1;
    TUI_Text v2;
    TUI_Text v3;
    TUI_Text v4;
} TUI_AIMatchStat;

typedef struct {
    TUI_Text key;
    TUI_AIMatchStat ai1;
    TUI_AIMatchStat ai2;
    TUI_StatsType type;
} TUI_MatchStat;

typedef struct {
    vector<TUI_GameStats> games;
    vector<TUI_GameStat> result_stats;
    vector<TUI_MatchStat> stats;
    uint32_t widest_key;
    uint32_t widest_v1;
    uint32_t widest_v2;
    uint32_t widest_v3;
    uint32_t widest_v4;
} TUI_MatchStats;

TUI_Text TUI_Text_From_GameResult(BShip_GameResult game_result)
{
    switch (game_result)
    {
    case BSHIP_WIN:
        return TUI_Text_New("WIN", {BOLD}, GREEN, RESET);
        break;
    case BSHIP_LOSS:
        return TUI_Text_New("LOSS", {BOLD}, RED, RESET);
        break;
    case BSHIP_TIE:
        return TUI_Text_New("TIE", {}, RESET, RESET);
        break;
    }
    return TUI_Text_Default("");
}

string String_From_Float(float value)
{
    stringstream strm = {};
    strm << fixed << setprecision(2) << value;
    string str = strm.str();
    while (str.back() == '0')
    {
        str.pop_back();
    }
    if (str.back() == '.')
    {
        str.pop_back();
    }
    return str;
}

TUI_Text TUI_Text_From_Percent(float percent)
{
    return TUI_Text_Default(String_From_Float(percent) + "%");
}

TUI_Text TUI_Text_From_Ratio(float ratio)
{
    // NOTE(mattg): infinity is hit when there is a 0 in the denominator.
    // Which, in our current case the 0 means 0 missed shots, so it's a perfect game.
    if (isinf(ratio))
    {
        return TUI_Text_Default("Perfect");
    }
    return TUI_Text_Default(String_From_Float(ratio) + ":1");
}

TUI_Text TUI_Text_From_Stddev(float stddev)
{
    string stddev_str = "±" + String_From_Float(stddev);
    return TUI_Text_Default(stddev_str);
}

TUI_AIGameStat TUI_AIGameStat_From_BShip_DerivedGameStat(BShip_DerivedGameStat game_stat, TUI_StatsType type)
{
    TUI_AIGameStat stat = {};
    switch (type)
    {
    case STATS_TEXT:
        break;
    case STATS_RESULT:
        stat.v1 = TUI_Text_From_GameResult((BShip_GameResult)game_stat.numerator);
        break;
    case STATS_PERCENT:
        stat.v1 = TUI_Text_Default(to_string(game_stat.numerator));
        stat.v2 = TUI_Text_Default(to_string(game_stat.denominator));
        stat.v3 = TUI_Text_From_Percent(game_stat.value);
        break;
    case STATS_RATIO:
        stat.v1 = TUI_Text_Default(to_string(game_stat.numerator));
        stat.v2 = TUI_Text_Default(to_string(game_stat.denominator));
        stat.v3 = TUI_Text_From_Ratio(game_stat.value);
        break;
    }
    return stat;
}

TUI_GameStat TUI_GameStat_From_BShip_DerivedGameStat(BShip_DerivedGameStat ai1_stat, BShip_DerivedGameStat ai2_stat,
    string key, TUI_StatsType type)
{
    TUI_GameStat stat = {
        .key = TUI_Text_New(key, { BOLD }, RESET, RESET),
        .ai1 = TUI_AIGameStat_From_BShip_DerivedGameStat(ai1_stat, type),
        .ai2 = TUI_AIGameStat_From_BShip_DerivedGameStat(ai2_stat, type),
        .type = type,
    };
    return stat;
}

TUI_AIMatchStat TUI_AIMatchStat_From_BShip_DerivedMatchStat(BShip_DerivedMatchStat match_stat, TUI_StatsType type)
{
    TUI_AIMatchStat stat = {};
    switch (type)
    {
    case STATS_TEXT:
    case STATS_RESULT:
        break;
    case STATS_PERCENT:
        stat.v1 = TUI_Text_From_Percent(match_stat.min);
        stat.v2 = TUI_Text_From_Percent(match_stat.max);
        stat.v3 = TUI_Text_From_Percent(match_stat.avg);
        stat.v4 = TUI_Text_From_Stddev(match_stat.stddev);
        break;
    case STATS_RATIO:
        stat.v1 = TUI_Text_From_Ratio(match_stat.min);
        stat.v2 = TUI_Text_From_Ratio(match_stat.max);
        stat.v3 = TUI_Text_From_Ratio(match_stat.avg);
        stat.v4 = TUI_Text_From_Stddev(match_stat.stddev);
        break;
    }
    return stat;
}

TUI_MatchStat TUI_MatchStat_From_BShip_DerivedMatchStat(BShip_DerivedMatchStat ai1_stat, BShip_DerivedMatchStat ai2_stat,
    string key, TUI_StatsType type)
{
    TUI_MatchStat stat = {
        .key = TUI_Text_New(key, { BOLD }, RESET, RESET),
        .ai1 = TUI_AIMatchStat_From_BShip_DerivedMatchStat(ai1_stat, type),
        .ai2 = TUI_AIMatchStat_From_BShip_DerivedMatchStat(ai2_stat, type),
        .type = type,
    };
    return stat;
}

TUI_MatchStats TUI_MatchStats_From_BShip_DerivedMatchStats(BShip_DerivedMatchStats match_stats)
{
    TUI_MatchStats stats = {};
    stats.result_stats.push_back(
        TUI_GameStat_From_BShip_DerivedGameStat(match_stats.ai1.wins, match_stats.ai2.wins, "Wins", STATS_PERCENT)
    );
    stats.result_stats.push_back(
        TUI_GameStat_From_BShip_DerivedGameStat(match_stats.ai1.losses, match_stats.ai2.losses, "Losses", STATS_PERCENT)
    );
    if (match_stats.ai1.ties.numerator > 0 || match_stats.ai2.ties.numerator > 0)
    {
        stats.result_stats.push_back(
            TUI_GameStat_From_BShip_DerivedGameStat(match_stats.ai1.ties, match_stats.ai2.ties, "Ties", STATS_PERCENT)
        );
    }
    if (match_stats.ai1.perfect_games.numerator > 0 || match_stats.ai2.perfect_games.numerator > 0)
    {
        stats.result_stats.push_back(
            TUI_GameStat_From_BShip_DerivedGameStat(match_stats.ai1.perfect_games, match_stats.ai2.perfect_games,
                "Perfect games", STATS_PERCENT)
        );
    }

    TUI_MatchStat title_stat = {
        .key = TUI_Text_Default(""),
        .ai1 = {
            .v1 = TUI_Text_New("Min", { BOLD }, RESET, RESET),
            .v2 = TUI_Text_New("Max", { BOLD }, RESET, RESET),
            .v3 = TUI_Text_New("Avg", { BOLD }, RESET, RESET),
            .v4 = TUI_Text_New("Stddev", { BOLD }, RESET, RESET),
        },
        .ai2 = {
            .v1 = TUI_Text_New("Min", { BOLD }, RESET, RESET),
            .v2 = TUI_Text_New("Max", { BOLD }, RESET, RESET),
            .v3 = TUI_Text_New("Avg", { BOLD }, RESET, RESET),
            .v4 = TUI_Text_New("Stddev", { BOLD }, RESET, RESET),
        },
        .type = STATS_TEXT,
    };
    stats.stats.push_back(title_stat);
    stats.stats.push_back(
        TUI_MatchStat_From_BShip_DerivedMatchStat(match_stats.ai1.hit_rate, match_stats.ai2.hit_rate,
            "Hit rate", STATS_PERCENT)
    );
    if (match_stats.ai1.duplicate_shots.max > 0.0f || match_stats.ai2.duplicate_shots.max > 0.0f)
    {
        stats.stats.push_back(
            TUI_MatchStat_From_BShip_DerivedMatchStat(match_stats.ai1.duplicate_shots, match_stats.ai2.duplicate_shots,
                "Duplicate shots", STATS_PERCENT)
        );
    }
    stats.stats.push_back(
        TUI_MatchStat_From_BShip_DerivedMatchStat(match_stats.ai1.useful_shot_ratio, match_stats.ai2.useful_shot_ratio,
            "Useful shot ratio", STATS_RATIO)
    );
    stats.stats.push_back(
        TUI_MatchStat_From_BShip_DerivedMatchStat(match_stats.ai1.amount_board_shot, match_stats.ai2.amount_board_shot,
            "Amount board shot", STATS_PERCENT)
    );
    stats.stats.push_back(
        TUI_MatchStat_From_BShip_DerivedMatchStat(match_stats.ai1.ships_killed, match_stats.ai2.ships_killed,
            "Ships killed", STATS_PERCENT)
    );
    stats.stats.push_back(
        TUI_MatchStat_From_BShip_DerivedMatchStat(match_stats.ai1.ship_cells_hit, match_stats.ai2.ship_cells_hit,
            "Ship cells hit", STATS_PERCENT)
    );
    for (size_t i = 0; i < stats.result_stats.size(); i++)
    {
        TUI_GameStat gs = stats.result_stats.at(i);
        MAX_SET(stats.widest_key, TUI_Text_Size(gs.key));
        MAX_SET(stats.widest_v1, TUI_Text_Size(gs.ai1.v1));
        MAX_SET(stats.widest_v1, TUI_Text_Size(gs.ai2.v1));
        MAX_SET(stats.widest_v2, TUI_Text_Size(gs.ai1.v2));
        MAX_SET(stats.widest_v2, TUI_Text_Size(gs.ai2.v2));
        MAX_SET(stats.widest_v3, TUI_Text_Size(gs.ai1.v3));
        MAX_SET(stats.widest_v3, TUI_Text_Size(gs.ai2.v3));
    }
    for (size_t i = 0; i < stats.stats.size(); i++)
    {
        TUI_MatchStat ms = stats.stats.at(i);
        MAX_SET(stats.widest_key, TUI_Text_Size(ms.key));
        MAX_SET(stats.widest_v1, TUI_Text_Size(ms.ai1.v1));
        MAX_SET(stats.widest_v1, TUI_Text_Size(ms.ai2.v1));
        MAX_SET(stats.widest_v2, TUI_Text_Size(ms.ai1.v2));
        MAX_SET(stats.widest_v2, TUI_Text_Size(ms.ai2.v2));
        MAX_SET(stats.widest_v3, TUI_Text_Size(ms.ai1.v3));
        MAX_SET(stats.widest_v3, TUI_Text_Size(ms.ai2.v3));
        MAX_SET(stats.widest_v4, TUI_Text_Size(ms.ai1.v4));
        MAX_SET(stats.widest_v4, TUI_Text_Size(ms.ai2.v4));
    }
    for (size_t i = 0; i < match_stats.game_stats.length; i++)
    {
        BShip_DerivedGameStats game = match_stats.game_stats.buffer[i];
        TUI_GameStats game_stats = {};
        game_stats.stats.push_back(
            TUI_GameStat_From_BShip_DerivedGameStat(game.ai1.result, game.ai2.result, "Result", STATS_RESULT)
        );
        game_stats.stats.push_back(
            TUI_GameStat_From_BShip_DerivedGameStat(game.ai1.hit_rate, game.ai2.hit_rate, "Hit rate", STATS_PERCENT)
        );
        if (game.ai1.hit_rate.numerator > 0 || game.ai2.hit_rate.numerator > 0)
        {
            game_stats.stats.push_back(
                TUI_GameStat_From_BShip_DerivedGameStat(game.ai1.duplicate_shots, game.ai2.duplicate_shots,
                    "Duplicate shots", STATS_PERCENT)
            );
        }
        game_stats.stats.push_back(
            TUI_GameStat_From_BShip_DerivedGameStat(game.ai1.useful_shot_ratio, game.ai2.useful_shot_ratio,
                "Useful shot ratio", STATS_RATIO)
        );
        game_stats.stats.push_back(
            TUI_GameStat_From_BShip_DerivedGameStat(game.ai1.ships_killed, game.ai2.ships_killed,
                "Ships killed", STATS_PERCENT)
        );
        game_stats.stats.push_back(
            TUI_GameStat_From_BShip_DerivedGameStat(game.ai1.ship_cells_hit, game.ai2.ship_cells_hit,
                "Ship cells hit", STATS_PERCENT)
        );
        for (size_t j = 0; j < game_stats.stats.size(); j++)
        {
            TUI_GameStat gs = game_stats.stats.at(j);
            MAX_SET(game_stats.widest_key, TUI_Text_Size(gs.key));
            MAX_SET(game_stats.widest_v1, TUI_Text_Size(gs.ai1.v1));
            MAX_SET(game_stats.widest_v1, TUI_Text_Size(gs.ai2.v1));
            MAX_SET(game_stats.widest_v2, TUI_Text_Size(gs.ai1.v2));
            MAX_SET(game_stats.widest_v2, TUI_Text_Size(gs.ai2.v2));
            MAX_SET(game_stats.widest_v3, TUI_Text_Size(gs.ai1.v3));
            MAX_SET(game_stats.widest_v3, TUI_Text_Size(gs.ai2.v3));
        }
        stats.games.push_back(game_stats);
    }

    return stats;
}

TUI_TextGroup TUI_TextGroup_AIGameStat_Get(TUI_AIGameStat stat, TUI_StatsType type, uint32_t column,
    uint32_t widest_v1, uint32_t widest_v2, uint32_t widest_v3)
{
    TUI_TextGroup group = {
        .text = {},
        .column = column,
    };
    if (type == STATS_RESULT)
    {
        TUI_TextGroup_Add(&group, stat.v1);
    }
    else
    {
        size_t v1_size = TUI_Text_Size(stat.v1);
        size_t leftover = v1_size < widest_v1 ? widest_v1 - v1_size : 0;
        if (leftover)
        {
            TUI_TextGroup_Add(&group, TUI_Text_Default(string(leftover, ' ')));
        }
        TUI_TextGroup_Add(&group, stat.v1);
        TUI_TextGroup_Add(&group, TUI_Text_Default("/"));
        TUI_TextGroup_Add(&group, stat.v2);

        // NOTE(mattg): both values may need padding between each other (and 2 spaces),
        // so add all spaces all at once.
        size_t v2_size = TUI_Text_Size(stat.v2);
        leftover = v2_size < widest_v2 ? widest_v2 - v2_size : 0;
        size_t v3_size = TUI_Text_Size(stat.v3);
        leftover += v3_size < widest_v3 ? widest_v3 - v3_size : 0;
        leftover += 2;
        TUI_TextGroup_Add(&group, TUI_Text_Default(string(leftover, ' ')));
        TUI_TextGroup_Add(&group, stat.v3);
    }
    return group;
}

TUI_TextGroup TUI_TextGroup_AIMatchStat_Get(TUI_AIMatchStat stat, TUI_StatsType type, uint32_t column,
    uint32_t widest_v1, uint32_t widest_v2, uint32_t widest_v3)
{
    TUI_TextGroup group = {
        .text = {},
        .column = column,
    };
    assert (type != STATS_RESULT);
    size_t v1_size = TUI_Text_Size(stat.v1);
    size_t leftover = v1_size < widest_v1 ? widest_v1 - v1_size : 0;
    if (leftover)
    {
        TUI_TextGroup_Add(&group, TUI_Text_Default(string(leftover, ' ')));
    }
    TUI_TextGroup_Add(&group, stat.v1);
    TUI_TextGroup_Add(&group, TUI_Text_Default("-"));
    TUI_TextGroup_Add(&group, stat.v2);

    // NOTE(mattg): both values may need padding between each other (and 2 spaces),
    // so add all spaces all at once.
    size_t v2_size = TUI_Text_Size(stat.v2);
    leftover = v2_size < widest_v2 ? widest_v2 - v2_size : 0;
    size_t v3_size = TUI_Text_Size(stat.v3);
    leftover += v3_size < widest_v3 ? widest_v3 - v3_size : 0;
    leftover += 2;
    TUI_TextGroup_Add(&group, TUI_Text_Default(string(leftover, ' ')));
    TUI_TextGroup_Add(&group, stat.v3);
    TUI_TextGroup_Add(&group, TUI_Text_Default(string(2, ' ')));
    TUI_TextGroup_Add(&group, stat.v4);
    return group;
}

void TUI_GameStats_Display(TUI_Window *window, string ai1_name, string ai2_name, TUI_GameStats stats)
{
    assert(window != NULL);

    TUI_Text game_stats_text = TUI_Text_New("Game Stats", { BOLD }, RESET, RESET);
    MAX_SET(stats.widest_key, TUI_Text_Size(game_stats_text));
    TUI_Text ai1_text = TUI_Text_New(ai1_name, { BOLD }, TUI_Player_Color_Get(BSHIP_PLAYER_1), RESET);
    TUI_Text ai2_text = TUI_Text_New(ai2_name, { BOLD }, TUI_Player_Color_Get(BSHIP_PLAYER_2), RESET);
    uint32_t player1_column = 0;
    uint32_t player2_column = 0;
    uint32_t max_stats_width = 0;
    {
        int key_width = (int)stats.widest_key;
        uint32_t ai1_name_size = TUI_Text_Size(ai1_text);
        uint32_t ai2_name_size = TUI_Text_Size(ai2_text);
        uint32_t key_spacer = 4;
        uint32_t player_spacer = 3;
        uint32_t player_stats_width = stats.widest_v1 + 1 + stats.widest_v2 + 2 + stats.widest_v3;
        uint32_t player1_width = player_stats_width > ai1_name_size ? player_stats_width : ai1_name_size;
        uint32_t player2_width = player_stats_width > ai2_name_size ? player_stats_width : ai2_name_size;
        uint32_t total_stats_width = stats.widest_key + key_spacer + player1_width + player_spacer + player2_width;
        if (total_stats_width > window->size.width)
        {
            int space_to_remove = total_stats_width - window->size.width;
            assert(space_to_remove > 0);
            if (key_width > space_to_remove)
            {
                key_width -= space_to_remove;
                assert(key_width > 0);
                space_to_remove = 0;
            }
            else
            {
                space_to_remove -= key_width;
                assert(space_to_remove >= 0);
                key_width = 0;
            }
            if (space_to_remove > 0 && player1_width > player_stats_width)
            {
                int player1_space_available = player1_width - player_stats_width;
                if (player1_space_available > space_to_remove)
                {
                    player1_space_available -= space_to_remove;
                    space_to_remove = 0;
                    player1_width = player_stats_width + player1_space_available;
                }
                else
                {
                    space_to_remove -= player1_space_available;
                    player1_width = player_stats_width;
                }
            }
            if (space_to_remove > 0 && player2_width > player_stats_width)
            {
                int player2_space_available = player2_width - player_stats_width;
                if (player2_space_available > space_to_remove)
                {
                    player2_space_available -= space_to_remove;
                    space_to_remove = 0;
                    player2_width = player_stats_width + player2_space_available;
                }
                else
                {
                    space_to_remove -= player2_space_available;
                    player2_width = player_stats_width;
                }
            }
        }
        player1_column = (uint32_t)key_width + key_spacer;
        player2_column = player1_column + player1_width + player_spacer;
        max_stats_width = (uint32_t)key_width + key_spacer + player1_width + player_spacer + player2_width;
    }
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    TUI_Line player_line = TUI_Line_Default(TUI_TextGroup_Default(game_stats_text));
    TUI_Line_Add(&player_line, TUI_TextGroup_New(ai1_text, player1_column));
    TUI_Line_Add(&player_line, TUI_TextGroup_New(ai2_text, player2_column));
    TUI_Window_Add(window, player_line);
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(string(max_stats_width-1, '-')))));
    for (size_t i = 0; i < stats.stats.size(); i++)
    {
        TUI_GameStat stat = stats.stats.at(i);
        TUI_Line line = TUI_Line_Default(TUI_TextGroup_Default(stat.key));
        TUI_TextGroup ai1_group = TUI_TextGroup_AIGameStat_Get(stat.ai1, stat.type, player1_column, stats.widest_v1,
            stats.widest_v2, stats.widest_v3);
        TUI_TextGroup ai2_group = TUI_TextGroup_AIGameStat_Get(stat.ai2, stat.type, player2_column, stats.widest_v1,
            stats.widest_v2, stats.widest_v3);
        TUI_Line_Add(&line, ai1_group);
        TUI_Line_Add(&line, ai2_group);
        TUI_Window_Add(window, line);
    }
}

void TUI_MatchStats_Display(TUI_Window *window, string ai1_name, string ai2_name, TUI_MatchStats stats)
{
    assert(window != NULL);

    TUI_Text match_stats_text = TUI_Text_New("Match Stats", { BOLD }, RESET, RESET);
    MAX_SET(stats.widest_key, TUI_Text_Size(match_stats_text));
    TUI_Text ai1_text = TUI_Text_New(ai1_name, { BOLD }, TUI_Player_Color_Get(BSHIP_PLAYER_1), RESET);
    TUI_Text ai2_text = TUI_Text_New(ai2_name, { BOLD }, TUI_Player_Color_Get(BSHIP_PLAYER_2), RESET);
    uint32_t player1_column = 0;
    uint32_t player2_column = 0;
    uint32_t max_stats_width = 0;
    {
        int key_width = (int)stats.widest_key;
        uint32_t ai1_name_size = TUI_Text_Size(ai1_text);
        uint32_t ai2_name_size = TUI_Text_Size(ai2_text);
        uint32_t key_spacer = 4;
        uint32_t player_spacer = 3;
        uint32_t player_stats_width = stats.widest_v1 + 1 + stats.widest_v2 + 2 + stats.widest_v3 + 2 + stats.widest_v4;
        uint32_t player1_width = player_stats_width > ai1_name_size ? player_stats_width : ai1_name_size;
        uint32_t player2_width = player_stats_width > ai2_name_size ? player_stats_width : ai2_name_size;
        uint32_t total_stats_width = stats.widest_key + key_spacer + player1_width + player_spacer + player2_width;
        if (total_stats_width > window->size.width)
        {
            int space_to_remove = total_stats_width - window->size.width;
            assert(space_to_remove > 0);
            if (key_width > space_to_remove)
            {
                key_width -= space_to_remove;
                assert(key_width > 0);
                space_to_remove = 0;
            }
            else
            {
                space_to_remove -= key_width;
                assert(space_to_remove >= 0);
                key_width = 0;
            }
            if (space_to_remove > 0 && player1_width > player_stats_width)
            {
                int player1_space_available = player1_width - player_stats_width;
                if (player1_space_available > space_to_remove)
                {
                    player1_space_available -= space_to_remove;
                    space_to_remove = 0;
                    player1_width = player_stats_width + player1_space_available;
                }
                else
                {
                    space_to_remove -= player1_space_available;
                    player1_width = player_stats_width;
                }
            }
            if (space_to_remove > 0 && player2_width > player_stats_width)
            {
                int player2_space_available = player2_width - player_stats_width;
                if (player2_space_available > space_to_remove)
                {
                    player2_space_available -= space_to_remove;
                    space_to_remove = 0;
                    player2_width = player_stats_width + player2_space_available;
                }
                else
                {
                    space_to_remove -= player2_space_available;
                    player2_width = player_stats_width;
                }
            }
        }
        player1_column = (uint32_t)key_width + key_spacer;
        player2_column = player1_column + player1_width + player_spacer;
        max_stats_width = (uint32_t)key_width + key_spacer + player1_width + player_spacer + player2_width;
    }
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    TUI_Line player_line = TUI_Line_Default(TUI_TextGroup_Default(match_stats_text));
    TUI_Line_Add(&player_line, TUI_TextGroup_New(ai1_text, player1_column));
    TUI_Line_Add(&player_line, TUI_TextGroup_New(ai2_text, player2_column));
    TUI_Window_Add(window, player_line);
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(string(max_stats_width-1, '-')))));
    for (size_t i = 0; i < stats.result_stats.size(); i++)
    {
        TUI_GameStat stat = stats.result_stats.at(i);
        TUI_Line line = TUI_Line_Default(TUI_TextGroup_Default(stat.key));
        TUI_TextGroup ai1_group = TUI_TextGroup_AIGameStat_Get(stat.ai1, stat.type, player1_column, stats.widest_v1,
            stats.widest_v2, stats.widest_v3);
        TUI_TextGroup ai2_group = TUI_TextGroup_AIGameStat_Get(stat.ai2, stat.type, player2_column, stats.widest_v1,
            stats.widest_v2, stats.widest_v3);
        TUI_Line_Add(&line, ai1_group);
        TUI_Line_Add(&line, ai2_group);
        TUI_Window_Add(window, line);
    }
    for (size_t i = 0; i < stats.stats.size(); i++)
    {
        TUI_MatchStat stat = stats.stats.at(i);
        TUI_Line line = TUI_Line_Default(TUI_TextGroup_Default(stat.key));
        TUI_TextGroup ai1_group = TUI_TextGroup_AIMatchStat_Get(stat.ai1, stat.type, player1_column, stats.widest_v1,
            stats.widest_v2, stats.widest_v3);
        TUI_TextGroup ai2_group = TUI_TextGroup_AIMatchStat_Get(stat.ai2, stat.type, player2_column, stats.widest_v1,
            stats.widest_v2, stats.widest_v3);
        TUI_Line_Add(&line, ai1_group);
        TUI_Line_Add(&line, ai2_group);
        TUI_Window_Add(window, line);
    }
}

void TUI_GameStepState_Display(TUI_Window *window, TUI_GameStepState *state, BShip_MatchData match,
    TUI_MatchStats stats, BShip_Board ai1_board, BShip_Board ai2_board)
{
    string ai1_name = match.ai1.name, ai2_name = match.ai2.name;
    // NOTE(mattg): 2 for the number and column line
    size_t board_display_width = 2 + match.board_size;
    size_t name_display_width = ai1_name.size() > ai2_name.size() ? ai1_name.size() : ai2_name.size();
    size_t total_display_width = board_display_width > name_display_width ? board_display_width : name_display_width;

    uint32_t game_index = TUI_GameStepState_GameIndex_Get(state);
    uint32_t game_num = game_index + 1;
    string game_num_str = "Game #" + to_string(game_num);
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(game_num_str))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));

    // we do it this way so that we can put the board wherever we need to depending on the screen size
    vector<TUI_TextGroup> ai1_group = {};
    vector<TUI_TextGroup> ai2_group = {};

    TUI_Board_Add(ai1_group, match.ai1.name, ai1_board, BSHIP_PLAYER_1);
    TUI_Board_Add(ai2_group, match.ai2.name, ai2_board, BSHIP_PLAYER_2);

    BShip_Event event = TUI_GameStepState_Event_Get(state, match);
    TUI_TextGroups_Add_EventDescriptions(ai1_group, ai2_group, match, event);

    assert(ai1_group.size() == ai2_group.size());

    if (window->size.width > (total_display_width * 2)+1)
    {
        // NOTE(mattg): left-right board display
        size_t leftover_width = window->size.width - (board_display_width * 2) - 1;
        leftover_width = leftover_width < 15 ? leftover_width : 15;
        size_t board2_column_offset = board_display_width + leftover_width;
        for (size_t i = 0; i < ai1_group.size(); i++)
        {
            TUI_Line line = TUI_Line_Default(ai1_group.at(i));
            ai2_group.at(i).column += board2_column_offset;
            TUI_Line_Add(&line, ai2_group.at(i));
            TUI_Window_Add(window, line);
        }
    }
    else
    {
        // NOTE(mattg): top-bottom board display
        for (size_t i = 0; i < ai1_group.size(); i++)
        {
            TUI_Window_Add(window, TUI_Line_Default(ai1_group.at(i)));
        }
        TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
        for (size_t i = 0; i < ai2_group.size(); i++)
        {
            TUI_Window_Add(window, TUI_Line_Default(ai2_group.at(i)));
        }
    }
    if (event.type == BSHIP_EVENT_GAME_RESULT)
    {
        TUI_GameStats_Display(window, ai1_name, ai2_name, stats.games.at(game_index));
    }
}

bool TUI_GameStepState_Input(TUI_GameStepState *state, TUI_Input input,
    BShip_MatchData match, bool manual_stepping)
{
    uint32_t game_event_index_start = 0;
    uint32_t game_event_index_end = 0;
    switch (input.type)
    {
    case INPUT_ESC:
        return true;
        break;
    case INPUT_ENTER:
        if (state->display_game_indexes.size() > 0)
        {
            // jump to the end of the events
            state->display_game_index = state->display_game_indexes.size()-1;
            game_event_index_start = TUI_GameStepState_Index_Start(state, match);
            game_event_index_end = TUI_GameStepState_Index_End(state, match);
            state->event_offset = game_event_index_end - game_event_index_start;
            state->game_stepping_over = true;
        }
        return false;
        break;
    default:
        break;
    }
    if (manual_stepping && !state->game_stepping_over)
    {
        switch (input.type)
        {
        case INPUT_UP:
            TUI_GameStepState_PreviousGame(state);
            break;
        case INPUT_DOWN:
            TUI_GameStepState_NextGame(state);
            break;
        case INPUT_LEFT:
            TUI_GameStepState_PreviousStep(state, match);
            break;
        case INPUT_RIGHT:
            TUI_GameStepState_NextStep(state, match);
            break;
        default:
            break;
        }
    }
    return false;
}

void TUI_VS_Display(TUI_Window *window, TUI_TextGroup &ai1_vs_group, TUI_TextGroup &ai2_vs_group)
{
    TUI_Text vs_text = TUI_Text_New("  VS  ", { BOLD, ITALIC, }, RESET, RESET);
    size_t vs_text_len = TUI_Text_Size(vs_text);
    size_t ai1_text_group_len = TUI_TextGroup_Size(ai1_vs_group);
    size_t ai2_text_group_len = TUI_TextGroup_Size(ai2_vs_group);
    
    if (window->size.width > (vs_text_len + ai1_text_group_len + ai2_text_group_len))
    {
        // horizontal vs grouping
        TUI_TextGroup group = ai1_vs_group;
        TUI_TextGroup_Add(&group, vs_text);
        for (size_t i = 0; i < ai2_vs_group.text.size(); i++)
        {
            TUI_TextGroup_Add(&group, ai2_vs_group.text.at(i));
        }
        TUI_Window_Add(window, TUI_Line_Default(group));
        return;
    }
    // vertical vs stacking
    TUI_Window_Add(window, TUI_Line_Default(ai1_vs_group));

    size_t longest = ai1_text_group_len > ai2_text_group_len ? ai1_text_group_len : ai2_text_group_len;
    if (longest > vs_text_len)
    {
        size_t side_buffer_len = (longest - vs_text_len) / 2;
        assert(side_buffer_len < longest);
        TUI_Text side_buffer = TUI_Text_Default(string(side_buffer_len, '-'));
        TUI_TextGroup vs_group = TUI_TextGroup_Default(side_buffer);
        TUI_TextGroup_Add(&vs_group, vs_text);
        TUI_TextGroup_Add(&vs_group, side_buffer);
        TUI_Window_Add(window, TUI_Line_Default(vs_group));
    }
    else
    {
        TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(vs_text)));
    }

    TUI_Window_Add(window, TUI_Line_Default(ai2_vs_group));
}

TUI_TextGroup TUI_Player_VS_Get(BShip_AIMatchData match, BShip_PlayerNum player)
{
    string authors = match.authors;
    string name = match.name;
    TUI_TextGroup player_vs = TUI_TextGroup_Default(TUI_Text_Default(authors + "'s "));
    name += player == BSHIP_PLAYER_1 ? " (P1)" : " (P2)";
    TUI_TextGroup_Add(&player_vs,
        TUI_Text_New(name, { BOLD }, TUI_Player_Color_Get(player), RESET)
    );
    return player_vs;
}

uint64_t TUI_Now_MS(void)
{
    struct timespec ts {};
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

void TUI_Match_Display(BShip_MatchData match, TUI_MatchDisplayType type,
    uint64_t step_delay_ms, bool manual_stepping, bool debug)
{
    TUI_Window window = {};
    TUI_WindowSize_Get(&window.size);

    BShip_Arena arena = {};
    BShip_Arena_Initialize(&arena, 0);

    BShip_DerivedMatchStats match_stats = BShip_DerivedMatchStats_Get(&arena, match, type == TUI_MATCH_DISPLAY_NONE);
    TUI_MatchStats stats = TUI_MatchStats_From_BShip_DerivedMatchStats(match_stats);

    BShip_Board ai1_board = BShip_Board_Allocate(&arena, match.board_size);
    BShip_Board ai2_board = BShip_Board_Allocate(&arena, match.board_size);

    TUI_TextGroup ai1_vs_group = TUI_Player_VS_Get(match.ai1, BSHIP_PLAYER_1);
    TUI_TextGroup ai2_vs_group = TUI_Player_VS_Get(match.ai2, BSHIP_PLAYER_2);

    TUI_GameStepState state = {};
    if (type == TUI_MATCH_DISPLAY_NONE)
    {
        state.game_stepping_over = true;
    }
    switch (type)
    {
    case TUI_MATCH_DISPLAY_NONE:
        break;
    case TUI_MATCH_DISPLAY_LAST:
        if (match.game_indexes.length > 0) state.display_game_indexes.push_back(match.game_indexes.length-1);
        break;
    case TUI_MATCH_DISPLAY_ALL:
        for (size_t i = 0; i < match.game_indexes.length; i++)
        {
            state.display_game_indexes.push_back(i);
        }
        break;
    }

    TUI_Input input = {};

    uint64_t next_step_ms = TUI_Now_MS() + step_delay_ms;

    // Automatic stepping means wait for input
    if (!TUI_Window_Enter(&window, manual_stepping))
    {
        goto on_exit;
    }

    while (!TUI_Should_Close())
    {
        if (TUI_Should_Resize())
        {
            TUI_WindowSize_Get(&window.size);
        }
        TUI_Window_Reset(&window);

        if (debug)
        {
            TUI_Debug_Line_Add(&window, input);
        }

        TUI_VS_Display(&window, ai1_vs_group, ai2_vs_group);

        if (type != TUI_MATCH_DISPLAY_NONE)
        {
            TUI_GameStepState_Apply(&state, match, ai1_board, ai2_board);

            TUI_GameStepState_Display(&window, &state, match, stats, ai1_board, ai2_board);

            TUI_Window_Add(&window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
            if (!state.game_stepping_over)
            {
                if (manual_stepping)
                {
                    TUI_Window_Add(&window, TUI_Line_Default(TUI_TextGroup_Default(
                        TUI_Text_Default("← → h/l  Prev/Next event")
                    )));
                    TUI_Window_Add(&window, TUI_Line_Default(TUI_TextGroup_Default(
                        TUI_Text_Default("↑ ↓ j/k  Prev/Next game")
                    )));
                }
                TUI_Window_Add(&window, TUI_Line_Default(TUI_TextGroup_Default(
                    TUI_Text_Default("Enter    Jump to Stats")
                )));
                TUI_Window_Add(&window, TUI_Line_Default(TUI_TextGroup_Default(
                    TUI_Text_Default("Esc/q    Quit")
                )));
            }
        }
        if (state.game_stepping_over)
        {
            TUI_MatchStats_Display(&window, match.ai1.name, match.ai2.name, stats);
        }

        TUI_Window_Print(&window);

        uint64_t now_ms = TUI_Now_MS();
        int timeout_ms = -1;
        if (!manual_stepping)
        {
            timeout_ms = (int)(next_step_ms - now_ms);
        }

        input = TUI_Input_Get(manual_stepping, timeout_ms);
        TUI_Input_ScrollState_Get(&window, input);
        if (TUI_GameStepState_Input(&state, input, match, manual_stepping))
        {
            break;
        }

        if (!manual_stepping)
        {
            now_ms = TUI_Now_MS();
            uint64_t total_delay_ms = step_delay_ms;

            if (now_ms >= next_step_ms || input.type == INPUT_ENTER)
            {
                if (input.type != INPUT_ENTER)
                {
                    TUI_GameStepState_NextStep(&state, match);
                }
                BShip_Event event = TUI_GameStepState_Event_Get(&state, match);
                if (event.type == BSHIP_EVENT_GAME_RESULT)
                {
                    total_delay_ms = 5000; // 5 seconds
                }
                next_step_ms = now_ms += total_delay_ms;
            }
        }
    }

on_exit:
    TUI_Window_Exit(&window);
    BShip_Arena_Destroy(&arena);
    return;
}

