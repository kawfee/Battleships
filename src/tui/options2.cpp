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
            .style = BOLD,
            .fg = RESET,
            .bg = RESET,
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

bool TUI_RuntimeType_Input(TUI_OptionsState *state)
{
    TUI_Input input = TUI_Input_Get(TUI_INPUT_60FPS, false);
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
            .style = BOLD,
            .fg = RESET,
            .bg = RESET,
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

bool TUI_BoardSize_Input(TUI_OptionsState *state)
{
    TUI_Input input = TUI_Input_Get(TUI_INPUT_60FPS, false);
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
            .style = BOLD,
            .fg = RESET,
            .bg = RESET,
        };
        TUI_TextGroup_Add(&group, value);
        TUI_Window_Add(window, TUI_Line_Default(group));
        return;
    }
    TUI_Text value = TUI_Text_Default(state->games_per_match_string);
    TUI_TextGroup_Add(&group, value);
    TUI_Text cursor = {
        .text = " ",
        .style = NEGATIVE_IMAGE,
        .fg = RESET,
        .bg = RESET,
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
            .style = NORMAL_INTENSITY,
            .fg = RED,
            .bg = RESET,
        };
        TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(error)));
    }
    else if (invalid_range)
    {
        string range = "Out of Range: ";
        range += to_string(state->games_per_match_min) + "-" + to_string(state->games_per_match_max);
        TUI_Text error = {
            .text = range,
            .style = NORMAL_INTENSITY,
            .fg = RED,
            .bg = RESET,
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
            .style = BOLD,
            .fg = RESET,
            .bg = RESET,
        };
        TUI_TextGroup_Add(&enter_group, disabled);
    }
    TUI_Window_Add(window, TUI_Line_Default(enter_group));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Esc/q    Quit"))));
}

bool TUI_GamesPerMatch_Input(TUI_OptionsState *state)
{
    TUI_Input input = TUI_Input_Get(TUI_INPUT_60FPS, true);
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
        }
        break;
    case INPUT_TEXT:
        if (input.value == 'q') return true;
        break;
    case INPUT_ESC:
        return true;
    default:
        break;
    }
    return false;
}

bool TUI_Options_Get(TUI_Options *options, vector<BShip_AIFileData> ais, bool debug)
{
    bool should_exit = false;

    (void)options;
    (void)ais;
    (void)debug;
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
        // NOTE(mattg): We'll set this after we get the board size
        .games_per_match_selection = 0,
        .games_per_match_min = 0,
        .games_per_match_max = 0,
        .games_per_match_string = "",
        .games_per_match_invalid = false,
        .ai1_selection = 0,
        .ai2_selection = 0,

    };
    size_t lines_size = 0, buffer_size = 0;

    if (!TUI_Window_Enter(&window))
    {
        should_exit = true;
        goto on_exit;
    }

    while (!TUI_Should_Close() && !should_exit)
    {
        lines_size = window.lines.size();
        buffer_size = window.buffer.size();
        TUI_Window_Print(&window);
        if (TUI_Should_Resize())
        {
            TUI_WindowSize_Get(&window.size);
        }
        TUI_Window_Reset(&window);
        if (debug)
        {
            string width_height = to_string(window.size.width) + "x" + to_string(window.size.height);
            TUI_TextGroup group = TUI_TextGroup_Default(TUI_Text_Default(width_height));
            string line_caps = " " + to_string(lines_size) + "/" + to_string(window.lines.capacity());
            TUI_TextGroup_Add(&group, TUI_Text_Default(line_caps));
            string buffer_caps = " " + to_string(buffer_size) + "/" + to_string(window.buffer.capacity());
            TUI_TextGroup_Add(&group, TUI_Text_Default(buffer_caps));

            TUI_Window_Add(&window, TUI_Line_Default(group));
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

        TUI_RuntimeType_Display(&window, &state);
        if (!state.runtime.has_value())
        {
            should_exit = TUI_RuntimeType_Input(&state);
            continue;
        }
        else options->runtime = state.runtime.value();

        TUI_BoardSize_Display(&window, &state);
        if (!state.board_size.has_value())
        {
            should_exit = TUI_BoardSize_Input(&state);
            continue;
        }
        else options->board_size = state.board_size.value();

        TUI_GamesPerMatch_Display(&window, &state);
        if (!state.games_per_match.has_value())
        {
            should_exit = TUI_GamesPerMatch_Input(&state);
            continue;
        }
        else options->games_per_match = state.games_per_match.value();

        break;
    }
on_exit:
    TUI_Window_Exit(&window);
    if (TUI_Should_Close())
    {
        should_exit = true;
    }
    return true; // should_exit;
}

