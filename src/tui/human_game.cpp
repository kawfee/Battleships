/**
 * @file human_game.cpp
 * @author Matthew Getgen
 * @brief Battleships TUI output results.
 * @date 2026-07-04
 */

#include <optional>

#include "results.cpp"

typedef enum {
    HUMAN_NAME_INPUT,
    HUMAN_SHIP_INPUT,
    HUMAN_SHOT_INPUT,
    HUMAN_GAME_OVER,
} TUI_HumanGameStateType;

typedef struct {
    string name;
    uint8_t ship_count;
    vector<uint8_t> ship_lengths;
    vector<BShip_Ship> ships_placed;
    BShip_Ship current_ship;
    bool current_ship_invalid;
    BShip_Shot current_shot;
    bool has_ended_game;
    TUI_MatchStats stats;
    TUI_HumanGameStateType type;
} TUI_HumanGameState;

bool TUI_ShipFitsOnBoard(BShip_Ship ship, uint8_t board_size)
{
    uint8_t front = 0, end = board_size, check = 0;
    switch (ship.direction)
    {
    case BSHIP_HORIZONTAL:
        front = ship.column;
        end = ship.column + (ship.length - 1);
        check = ship.row;
        break;
    case BSHIP_VERTICAL:
        front = ship.row;
        end = ship.row + (ship.length - 1);
        check = ship.column;
    }
    return front < board_size && end < board_size && check < board_size;
}

bool TUI_ShipIsInvalid(BShip_Ship ship, BShip_Board board)
{
    assert(TUI_ShipFitsOnBoard(ship, board.size));
    uint8_t row_multiplier = ship.direction == BSHIP_VERTICAL;
    uint8_t column_multiplier = ship.direction == BSHIP_HORIZONTAL;
    for (uint8_t i = 0; i < ship.length; i++)
    {
        uint8_t row = ship.row + (i * row_multiplier);
        uint8_t column = ship.column + (i * column_multiplier);
        if (BShip_Board_Get(board, row, column) != BSHIP_WATER)
        {
            return true;
        }
    }
    return false;
}

void TUI_HumanGameState_NameInput_Display(TUI_Window *window, TUI_HumanGameState *state)
{
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    TUI_Text prompt = TUI_Text_Default("Please enter your name: ");
    TUI_TextGroup group = TUI_TextGroup_Default(prompt);
    TUI_TextGroup_Add(&group, TUI_Text_Default(state->name));
    TUI_TextGroup_Add(&group, TUI_Text_New(" ", { NEGATIVE_IMAGE }, RESET, RESET));
    if (state->name.size() >= BSHIP_MESSAGE_NAME_SIZE_MAX/2)
    {
        string space_remaining = "   " + to_string(state->name.size()) + "/" + to_string(BSHIP_MESSAGE_NAME_SIZE_MAX);
        TUI_TextGroup_Add(&group, TUI_Text_Default(space_remaining));
    }
    TUI_Window_Add(window, TUI_Line_Default(group));

    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Type     Edit"))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Bksp     Delete"))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Enter    Select"))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Esc      Quit"))));
}

bool TUI_HumanGameState_NameInput_Input(TUI_HumanGameState *state, TUI_Input *input, BShip_MatchData *match,
    BShip_Connection *conn, char *socket_path)
{
    // NOTE(mattg): This is the one case where we don't convert the text types
    switch (input->type)
    {
    case INPUT_NUM:
    case INPUT_TEXT:
    case INPUT_SPACE:
    case INPUT_PERIOD:
        if (state->name.size() < BSHIP_MESSAGE_NAME_SIZE_MAX) state->name.push_back(input->value);
        break;
    case INPUT_BACKSPACE:
        if (state->name.size() > 0) state->name.pop_back();
        break;
    case INPUT_ENTER:
    {
        state->type = HUMAN_SHIP_INPUT;
        BShip_HumanGame_Start(match, conn, socket_path,
            (char *)state->name.c_str(), state->name.size(), NULL, 0);
        if (!BShip_HumanGame_ShouldContinue(match))
        {
            state->type = HUMAN_GAME_OVER;
        }
        
        BShip_U8Array ship_lengths = BShip_HumanGame_GetShipLengths(match);
        state->ship_count = ship_lengths.length;
        if (ship_lengths.length > 0)
        {
            state->current_ship.row = 0;
            state->current_ship.column = 0;
            state->current_ship.length = ship_lengths.buffer[0];
            state->current_ship.direction = BSHIP_HORIZONTAL;

            for (size_t i = 1; i < ship_lengths.length; i++)
            {
                state->ship_lengths.push_back(ship_lengths.buffer[i]);
            }
        }
    } break;
    case INPUT_ESC:
        return true;
        break;
    default:
        break;
    }
    return false;
}

size_t TUI_HumanGame_ShipLengths_Add(vector<TUI_TextGroup> &group, const vector<uint8_t> &ship_lengths)
{
    if (ship_lengths.size() == 0)
    {
        return 0;
    }
    TUI_TextGroup ships_to_place = TUI_TextGroup_Default(TUI_Text_Default("Ships to place:"));
    size_t widest = TUI_TextGroup_Size(ships_to_place);
    group.push_back(ships_to_place);
    for (size_t i = 0; i < ship_lengths.size(); i++)
    {
        uint8_t length = ship_lengths.at(i);
        assert(length >= 3);
        string ship = "";
        ship += Char_From_TUI_ShipType(SHIP_TYPE_BOW_HORIZONTAL);
        for (size_t i = 1; i < (size_t)length-1; i++)
        {
            ship += Char_From_TUI_ShipType(SHIP_TYPE_BODY_HORIZONTAL);
        }
        ship += Char_From_TUI_ShipType(SHIP_TYPE_STERN_HORIZONTAL);
        TUI_TextGroup ship_group = TUI_TextGroup_Default(TUI_Text_Default(ship));
        size_t width = TUI_TextGroup_Size(ship_group);
        widest = widest > width ? widest : width;
        group.push_back(ship_group);
    }
    return widest;
}

void TUI_HumanGameState_ShipInput_Display(TUI_Window *window, TUI_HumanGameState *state,
    TUI_BoardState p1_board_state)
{
    TUI_BoardState_Reset(p1_board_state);

    for (size_t i = 0; i < state->ships_placed.size(); i++)
    {
        BShip_Ship ship = state->ships_placed.at(i);
        TUI_Store_Ship(p1_board_state.board, ship, BSHIP_SHIP);
        TUI_ShipTypeTable_Store_Ship(p1_board_state.ship_table, ship);
    }
    TUI_BoardDisplayTable_Set(p1_board_state);
    assert(TUI_ShipFitsOnBoard(state->current_ship, p1_board_state.board.size));
    uint8_t row_multiplier = state->current_ship.direction == BSHIP_VERTICAL;
    uint8_t column_multiplier = state->current_ship.direction == BSHIP_HORIZONTAL;
    for (uint8_t i = 0; i < state->current_ship.length; i++)
    {
        uint8_t row = state->current_ship.row + (i * row_multiplier);
        uint8_t column = state->current_ship.column + (i * column_multiplier);
        BShip_BoardValue value = BShip_Board_Get(p1_board_state.board, row, column);
        TUI_BoardDisplayCell cell = TUI_BoardDisplayTable_Get(p1_board_state.display_table, row, column);
        if (value == BSHIP_WATER) 
        {
            cell.fg = BLACK;
            cell.bg = WHITE;
        }
        else
        {
            cell.fg = WHITE;
            cell.bg = LIGHT_RED;
        }
        if (i == 0)
        {
            cell.c = Char_From_TUI_ShipType(state->current_ship.direction == BSHIP_HORIZONTAL ?
                SHIP_TYPE_BOW_HORIZONTAL : SHIP_TYPE_BOW_VERTICAL);
        }
        else if (i == state->current_ship.length-1)
        {
            cell.c = Char_From_TUI_ShipType(state->current_ship.direction == BSHIP_HORIZONTAL ?
                SHIP_TYPE_STERN_HORIZONTAL : SHIP_TYPE_STERN_VERTICAL);
        }
        else 
        {
            cell.c = Char_From_TUI_ShipType(state->current_ship.direction == BSHIP_HORIZONTAL ?
                SHIP_TYPE_BODY_HORIZONTAL : SHIP_TYPE_BODY_VERTICAL);
        }
        TUI_BoardDisplayTable_Set(p1_board_state.display_table, cell, row, column);
    }

    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Place ships:"))));
    vector<TUI_TextGroup> board_groups = {};
    vector<TUI_TextGroup> ships_to_place = {};
    size_t board_display_width = 2 + p1_board_state.board.size; // NOTE(mattg): 2 for the number and column line
    TUI_Board_Add(board_groups, p1_board_state.display_table);
    size_t ship_lengths_width = TUI_HumanGame_ShipLengths_Add(ships_to_place, state->ship_lengths);

    assert(board_groups.size() >= ships_to_place.size());
    if (window->size.width > (board_display_width + 1 + ship_lengths_width))
    {
        // NOTE(mattg): left-right board game display
        size_t leftover_width = window->size.width - (board_display_width + ship_lengths_width) - 1;
        leftover_width = leftover_width < 5 ? leftover_width : 5;
        size_t ship_lengths_column_offset = board_display_width + leftover_width;
        for (size_t i = 0; i < board_groups.size(); i++)
        {
            TUI_Line line = TUI_Line_Default(board_groups.at(i));
            if (i < ships_to_place.size())
            {
                ships_to_place.at(i).column += ship_lengths_column_offset;
                TUI_Line_Add(&line, ships_to_place.at(i));
            }
            TUI_Window_Add(window, line);
        }
    }
    else
    {
        // NOTE(mattg): top-bottom board display
        for (size_t i = 0; i < board_groups.size(); i++)
        {
            TUI_Window_Add(window, TUI_Line_Default(board_groups.at(i)));
        }
        for (size_t i = 0; i < ships_to_place.size(); i++)
        {
            TUI_Window_Add(window, TUI_Line_Default(ships_to_place.at(i)));
        }
    }
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("↑ ↓ ← →    Move ship"))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Space      Flip direction"))));
    Color enter_color = state->current_ship_invalid ? GRAY : RESET;
    TUI_TextGroup enter_group = TUI_TextGroup_Default(TUI_Text_New("Enter      Select", {}, enter_color, RESET));
    if (state->current_ship_invalid)
    {
        TUI_TextGroup_Add(&enter_group, TUI_Text_New(" (disabled)", { BOLD }, GRAY, RESET));
    }
    TUI_Window_Add(window, TUI_Line_Default(enter_group));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Esc/q      Quit"))));
}

bool TUI_HumanGameState_ShipInput_Input(TUI_HumanGameState *state, TUI_Input *input, BShip_Board board,
    BShip_MatchData *match, BShip_Arena *arena)
{
    TUI_Input_ConvertTextTypes(input);
    switch (input->type)
    {
    case INPUT_ESC:
        return true;
        break;
    case INPUT_ENTER:
        if (!state->current_ship_invalid)
        {
            state->ships_placed.push_back(state->current_ship);
            if (state->ship_lengths.size() > 0)
            {
                state->current_ship.length = state->ship_lengths.at(0);
                state->ship_lengths.erase(state->ship_lengths.begin());
            }
            else
            {
                assert(state->ships_placed.size() == state->ship_count);
                state->type = HUMAN_SHOT_INPUT;
                state->current_shot = {
                    .row = 0,
                    .column = 0,
                    .value = BSHIP_WATER,
                };
                BShip_ShipArray array = {
                    .buffer = (BShip_Ship *)BShip_Arena_Push(arena, sizeof(BShip_Ship) * state->ship_count),
                    .length = 0,
                    .capacity = state->ship_count,
                };
                for (array.length = 0; array.length < state->ships_placed.size(); array.length++)
                {
                    array.buffer[array.length] = state->ships_placed.at(array.length);
                }
                if (!BShip_HumanGame_PlaceShips(match, array))
                {
                    state->type = HUMAN_GAME_OVER;
                }
                if (!BShip_HumanGame_ShouldContinue(match))
                {
                    state->type = HUMAN_GAME_OVER;
                }
            }
        }
        break;
    case INPUT_UP:
        if (state->current_ship.row > 0)
        {
            state->current_ship.row--;
        }
        break;
    case INPUT_DOWN:
        state->current_ship.row++;
        if (!TUI_ShipFitsOnBoard(state->current_ship, board.size))
        {
            state->current_ship.row--;
        }
        break;
    case INPUT_LEFT:
        if (state->current_ship.column > 0)
        {
            state->current_ship.column--;
        }
        break;
    case INPUT_RIGHT:
        state->current_ship.column++;
        if (!TUI_ShipFitsOnBoard(state->current_ship, board.size))
        {
            state->current_ship.column--;
        }
        break;
    case INPUT_SPACE:
        state->current_ship.direction = state->current_ship.direction == BSHIP_HORIZONTAL
            ? BSHIP_VERTICAL : BSHIP_HORIZONTAL;
        if (!TUI_ShipFitsOnBoard(state->current_ship, board.size))
        {
            switch (state->current_ship.direction)
            {
            case BSHIP_HORIZONTAL:
                state->current_ship.column = board.size - state->current_ship.length;
                break;
            case BSHIP_VERTICAL:
                state->current_ship.row = board.size - state->current_ship.length;
                break;
            }
            assert(TUI_ShipFitsOnBoard(state->current_ship, board.size));
        }
        break;
    default:
        break;
    }
    switch (input->type)
    {
    case INPUT_UP:
    case INPUT_DOWN:
    case INPUT_LEFT:
    case INPUT_RIGHT:
    case INPUT_SPACE:
        state->current_ship_invalid = TUI_ShipIsInvalid(state->current_ship, board);
        break;
    default:
        break;
    }
    return false;
}

void TUI_HumanGameState_ShotInput_Display(TUI_Window *window, TUI_HumanGameState *state,
    BShip_MatchData match, TUI_BoardState p1_board_state, TUI_BoardState ai2_board_state)
{
    TUI_BoardState_Reset(p1_board_state);
    TUI_BoardState_Reset(ai2_board_state);

    uint8_t remaining_ships = state->ship_count;
    for (size_t i = 0; i < match.events.length; i++)
    {
        BShip_Event event = match.events.buffer[i];
        if (event.type == BSHIP_EVENT_SHIP_PLACEMENT)
        {
            BShip_Ship p1_ship = BShip_Ship_From_CompactShip(event.value.compact.ai1_ship);
            BShip_Ship ai2_ship = BShip_Ship_From_CompactShip(event.value.compact.ai2_ship);
            // NOTE(mattg): We only want to store p1_ship, as player 1 knows where they put their ships,
            // but shouldn't know where ai2 put their ships. However, we should put all ships into the
            // ship type table, for use later.
            TUI_Store_Ship(p1_board_state.board, p1_ship, BSHIP_SHIP);
            TUI_ShipTypeTable_Store_Ship(p1_board_state.ship_table, p1_ship);
            TUI_ShipTypeTable_Store_Ship(ai2_board_state.ship_table, ai2_ship);
        }
        else if (event.type == BSHIP_EVENT_SHOT_RESULT)
        {
            BShip_Shot p1_shot = BShip_Shot_From_CompactShot(event.value.compact.ai1_shot);
            BShip_Shot ai2_shot = BShip_Shot_From_CompactShot(event.value.compact.ai2_shot);
            BShip_Board_Set(p1_board_state.board, ai2_shot.row, ai2_shot.column, ai2_shot.value);
            BShip_Board_Set(ai2_board_state.board, p1_shot.row, p1_shot.column, p1_shot.value);

            if (event.value.compact.ai1_ship > 0)
            {
                BShip_Ship p1_dead_ship = BShip_Ship_From_CompactShip(event.value.compact.ai1_ship);
                TUI_Store_Ship(p1_board_state.board, p1_dead_ship, BSHIP_KILL);
            }
            if (event.value.compact.ai2_ship > 0)
            {
                assert(remaining_ships > 0);
                remaining_ships--;
                assert(remaining_ships >= 0);
                BShip_Ship ai2_dead_ship = BShip_Ship_From_CompactShip(event.value.compact.ai2_ship);
                TUI_Store_Ship(ai2_board_state.board, ai2_dead_ship, BSHIP_KILL);
            }
        }
    }

    TUI_BoardDisplayTable_Set(p1_board_state);
    TUI_BoardDisplayTable_Set(ai2_board_state);

    // NOTE(mattg): add cursor to ai2's board for shooting
    assert(state->current_shot.row < ai2_board_state.board.size);
    assert(state->current_shot.column < ai2_board_state.board.size);
    TUI_BoardDisplayCell cell = TUI_BoardDisplayTable_Get(ai2_board_state.display_table,
        state->current_shot.row, state->current_shot.column);
    Color fg = cell.fg;
    // NOTE(mattg): invert colors, add crosshair
    cell.fg = cell.bg;
    cell.bg = fg;
    cell.c = '+';
    TUI_BoardDisplayTable_Set(ai2_board_state.display_table, cell,
        state->current_shot.row, state->current_shot.column);

    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    vector<TUI_TextGroup> p1_group = {};
    vector<TUI_TextGroup> ai2_group = {};

    string p1_name = match.ai1.name, ai2_name = match.ai2.name;
    // NOTE(mattg): 2 for the number and column line
    size_t board_display_width = 2 + match.board_size;
    size_t name_display_width = p1_name.size() > ai2_name.size() ? p1_name.size() : ai2_name.size();
    size_t total_display_width = board_display_width > name_display_width ? board_display_width : name_display_width;

    p1_group.push_back(TUI_TextGroup_Default(
        TUI_Text_New(p1_name, { BOLD }, TUI_Player_Color_Get(BSHIP_PLAYER_1), RESET)
    ));
    ai2_group.push_back(TUI_TextGroup_Default(
        TUI_Text_New(ai2_name, { BOLD }, TUI_Player_Color_Get(BSHIP_PLAYER_2), RESET)
    ));
    
    TUI_Board_Add(p1_group, p1_board_state.display_table);
    TUI_Board_Add(ai2_group, ai2_board_state.display_table);

    BShip_Event event = match.events.buffer[match.events.length-1];
    if (event.type == BSHIP_EVENT_SHOT_RESULT)
    {
        TUI_TextGroups_Add_EventDescriptions(p1_group, ai2_group, match, event);
    }

    assert(p1_group.size() == ai2_group.size());
    TUI_TextGroup remaining_group = TUI_TextGroup_Default(
        TUI_Text_New(to_string(remaining_ships), { BOLD }, RESET, RESET)
    );
    TUI_TextGroup_Add(&remaining_group, TUI_Text_Default(remaining_ships == 1 ? " ship left" : " ships left"));

    if (window->size.width > (total_display_width * 2) + TUI_TextGroup_Size(remaining_group) + 1)
    {
        // NOTE(mattg): left-right board game display
        size_t leftover_width = window->size.width - (board_display_width * 2) - 1;
        leftover_width = leftover_width < 15 ? leftover_width : 15;
        size_t board2_column_offset = board_display_width + leftover_width;
        for (size_t i = 0; i < p1_group.size(); i++)
        {
            TUI_Line line = TUI_Line_Default(p1_group.at(i));
            ai2_group.at(i).column += board2_column_offset;
            TUI_Line_Add(&line, ai2_group.at(i));
            TUI_Window_Add(window, line);
        }
        remaining_group.column += board2_column_offset;
        TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
        TUI_Window_Add(window, TUI_Line_Default(remaining_group));
    }
    else
    {
        // NOTE(mattg): top-bottom board display
        for (size_t i = 0; i < p1_group.size(); i++)
        {
            TUI_Window_Add(window, TUI_Line_Default(p1_group.at(i)));
        }
        for (size_t i = 0; i < ai2_group.size(); i++)
        {
            TUI_Window_Add(window, TUI_Line_Default(ai2_group.at(i)));
        }
        TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
        TUI_Window_Add(window, TUI_Line_Default(remaining_group));
    }
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("↑ ↓ ← →    Move shot"))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Enter      Select"))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Esc/q      Quit"))));
}

bool TUI_HumanGameState_ShotInput_Input(TUI_HumanGameState *state, TUI_Input *input, BShip_MatchData *match)
{
    TUI_Input_ConvertTextTypes(input);
    switch (input->type)
    {
    case INPUT_ESC:
        return true;
        break;
    case INPUT_ENTER:
        if (!BShip_HumanGame_PlaceShot(match, state->current_shot))
        {
            state->type = HUMAN_GAME_OVER;
        }
        if (!BShip_HumanGame_ShouldContinue(match))
        {
            state->type = HUMAN_GAME_OVER;
        }
        break;
    case INPUT_UP:
        if (state->current_shot.row > 0)
        {
            state->current_shot.row--;
        }
        break;
    case INPUT_DOWN:
        if (state->current_shot.row < match->board_size-1)
        {
            state->current_shot.row++;
        }
        break;
    case INPUT_LEFT:
        if (state->current_shot.column > 0)
        {
            state->current_shot.column--;
        }
        break;
    case INPUT_RIGHT:
        if (state->current_shot.column < match->board_size-1)
        {
            state->current_shot.column++;
        }
        break;
    default:
        break;
    }
    return false;
}

void TUI_HumanGameState_GameOver_Display(TUI_Window *window, TUI_HumanGameState *state, BShip_MatchData *match,
    BShip_Arena *arena, char *log_path, TUI_BoardState p1_board_state, TUI_BoardState ai2_board_state)
{
    if (!state->has_ended_game)
    {
        state->has_ended_game = true;
        BShip_HumanGame_Stop(match, arena);
        BShip_Match_Log_Store(*match, log_path);
        {
            BShip_ArenaMark mark = BShip_ArenaMark_Get(arena);
            BShip_DerivedMatchStats derived_stats = BShip_DerivedMatchStats_Get(arena, *match, false);
            state->stats = TUI_MatchStats_From_BShip_DerivedMatchStats(derived_stats);
            BShip_Arena_Rollback(arena, mark);
        }
        for (size_t i = 0; i < match->events.length; i++)
        {
            BShip_Event event = match->events.buffer[i];
            if (event.type == BSHIP_EVENT_SHIP_PLACEMENT)
            {
                BShip_Ship p1_ship = BShip_Ship_From_CompactShip(event.value.compact.ai1_ship);
                BShip_Ship ai2_ship = BShip_Ship_From_CompactShip(event.value.compact.ai2_ship);
                TUI_Store_Ship(p1_board_state.board, p1_ship, BSHIP_SHIP);
                TUI_Store_Ship(ai2_board_state.board, ai2_ship, BSHIP_SHIP);
                TUI_ShipTypeTable_Store_Ship(p1_board_state.ship_table, p1_ship);
                TUI_ShipTypeTable_Store_Ship(ai2_board_state.ship_table, ai2_ship);
            }
            else if (event.type == BSHIP_EVENT_SHOT_RESULT)
            {
                BShip_Shot p1_shot = BShip_Shot_From_CompactShot(event.value.compact.ai1_shot);
                BShip_Shot ai2_shot = BShip_Shot_From_CompactShot(event.value.compact.ai2_shot);
                BShip_Board_Set(p1_board_state.board, ai2_shot.row, ai2_shot.column, ai2_shot.value);
                BShip_Board_Set(ai2_board_state.board, p1_shot.row, p1_shot.column, p1_shot.value);

                if (event.value.compact.ai1_ship > 0)
                {
                    BShip_Ship p1_dead_ship = BShip_Ship_From_CompactShip(event.value.compact.ai1_ship);
                    TUI_Store_Ship(p1_board_state.board, p1_dead_ship, BSHIP_KILL);
                }
                if (event.value.compact.ai2_ship > 0)
                {
                    BShip_Ship ai2_dead_ship = BShip_Ship_From_CompactShip(event.value.compact.ai2_ship);
                    TUI_Store_Ship(ai2_board_state.board, ai2_dead_ship, BSHIP_KILL);
                }
            }
        }
        TUI_BoardDisplayTable_Set(p1_board_state);
        TUI_BoardDisplayTable_Set(ai2_board_state);
    }
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    vector<TUI_TextGroup> p1_group = {};
    vector<TUI_TextGroup> ai2_group = {};

    string p1_name = match->ai1.name, ai2_name = match->ai2.name;
    // NOTE(mattg): 2 for the number and column line
    size_t board_display_width = 2 + match->board_size;
    size_t name_display_width = p1_name.size() > ai2_name.size() ? p1_name.size() : ai2_name.size();
    size_t total_display_width = board_display_width > name_display_width ? board_display_width : name_display_width;

    p1_group.push_back(TUI_TextGroup_Default(
        TUI_Text_New(p1_name, { BOLD }, TUI_Player_Color_Get(BSHIP_PLAYER_1), RESET)
    ));
    ai2_group.push_back(TUI_TextGroup_Default(
        TUI_Text_New(ai2_name, { BOLD }, TUI_Player_Color_Get(BSHIP_PLAYER_2), RESET)
    ));
    
    TUI_Board_Add(p1_group, p1_board_state.display_table);
    TUI_Board_Add(ai2_group, ai2_board_state.display_table);

    BShip_Event event = match->events.buffer[match->events.length-1];
    if (event.type == BSHIP_EVENT_GAME_RESULT)
    {
        TUI_TextGroups_Add_EventDescriptions(p1_group, ai2_group, *match, event);
    }

    assert(p1_group.size() == ai2_group.size());

    if (window->size.width > (total_display_width * 2) + 1)
    {
        // NOTE(mattg): left-right board game display
        size_t leftover_width = window->size.width - (board_display_width * 2) - 1;
        leftover_width = leftover_width < 15 ? leftover_width : 15;
        size_t board2_column_offset = board_display_width + leftover_width;
        for (size_t i = 0; i < p1_group.size(); i++)
        {
            TUI_Line line = TUI_Line_Default(p1_group.at(i));
            ai2_group.at(i).column += board2_column_offset;
            TUI_Line_Add(&line, ai2_group.at(i));
            TUI_Window_Add(window, line);
        }
    }
    else
    {
        // NOTE(mattg): top-bottom board display
        for (size_t i = 0; i < p1_group.size(); i++)
        {
            TUI_Window_Add(window, TUI_Line_Default(p1_group.at(i)));
        }
        for (size_t i = 0; i < ai2_group.size(); i++)
        {
            TUI_Window_Add(window, TUI_Line_Default(ai2_group.at(i)));
        }
    }
    if (state->stats.games.size() > 0)
    {
        TUI_GameStats_Display(window, p1_name, ai2_name, state->stats.games.at(0));
    }
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Esc/q      Quit"))));
}

bool TUI_HumanGameState_GameOver_Input(TUI_Input *input)
{
    TUI_Input_ConvertTextTypes(input);
    return input->type == INPUT_ESC;
}

void TUI_HumanGame_Play(char *socket_path, char *log_path, BShip_AIFileData ai2_file_data, uint8_t board_size,
    bool debug)
{
    TUI_Window window = {};
    TUI_WindowSize_Get(&window.size);

    BShip_Arena arena = {};
    BShip_Arena_Initialize(&arena, BShip_Match_CalculateMemorySize(board_size, 1));
    BShip_Connection *conn = BShip_Connection_Create(&arena, socket_path);

    TUI_BoardState p1_board_state = TUI_BoardState_Allocate(&arena, board_size);
    TUI_BoardState ai2_board_state = TUI_BoardState_Allocate(&arena, board_size);

    BShip_MatchData match = BShip_HumanGame_Initialize(&arena, ai2_file_data, board_size, debug);

    TUI_Input input = {};
    TUI_HumanGameState state = {
        .name = "",
        .ship_count = 0,
        .ship_lengths = {},
        .ships_placed = {},
        .current_ship = {},
        .current_ship_invalid = false,
        .current_shot = {},
        .has_ended_game = false,
        .stats = {},
        .type = HUMAN_NAME_INPUT,
    };
    const bool wait_for_input = true;

    if (!TUI_Window_Enter(&window, wait_for_input))
    {
        TUI_Window_Exit(&window);
        BShip_Arena_Destroy(&arena);
        return;
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

        if (state.type != HUMAN_NAME_INPUT)
        {
            TUI_TextGroup p1_vs_group = TUI_Player_VS_Get(match.ai1, BSHIP_PLAYER_1);
            TUI_TextGroup ai2_vs_group = TUI_Player_VS_Get(match.ai2, BSHIP_PLAYER_2);

            TUI_VS_Display(&window, p1_vs_group, ai2_vs_group);
        }

        switch (state.type)
        {
        case HUMAN_NAME_INPUT:
            TUI_HumanGameState_NameInput_Display(&window, &state);
            break;
        case HUMAN_SHIP_INPUT:
            TUI_HumanGameState_ShipInput_Display(&window, &state, p1_board_state);
            break;
        case HUMAN_SHOT_INPUT:
            TUI_HumanGameState_ShotInput_Display(&window, &state, match, p1_board_state, ai2_board_state);
            break;
        case HUMAN_GAME_OVER:
            TUI_HumanGameState_GameOver_Display(&window, &state, &match, &arena, log_path,
                p1_board_state, ai2_board_state);
            break;
        }

        TUI_Window_Print(&window);

        input = TUI_Input_Get(wait_for_input, -1);
        TUI_Input_ScrollState_Get(&window, input);

        bool should_exit = false;
        switch (state.type)
        {
        case HUMAN_NAME_INPUT:
            should_exit = TUI_HumanGameState_NameInput_Input(&state, &input, &match, conn, socket_path);
            break;
        case HUMAN_SHIP_INPUT:
            should_exit = TUI_HumanGameState_ShipInput_Input(&state, &input, p1_board_state.board,
                &match, &arena);
            break;
        case HUMAN_SHOT_INPUT:
            should_exit = TUI_HumanGameState_ShotInput_Input(&state, &input, &match);
            break;
        case HUMAN_GAME_OVER:
            should_exit = TUI_HumanGameState_GameOver_Input(&input);
            break;
        }
        if (should_exit)
        {
            break;
        }
    }
    if (!state.has_ended_game)
    {
        BShip_HumanGame_Stop(&match, &arena);
        BShip_Match_Log_Store(match, log_path);
    }

    TUI_Window_Exit(&window);
    BShip_Arena_Destroy(&arena);
}
