/**
 * @file options.cpp
 * @author Matthew Getgen
 * @brief Battleships TUI output results.
 * @date 2026-07-04
 */

#include "shared.cpp"
#include "../../lib/battleshipslib.h"

const Color PLAYER_1_COLOR = BLUE, PLAYER_2_COLOR = YELLOW;

Color TUI_Player_Color_Get(BShip_PlayerNum player)
{
    return player == BSHIP_PLAYER_1 ? PLAYER_1_COLOR : PLAYER_2_COLOR;
}

void TUI_TextGroup_Add_Board(vector<TUI_TextGroup> &group, const string &name,
    BShip_Board board, BShip_PlayerNum player)
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

        for (size_t j = 0; j < board.size; j++)
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
            string str = "";
            str += c;
            TUI_TextGroup_Add(&row_group, TUI_Text_New(str, {}, fg, bg));
        }
        group.push_back(row_group);
    }
}

typedef struct {
    uint32_t game_index;
    uint32_t event_index;
} TUI_GameStepState;

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

void TUI_GameStepState_Apply(TUI_GameStepState state, BShip_MatchData match,
    BShip_Board ai1_board, BShip_Board ai2_board)
{
    assert(state.game_index < match.game_indexes.length);
    assert(state.event_index < match.events.length);

    uint32_t game_start_event_index = match.game_indexes.buffer[state.game_index];
    assert(game_start_event_index < match.events.length);
    assert(game_start_event_index <= state.event_index);

    memset(ai1_board.buffer, BSHIP_WATER, ai1_board.size * ai1_board.size);
    memset(ai2_board.buffer, BSHIP_WATER, ai2_board.size * ai2_board.size);

    uint32_t event_index = game_start_event_index;
    while (event_index < match.events.length && event_index <= state.event_index)
    {
        BShip_Event event = match.events.buffer[event_index];
        if (event.type == BSHIP_EVENT_SHIP_PLACEMENT)
        {
            assert(event.value.indexes.ai1_ship_index < match.ai1.ships.length);
            assert(event.value.indexes.ai2_ship_index < match.ai2.ships.length);
            BShip_Ship ai1_ship = match.ai1.ships.buffer[event.value.indexes.ai1_ship_index];
            BShip_Ship ai2_ship = match.ai2.ships.buffer[event.value.indexes.ai2_ship_index];
            TUI_Store_Ship(ai1_board, ai1_ship, BSHIP_SHIP);
            TUI_Store_Ship(ai2_board, ai2_ship, BSHIP_SHIP);
        }
        else if (event.type == BSHIP_EVENT_SHOT_RESULT)
        {
            assert(event.value.indexes.ai1_shot_index < match.ai1.shots.length);
            assert(event.value.indexes.ai2_shot_index < match.ai2.shots.length);
            BShip_Shot ai1_shot = match.ai1.shots.buffer[event.value.indexes.ai1_shot_index];
            BShip_Shot ai2_shot = match.ai2.shots.buffer[event.value.indexes.ai2_shot_index];
            BShip_Board_Set(ai1_board, ai2_shot.row, ai2_shot.column, ai2_shot.value);
            BShip_Board_Set(ai2_board, ai1_shot.row, ai1_shot.column, ai1_shot.value);

            if (event.value.indexes.ai1_ship_index > 0)
            {
                assert(event.value.indexes.ai1_ship_index < match.ai1.ships.length);
                BShip_Ship ai1_dead_ship = match.ai1.ships.buffer[event.value.indexes.ai1_ship_index];
                TUI_Store_Ship(ai1_board, ai1_dead_ship, BSHIP_KILL);
            }
            if (event.value.indexes.ai2_ship_index > 0)
            {
                assert(event.value.indexes.ai2_ship_index < match.ai2.ships.length);
                BShip_Ship ai2_dead_ship = match.ai2.ships.buffer[event.value.indexes.ai2_ship_index];
                TUI_Store_Ship(ai2_board, ai2_dead_ship, BSHIP_KILL);
            }
        }
        else if (event.type == BSHIP_EVENT_GAME_START)
        {
            if (event_index > game_start_event_index) break;
        }
        else break;
        event_index++;
    }
}

TUI_TextGroup TUI_TextGroup_Make_ShipPlacementEvent(BShip_Ship ship, BShip_PlayerNum player)
{
    Color player_color = player == BSHIP_PLAYER_1 ? PLAYER_1_COLOR : PLAYER_2_COLOR;
    string direction_string = ship.direction == BSHIP_HORIZONTAL ? "HORIZONTAL" : "VERTICAL";
    TUI_TextGroup group = TUI_TextGroup_Default(TUI_Text_New(direction_string, { BOLD }, player_color, RESET));

    string ship_event_string = " @ [" + to_string(ship.row) + "," + to_string(ship.column) + "] x "
        + to_string(ship.length);
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

    string shot_event_string =" @ [" + to_string(shot.row) + "," + to_string(shot.column) + "]";
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
        assert(event.value.indexes.ai1_ship_index < match.ai1.ships.length);
        assert(event.value.indexes.ai2_ship_index < match.ai2.ships.length);
        ai1_ship = match.ai1.ships.buffer[event.value.indexes.ai1_ship_index];
        ai2_ship = match.ai2.ships.buffer[event.value.indexes.ai2_ship_index];
        ai1_group.push_back(TUI_TextGroup_Make_ShipPlacementEvent(ai1_ship, BSHIP_PLAYER_1));
        ai2_group.push_back(TUI_TextGroup_Make_ShipPlacementEvent(ai2_ship, BSHIP_PLAYER_2));
        break;
    case BSHIP_EVENT_SHOT_RESULT:
        assert(event.value.indexes.ai1_shot_index < match.ai1.shots.length);
        assert(event.value.indexes.ai2_shot_index < match.ai2.shots.length);
        ai1_shot = match.ai1.shots.buffer[event.value.indexes.ai1_shot_index];
        if (event.value.indexes.ai1_ship_index > 0)
        {
            ai1_shot.value = BSHIP_KILL;
        }
        ai2_shot = match.ai2.shots.buffer[event.value.indexes.ai2_shot_index];
        if (event.value.indexes.ai1_ship_index > 0)
        {
            ai1_shot.value = BSHIP_KILL;
        }
        ai1_group.push_back(TUI_TextGroup_Make_ShotPlacementEvent(ai2_shot, BSHIP_PLAYER_2));
        ai2_group.push_back(TUI_TextGroup_Make_ShotPlacementEvent(ai1_shot, BSHIP_PLAYER_1));
        break;
    case BSHIP_EVENT_GAME_RESULT:
        ai1_result = event.value.ai1_game_result;
        ai2_result = ai1_result == BSHIP_TIE ? BSHIP_TIE :
            ai1_result == BSHIP_WIN ? BSHIP_LOSS : BSHIP_WIN;
        ai1_group.push_back(TUI_TextGroup_Make_GameResultEvent(ai1_result, match.ai1.name, BSHIP_PLAYER_1));
        ai2_group.push_back(TUI_TextGroup_Make_GameResultEvent(ai2_result, match.ai2.name, BSHIP_PLAYER_2));
        break;
    }
}

void TUI_GameStepState_Display(TUI_Window *window, TUI_GameStepState state, BShip_MatchData match,
    BShip_Board ai1_board, BShip_Board ai2_board)
{
    string ai1_name = match.ai1.name, ai2_name = match.ai2.name;
    size_t board_display_width = match.board_size + 15;
    size_t name_display_width = ai1_name.size() + 15;
    size_t board2_column_offset = board_display_width > name_display_width
        ? board_display_width : name_display_width;

    uint32_t game_num = state.game_index + 1;
    string game_num_str = "Game #" + to_string(game_num);
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(game_num_str))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));

    // we do it this way so that we can put the board wherever we need to depending on the screen size
    vector<TUI_TextGroup> ai1_group = {};
    vector<TUI_TextGroup> ai2_group = {};

    TUI_TextGroup_Add_Board(ai1_group, match.ai1.name, ai1_board, BSHIP_PLAYER_1);
    TUI_TextGroup_Add_Board(ai2_group, match.ai2.name, ai2_board, BSHIP_PLAYER_2);

    assert(state.event_index < match.events.length);
    BShip_Event event = match.events.buffer[state.event_index];
    TUI_TextGroups_Add_EventDescriptions(ai1_group, ai2_group, match, event);

    assert(ai1_group.size() == ai2_group.size());

    for (size_t i = 0; i < ai1_group.size(); i++)
    {
        TUI_Line line = TUI_Line_Default(ai1_group.at(i));
        ai2_group.at(i).column += board2_column_offset;
        TUI_Line_Add(&line, ai2_group.at(i));
        TUI_Window_Add(window, line);
    }
}

bool TUI_GameStepState_Input(TUI_GameStepState *state, TUI_Input input,
    BShip_MatchData match, bool automatic_stepping)
{
    // true for both types
    if (input.type == INPUT_ESC)
    {
        return true;
    }
    if (automatic_stepping)
    {
        assert(state->event_index < match.events.length);
        if (state->event_index < match.events.length-1)
        {
            state->event_index++;

            BShip_Event event = match.events.buffer[state->event_index];
            if (event.type == BSHIP_EVENT_GAME_START)
            {
                state->game_index++;
            }
        }
    }
    else
    {
        // TODO(mattg): handle input and take a step in any direction.
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

void TUI_Match_Display(BShip_MatchData match, bool automatic_stepping, int32_t step_delay_ms, bool debug)
{
    TUI_Window window = {};
    TUI_WindowSize_Get(&window.size);

    BShip_Arena arena = {};
    BShip_Arena_Initialize(&arena, 0);

    BShip_Board ai1_board = BShip_Board_Allocate(&arena, match.board_size);
    BShip_Board ai2_board = BShip_Board_Allocate(&arena, match.board_size);

    TUI_TextGroup ai1_vs_group = TUI_Player_VS_Get(match.ai1, BSHIP_PLAYER_1);
    TUI_TextGroup ai2_vs_group = TUI_Player_VS_Get(match.ai2, BSHIP_PLAYER_2);

    TUI_GameStepState state = {};

    TUI_Input input = {};

    // Automatic stepping means wait for input
    if (!TUI_Window_Enter(&window, !automatic_stepping))
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

        TUI_GameStepState_Apply(state, match, ai1_board, ai2_board);

        TUI_VS_Display(&window, ai1_vs_group, ai2_vs_group);

        TUI_GameStepState_Display(&window, state, match, ai1_board, ai2_board);

        TUI_Window_Print(&window);

        int32_t delay_ms = step_delay_ms;
        if (state.event_index < match.events.length && 
            match.events.buffer[state.event_index].type == BSHIP_EVENT_GAME_RESULT)
        {
            delay_ms = 5000; // 5 seconds
        }
        input = TUI_Input_Get(!automatic_stepping, delay_ms);
        TUI_Input_ScrollState_Get(&window, input);
        if (TUI_GameStepState_Input(&state, input, match, automatic_stepping))
        {
            break;
        }
    }

on_exit:
    TUI_Window_Exit(&window);
    BShip_Arena_Destroy(&arena);
    return;
}

