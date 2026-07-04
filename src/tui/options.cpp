/**
 * @file options.cpp
 * @author Matthew Getgen
 * @brief Battleships tui input options.
 * @date 2026-06-18
 */

#include <optional>

#include "tui.cpp"
#include "../../lib/battleshipslib.h"

using namespace std;
using namespace conio;

typedef enum {
    RUNTIME_NONE,
    RUNTIME_MATCH,
    RUNTIME_CONTEST,
    RUNTIME_REPLAY_MATCH,
    RUNTIME_REPLAY_CONTEST,
} TUI_RuntimeType;

typedef struct {
    TUI_RuntimeType runtime;
    BShip_AIFileData ai1;
    BShip_AIFileData ai2;
    uint32_t games_per_match;
    uint8_t board_size;
} TUI_Options;

typedef struct {
    optional<TUI_RuntimeType> runtime;
    optional<uint8_t> board_size;
    optional<uint32_t> games_per_match;
    optional<BShip_AIFileData> ai1;
    optional<BShip_AIFileData> ai2;
    size_t runtime_selection;
    size_t board_size_selection;
    size_t games_per_match_selection;
    size_t games_per_match_min;
    size_t games_per_match_max;
    string games_per_match_string;
    bool games_per_match_invalid;
    size_t ai1_selection;
    size_t ai2_selection;
    size_t ai_window_top;
} TUI_OptionsState;

const TUI_RuntimeType RuntimeTypeTable[] = {
    RUNTIME_MATCH,
    RUNTIME_CONTEST,
    RUNTIME_REPLAY_MATCH,
    RUNTIME_REPLAY_CONTEST,
};
const char* RuntimeStringTable[] = {
    "Test AI",
    "Run Contest",
    "Replay Test",
    "Replay Contest",
};

void TUI_RuntimeType_Display(TUI_Window *window, TUI_OptionsState *state)
{
    if (state->runtime_selection >= TUI_ARRAY_LENGTH(RuntimeStringTable))
    {
        state->runtime_selection = TUI_ARRAY_LENGTH(RuntimeStringTable)-1;
    }

    TUI_Text prompt = TUI_Text_Default("Runtime: ");
    TUI_TextGroup prompt_group = TUI_TextGroup_Default(prompt);
    if (state->runtime.has_value() && state->runtime.value() != RUNTIME_NONE)
    {
        TUI_Text value = {
            .text = RuntimeStringTable[state->runtime_selection],
            .style = {
                .styles = { BOLD },
                .fg = RESET,
                .bg = RESET,
            },
        };
        TUI_TextGroup_Add(&prompt_group, value);
        TUI_Window_Add(window, TUI_Line_Default(prompt_group));
        return;
    }
    TUI_Window_Add(window, TUI_Line_Default(prompt_group));

    for (size_t i = 0; i < TUI_ARRAY_LENGTH(RuntimeStringTable); i++)
    {
        string runtime_string = state->runtime_selection == i ? " > " : "    ";
        runtime_string += RuntimeStringTable[i];
        TUI_Text runtime = TUI_Text_Default(runtime_string);
        TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(runtime)));
    }
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("↑ ↓ j/k  Move"))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Enter    Select"))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Esc/q    Quit"))));
}

bool TUI_RuntimeType_Input(TUI_OptionsState *state, TUI_Options *options, TUI_Input input)
{
    switch (input.type)
    {
    case INPUT_UP:
        if (state->runtime_selection > 0) state->runtime_selection--;
        break;
    case INPUT_DOWN:
        if (state->runtime_selection < TUI_ARRAY_LENGTH(RuntimeTypeTable)-1) state->runtime_selection++;
        break;
    case INPUT_ENTER:
        state->runtime = RuntimeTypeTable[state->runtime_selection];
        options->runtime = state->runtime.value();
        break;
    case INPUT_ESC:
        return true;
    default:
        break;
    }
    return false;
}

void TUI_BoardSize_Display(TUI_Window *window, TUI_OptionsState *state)
{
    if (state->board_size_selection < BSHIP_BOARD_SIZE_MIN)
    {
        state->board_size_selection = BSHIP_BOARD_SIZE_MIN;
    }
    else if (state->board_size_selection > BSHIP_BOARD_SIZE_MAX)
    {
        state->board_size_selection = BSHIP_BOARD_SIZE_MAX;
    }
    
    TUI_Text prompt = TUI_Text_Default("Board Size: ");
    TUI_TextGroup group = TUI_TextGroup_Default(prompt);
    if (state->board_size.has_value())
    {
        string bs = to_string(state->board_size.value());
        if (bs.size() == 1) bs = " " + bs;
        TUI_Text value = {
            .text = bs,
            .style = {
                .styles = { BOLD, },
                .fg = RESET,
                .bg = RESET,
            },
        };
        TUI_TextGroup_Add(&group, value);
        TUI_Window_Add(window, TUI_Line_Default(group));
        return;
    }

    string prefix = state->board_size_selection == BSHIP_BOARD_SIZE_MIN ? "  " : "- ";
    string postfix = state->board_size_selection == BSHIP_BOARD_SIZE_MAX ? "" : " +";
    string bs = to_string(state->board_size_selection);
    if (bs.size() == 1) bs = " " + bs;
    TUI_Text text = TUI_Text_Default(prefix + bs + postfix);
    TUI_TextGroup_Add(&group, text);
    TUI_Window_Add(window, TUI_Line_Default(group));

    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("← → h/l  Change"))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Enter    Select"))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Esc/q    Quit"))));
}

bool TUI_BoardSize_Input(TUI_OptionsState *state, TUI_Options *options, TUI_Input input)
{
    switch (input.type)
    {
    case INPUT_LEFT:
        if (state->board_size_selection > BSHIP_BOARD_SIZE_MIN) state->board_size_selection--;
        break;
    case INPUT_RIGHT:
        if (state->board_size_selection < BSHIP_BOARD_SIZE_MAX) state->board_size_selection++;
        break;
    case INPUT_ENTER:
        state->board_size = state->board_size_selection;
        options->board_size = state->board_size_selection;
        state->games_per_match_selection = BShip_GamesPerMatchDefault_From_BoardSize(state->board_size_selection);
        state->games_per_match_min = BShip_GamesPerMatchMin_From_BoardSize(state->board_size_selection);
        state->games_per_match_max = BShip_GamesPerMatchMax_From_BoardSize(state->board_size_selection);
        state->games_per_match_string = to_string(state->games_per_match_selection);
        break;
    case INPUT_ESC:
        return true;
    default:
        break;
    }
    return false;
}

void TUI_GamesPerMatch_Display(TUI_Window *window, TUI_OptionsState *state)
{
    if (state->games_per_match_selection < state->games_per_match_min)
    {
        state->games_per_match_selection = state->games_per_match_min;
    }
    else if (state->games_per_match_selection > state->games_per_match_max)
    {
        state->games_per_match_selection = state->games_per_match_max;
    }
    TUI_Text prompt = TUI_Text_Default("Games: ");
    TUI_TextGroup group = TUI_TextGroup_Default(prompt);
    if (state->games_per_match.has_value())
    {
        string gpm = to_string(state->games_per_match.value());
        TUI_Text value = {
            .text = gpm,
            .style = {
                .styles = { BOLD },
                .fg = RESET,
                .bg = RESET,
            },
        };
        TUI_TextGroup_Add(&group, value);
        TUI_Window_Add(window, TUI_Line_Default(group));
        return;
    }
    TUI_Text value = TUI_Text_Default(state->games_per_match_string);
    TUI_TextGroup_Add(&group, value);
    TUI_Text cursor = {
        .text = " ",
        .style = {
            .styles = { NEGATIVE_IMAGE, },
            .fg = RESET,
            .bg = RESET,
        },
    };
    TUI_TextGroup_Add(&group, cursor);
    TUI_Window_Add(window, TUI_Line_Default(group));

    bool invalid_number = false;
    bool invalid_range = false;
    try
    {
        size_t pos = 0;
        size_t selection = stoi(state->games_per_match_string, &pos);

        if (pos != state->games_per_match_string.size())
        {
            invalid_number = true;
        }
        if (selection < state->games_per_match_min || selection > state->games_per_match_max)
        {
            invalid_range = true;
        }
        if (!invalid_number && !invalid_range)
        {
            state->games_per_match_selection = selection;
        }
    }
    catch (const std::invalid_argument&)
    {
        invalid_number = true;
    }
    catch (const std::out_of_range&)
    {
        invalid_range = true;
    }
    state->games_per_match_invalid = (invalid_number || invalid_range);

    if (invalid_number)
    {
        TUI_Text error = {
            .text = "Invalid Number!",
            .style = {
                .styles = {},
                .fg = RED,
                .bg = RESET,
            },
        };
        TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(error)));
    }
    else if (invalid_range)
    {
        string range = "Out of Range: ";
        range += to_string(state->games_per_match_min) + "-" + to_string(state->games_per_match_max);
        TUI_Text error = {
            .text = range,
            .style = {
                .styles = {},
                .fg = RED,
                .bg = RESET,
            },
        };
        TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(error)));
    }

    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("0-9      Edit"))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Bksp     Delete"))));
    TUI_TextGroup enter_group = TUI_TextGroup_Default(TUI_Text_Default("Enter    Select"));
    if (state->games_per_match_invalid)
    {
        TUI_Text disabled = {
            .text = " (disabled)",
            .style = {
                .styles = { BOLD },
                .fg = RESET,
                .bg = RESET,
            },
        };
        TUI_TextGroup_Add(&enter_group, disabled);
    }
    TUI_Window_Add(window, TUI_Line_Default(enter_group));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Esc/q    Quit"))));
}

bool TUI_GamesPerMatch_Input(TUI_OptionsState *state, TUI_Options *options, TUI_Input input)
{
    switch (input.type)
    {
    case INPUT_NUM:
        if (state->games_per_match_string.size() < 10) state->games_per_match_string.push_back(input.value);
        break;
    case INPUT_BACKSPACE:
        if (state->games_per_match_string.size() > 0) state->games_per_match_string.pop_back();
        break;
    case INPUT_ENTER:
        if (!state->games_per_match_invalid)
        {
            state->games_per_match = state->games_per_match_selection;
            options->games_per_match = state->games_per_match_selection;

            state->ai1_selection = 0;
            state->ai2_selection = 0;
            state->ai_window_top = 0;
        }
        break;
    case INPUT_ESC:
        return true;
    default:
        break;
    }
    return false;
}

void TUI_MatchPlayer_Display(TUI_Window *window, TUI_OptionsState *state,
    const vector<BShip_AIFileData> &ais, BShip_PlayerNum player)
{
    if (player == BSHIP_PLAYER_1 && state->ai1_selection >= ais.size())
    {
        state->ai1_selection = ais.size() - 1;
    }
    else if (player == BSHIP_PLAYER_2 && state->ai2_selection >= ais.size())
    {
        state->ai2_selection = ais.size() - 1;
    }
    
    TUI_TextGroup prompt_group = TUI_TextGroup_Default(
        TUI_Text_Default((player == BSHIP_PLAYER_1 ? "Player 1: " : "Player 2: "))
    );
    if ((player == BSHIP_PLAYER_1 && state->ai1.has_value()) || (player == BSHIP_PLAYER_2 && state->ai2.has_value()))
    {
        BShip_AIFileData ai_selection = player == BSHIP_PLAYER_1 ? state->ai1.value() : state->ai2.value();
        TUI_Text value = {
            .text = ai_selection.file_name,
            .style = {
                .styles = { BOLD },
                .fg = RESET,
                .bg = RESET,
            },
        };
        TUI_TextGroup_Add(&prompt_group, value);
        TUI_Window_Add(window, TUI_Line_Default(prompt_group));
        return;
    }
    TUI_Window_Add(window, TUI_Line_Default(prompt_group));

    const size_t LEGEND_LINE_COUNT = 4; // 3 for legend, 1 for spacer.
    const size_t MORE_LINE_COUNT = 2;
    const size_t AI_SELECTION_LINE_COUNT_MIN = 3;
    const size_t LINE_COUNT_MIN = LEGEND_LINE_COUNT + MORE_LINE_COUNT + AI_SELECTION_LINE_COUNT_MIN;
    size_t height = window->size.height;
    size_t lines = window->lines.size();
    size_t space = (height <= lines ||  (height-lines) < LINE_COUNT_MIN ) ? LINE_COUNT_MIN : height - lines;
    assert(space >= LINE_COUNT_MIN);
    size_t space_without_more = space - LEGEND_LINE_COUNT;
    assert(space_without_more < space);
    size_t display_space = ais.size() > space_without_more ? space_without_more - MORE_LINE_COUNT : space_without_more;
    assert(display_space <= space_without_more);

    size_t ai_selection = player == BSHIP_PLAYER_1 ? state->ai1_selection : state->ai2_selection;
    size_t ai_window_bottom = state->ai_window_top + display_space - 1;
    if (display_space < ais.size())
    {
        if (ai_selection == 0)
        {
            state->ai_window_top = 0;
        }
        else if (ai_selection == ais.size() - 1)
        {
            state->ai_window_top = ais.size() - display_space;
        }
        else if (ai_selection == state->ai_window_top)
        {
            state->ai_window_top = ai_selection - 1;
        }
        else if (ai_selection == ai_window_bottom)
        {
            state->ai_window_top += 1;
        }
        else
        {
            assert(ai_selection >= state->ai_window_top);
            assert(ai_selection < state->ai_window_top + display_space);
        }
    }
    else 
    {
        state->ai_window_top = 0;
    }

    if (display_space < ais.size())
    {
        string more_above = state->ai_window_top == 0 ? "" : "↑ " + to_string(state->ai_window_top) + " more";
        TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(more_above))));
    }

    for (size_t i = 0; i < ais.size(); i++)
    {
        if (i < state->ai_window_top) continue;
        else if (i > (state->ai_window_top + display_space-1)) break;

        string ai = i == ai_selection ? " > " : "    ";
        ai += ais.at(i).file_name;
        TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(ai))));
    }

    if (display_space < ais.size())
    {
        string more_below = ai_window_bottom == ais.size()-1 ? "" : "↓ " + to_string((ais.size()-1) - ai_window_bottom) + " more";
        TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(more_below))));
    }

    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("↑ ↓ j/k  Move"))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Enter    Select"))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Esc/q    Quit"))));
}

bool TUI_MatchPlayer_Input(TUI_OptionsState *state, TUI_Options *options, TUI_Input input,
    const vector<BShip_AIFileData> &ais, BShip_PlayerNum player)
{
    size_t *ai_selection_ref = player == BSHIP_PLAYER_1 ? &state->ai1_selection : &state->ai2_selection;
    switch (input.type)
    {
    case INPUT_UP:
        if (*ai_selection_ref > 0) (*ai_selection_ref)--;
        break;
    case INPUT_DOWN:
        if (*ai_selection_ref < ais.size()-1) (*ai_selection_ref)++;
        break;
    case INPUT_ENTER:
        if (player == BSHIP_PLAYER_1)
        {
            state->ai1 = ais.at(state->ai1_selection);
            options->ai1 = ais.at(state->ai1_selection);
        }
        else
        {
            state->ai2 = ais.at(state->ai2_selection);
            options->ai2 = ais.at(state->ai2_selection);
        }
        state->ai_window_top = 0;
        break;
    case INPUT_ESC:
        return true;
    default:
        break;
    }
    return false;
}

bool TUI_Options_Display(TUI_Window *window, TUI_OptionsState *state, const vector<BShip_AIFileData> &ais)
{
    TUI_RuntimeType_Display(window, state);
    if (!state->runtime.has_value()) return false;

    TUI_RuntimeType runtime = state->runtime.has_value() ? state->runtime.value() : RUNTIME_NONE;
    if (runtime == RUNTIME_MATCH || runtime == RUNTIME_CONTEST)
    {
        TUI_BoardSize_Display(window, state);
        if (!state->board_size.has_value()) return false;

        TUI_GamesPerMatch_Display(window, state);
        if (!state->games_per_match.has_value()) return false;
    }

    if (runtime == RUNTIME_MATCH)
    {
        TUI_MatchPlayer_Display(window, state, ais, BSHIP_PLAYER_1);
        if (!state->ai1.has_value()) return false;

        TUI_MatchPlayer_Display(window, state, ais, BSHIP_PLAYER_2);
        if (!state->ai2.has_value()) return false;
    }
    return true;
}

bool TUI_Options_Input(TUI_OptionsState *state, TUI_Options *options, const vector<BShip_AIFileData> &ais)
{
    TUI_Input input = TUI_Input_Get();

    if (!state->runtime.has_value())
    {
        return TUI_RuntimeType_Input(state, options, input);
    }
    TUI_RuntimeType runtime = state->runtime.has_value() ? state->runtime.value() : RUNTIME_NONE;
    if (runtime == RUNTIME_MATCH || runtime == RUNTIME_CONTEST)
    {
        if (!state->board_size.has_value())
        {
            return TUI_BoardSize_Input(state, options, input);
        }
        
        if (!state->games_per_match.has_value())
        {
            return TUI_GamesPerMatch_Input(state, options, input);
        }
    }

    if (runtime == RUNTIME_MATCH)
    {
        if (!state->ai1.has_value())
        {
            return TUI_MatchPlayer_Input(state, options, input, ais, BSHIP_PLAYER_1);
        }

        if (!state->ai2.has_value())
        {
            return TUI_MatchPlayer_Input(state, options, input, ais, BSHIP_PLAYER_2);
        }
    }
    return false;
}

bool TUI_Options_Get(TUI_Options *options, const vector<BShip_AIFileData> &ais, bool debug)
{
    bool should_exit = false;

    vector<string> ascii_banner = {
        "  ____        _   _   _           _",
        " | __ )  __ _| |_| |_| | ___  ___| |__  _ _ __  ___",
        " |  _ \\ / _` | __| __| |/ _ \\/ __| '_ \\| | '_ \\/ __|",
        " | |_) | (_| | |_| |_| |  __/\\__ \\ | | | | |_) \\__ \\",
        " |____/ \\__,_|\\__|\\__|_|\\___||___/_| |_|_| .__/|___/",
        "                                         |_|",
        "",
    };
    vector<string> text_banner = {
        "                B A T T L E S H I P S",
        "----------------------------------------------------",
    };
    uint32_t ascii_banner_height_min = (ascii_banner.size() + text_banner.size()) * 5;

    TUI_Window window = {};
    TUI_WindowSize_Get(&window.size);

    TUI_OptionsState state = {
        .runtime = {},
        .board_size = {},
        .games_per_match = {},
        .ai1 = {},
        .ai2 = {},
        // defaults
        .runtime_selection = 0,
        .board_size_selection = 10,
        .games_per_match_selection = 0,
        .games_per_match_min = 0,
        .games_per_match_max = 0,
        .games_per_match_string = "",
        .games_per_match_invalid = false,
        .ai1_selection = 0,
        .ai2_selection = 0,
        .ai_window_top = 0,
    };

    if (!TUI_Window_Enter(&window))
    {
        should_exit = true;
        goto on_exit;
    }

    while (!TUI_Should_Close() && !should_exit)
    {
        if (TUI_Should_Resize())
        {
            TUI_WindowSize_Get(&window.size);
        }
        TUI_Window_Reset(&window);
        if (debug)
        {
            string width_height = to_string(window.size.width) + "x" + to_string(window.size.height);
            TUI_Line debug_line = TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(width_height)));
            TUI_Window_Add(&window, debug_line);
        }
        
        if (window.size.height >= ascii_banner_height_min)
        {
            for (size_t i = 0; i < ascii_banner.size(); i++)
            {
                TUI_Text text = TUI_Text_Default(ascii_banner.at(i));
                TUI_TextGroup group = TUI_TextGroup_Default(text);
                TUI_Line line = TUI_Line_Default(group);
                TUI_Window_Add(&window, line);
            }
        }
        for (size_t i = 0; i < text_banner.size(); i++)
        {
            TUI_Text text = TUI_Text_Default(text_banner.at(i));
            TUI_TextGroup group = TUI_TextGroup_Default(text);
            TUI_Line line = TUI_Line_Default(group);
            TUI_Window_Add(&window, line);
        }

        // NOTE(mattg): TUI_Options_Display returns true if all necessary state has been input.
        if (TUI_Options_Display(&window, &state, ais))
        {
            break;
        }

        TUI_Window_Print(&window);

        should_exit = TUI_Options_Input(&state, options, ais);
    }
on_exit:
    TUI_Window_Exit(&window);
    if (TUI_Should_Close())
    {
        should_exit = true;
    }
    return should_exit;
}

