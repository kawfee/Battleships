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
    uint32_t ship_index;
    uint32_t shot_index;
} TUI_GameStepState;

void TUI_GameStepState_Display(TUI_Window *window, TUI_GameStepState *state,
    BShip_MatchData match, BShip_Board ai1_board, BShip_Board ai2_board)
{
    string ai1_name = match.ai1.name, ai2_name = match.ai2.name;
    size_t board_display_width = match.board_size + 12;
    size_t name_display_width = ai1_name.size() + 12;
    size_t board2_column_offset = board_display_width > name_display_width
        ? board_display_width : name_display_width;

    uint32_t game_num = match.games_per_match;
    string game_num_str = "Game #" + to_string(game_num);
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(game_num_str))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));

    // we do it this way so that we can put the board wherever we need to depending on the screen size
    vector<TUI_TextGroup> ai1_group = {};
    vector<TUI_TextGroup> ai2_group = {};

    TUI_TextGroup_Add_Board(ai1_group, match.ai1.name, ai1_board, BSHIP_PLAYER_1);
    TUI_TextGroup_Add_Board(ai2_group, match.ai2.name, ai2_board, BSHIP_PLAYER_2);

    assert(ai1_group.size() == ai2_group.size());

    for (size_t i = 0; i < ai1_group.size(); i++)
    {
        TUI_Line line = TUI_Line_Default(ai1_group.at(i));
        ai2_group.at(i).column += board2_column_offset;
        TUI_Line_Add(&line, ai2_group.at(i));
        TUI_Window_Add(window, line);
    }
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

void TUI_Match_Display(BShip_MatchData match, bool debug)
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

    bool wait_for_input = false;
    if (!TUI_Window_Enter(&window, wait_for_input))
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

        TUI_GameStepState_Display(&window, &state, match, ai1_board, ai2_board);

        TUI_Window_Print(&window);
        sleep(1);
    }

on_exit:
    TUI_Window_Exit(&window);
    BShip_Arena_Destroy(&arena);
    return;
}
