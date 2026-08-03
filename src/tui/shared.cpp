/**
 * @file shared.cpp
 * @author Matthew Getgen
 * @brief Shared Battleships TUI code.
 * @date 2026-07-15
 */

#ifndef TUI_SHARED_CPP
#define TUI_SHARED_CPP

#include "tui.cpp"

using namespace std;
using namespace conio;

typedef enum {
    TUI_MATCH_DISPLAY_NONE,
    TUI_MATCH_DISPLAY_LAST,
    TUI_MATCH_DISPLAY_ALL,
    // TODO(mattg): Support other match display types in the future.
    // TUI_MATCH_DISPLAY_EACH_RESULT,
    // TUI_MATCH_DISPLAY_EVERY_NTH,
    // TUI_MATCH_DISPLAY_CHOOSE,
} TUI_MatchDisplayType;

void TUI_Debug_Line_Add(TUI_Window *window, TUI_Input input)
{
    string width_height = to_string(window->size.width) + "x" + to_string(window->size.height);
    TUI_Line debug_line = TUI_Line_Default(TUI_TextGroup_Default(TUI_Text_Default(width_height)));

    TUI_TextGroup debug_msg_group = TUI_TextGroup_Default(TUI_Text_New(" * DEBUG * ", {}, RESET, RED));
    debug_msg_group.column = (window->size.width / 2) - (TUI_TextGroup_Size(debug_msg_group) / 2);
    if (debug_msg_group.column < 2 || debug_msg_group.column > window->size.width)
    {
        debug_msg_group.column = 2;
    }
    TUI_Line_Add(&debug_line, debug_msg_group);

    string input_type = "";
    switch (input.type)
    {
    case INPUT_NONE:
        input_type = "N/A";
        break;
    case INPUT_UP:
        input_type = "^ UP";
        break;
    case INPUT_DOWN:
        input_type = "v DOWN";
        break;
    case INPUT_LEFT:
        input_type = "< LEFT";
        break;
    case INPUT_RIGHT:
        input_type = "> RIGHT";
        break;
    case INPUT_SCROLL_UP:
        input_type = "^ SCROLL UP";
        break;
    case INPUT_SCROLL_DOWN:
        input_type = "v SCROLL DOWN";
        break;
    case INPUT_BACKSPACE:
        input_type = "BACKSPACE";
        break;
    case INPUT_ENTER:
        input_type = "ENTER";
        break;
    case INPUT_ESC:
        input_type = "ESC";
        break;
    case INPUT_PERIOD:
        input_type = "PERIOD";
        break;
    case INPUT_NUM:
        input_type = "NUM ";
        input_type += input.value;
        break;
    }
    TUI_TextGroup input_group = TUI_TextGroup_Default(TUI_Text_Default(input_type));
    input_group.column = window->size.width - TUI_TextGroup_Size(input_group) - 1;
    if (input_group.column < 3 || input_group.column > window->size.width)
    {
        input_group.column = 3;
    }
    TUI_Line_Add(&debug_line, input_group);

    TUI_Window_Add(window, debug_line);
}

#endif // TUI_SHARED_CPP

