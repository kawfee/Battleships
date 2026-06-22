/**
 * @file options.cpp
 * @author Matthew Getgen
 * @brief Battleships tui input options.
 * @date 2026-06-18
 */

#include <optional>

#include "tui.cpp"
#include "../../lib/battleshipslib.h"

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
    optional<BShip_AIFileData> ai1;
    optional<BShip_AIFileData> ai2;
    optional<uint32_t> games_per_match;
    optional<uint8_t> board_size;
    size_t selection;
} TUI_OptionsState;

void TUI_RuntimeType_Display(TUI_Window *window, TUI_OptionsState *state)
{
    typedef struct {
        string text;
        TUI_RuntimeType type;
    } RuntimeTypeTable;
    RuntimeTypeTable runtimes[] = {
        [RUNTIME_NONE] = {
            .text = "NONE",
            .type = RUNTIME_NONE,
        },
        [RUNTIME_MATCH] = {
            .text = "Test AI",
            .type = RUNTIME_MATCH,
        },
        [RUNTIME_CONTEST] = {
            .text = "Run Contest",
            .type = RUNTIME_CONTEST,
        },
        [RUNTIME_REPLAY_MATCH] = {
            .text = "Replay Test",
            .type = RUNTIME_REPLAY_MATCH,
        },
        [RUNTIME_REPLAY_CONTEST] = {
            .text = "Replay Contest",
            .type = RUNTIME_REPLAY_CONTEST,
        },
    };

    TUI_Text prompt = TUI_Text_Default("Runtime: ");
    TUI_TextGroup prompt_group = TUI_TextGroup_Default(prompt);
    if (state->runtime.has_value() && state->runtime.value() != RUNTIME_NONE)
    {
        TUI_Text value = {
            .text = runtimes[state->runtime.value()].text,
            .style = BOLD,
            .fg = RESET,
            .bg = RESET,
        };
        TUI_TextGroup_Add(&prompt_group, value);
        TUI_Window_Add(window, TUI_Line_Default(prompt_group));
        return;
    }
    TUI_Window_Add(window, TUI_Line_Default(prompt_group));

    // bounds
    if (state->selection == 0) state->selection = 1;
    if (state->selection >= TUI_ARRAY_LENGTH(runtimes)) state->selection = TUI_ARRAY_LENGTH(runtimes)-1;

    // NOTE(mattg): skip RUNTIME_NONE
    for (size_t i = 1; i < TUI_ARRAY_LENGTH(runtimes); i++)
    {
        TUI_Text runtime = TUI_Text_Default((state->selection == i ? " > " : "    ") + runtimes[i].text);
        TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(runtime)));
    }
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(""))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("↑ ↓ j/k  Move"))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Enter    Select"))));
    TUI_Window_Add(window, TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default("Esc/q    Quit"))));
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
        .ai1 = {},
        .ai2 = {},
        .games_per_match = {},
        .board_size = {},
        .selection = 0,
    };

    if (!TUI_Window_Enter(&window))
    {
        should_exit = true;
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
            string width_height = to_string(window.size.width) + "x" + to_string(window.size.height);
            TUI_Text text = TUI_Text_Default(width_height);
            TUI_TextGroup group = TUI_TextGroup_Default(text);
            TUI_Line line = TUI_Line_Default(group);
            TUI_Window_Add(&window, line);
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

        TUI_Window_Print(&window);
        // TODO(mattg): wait for refresh rate time.
        sleep(1);
    }
on_exit:
    TUI_Window_Exit(&window);
    if (TUI_Should_Close())
    {
        should_exit = true;
    }
    return true; // should_exit;
}

