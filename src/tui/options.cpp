/**
 * @file options.cpp
 * @author Matthew Getgen
 * @brief Battleships tui input options.
 * @date 2026-06-07
 */

#include <csignal>
#include <iostream>
#include <poll.h>
#include <string>
#include <termios.h>

#include "conio.cpp"
#include "../../lib/battleshipslib.h"

#define ARRAY_LENGTH(array) \
    (sizeof(array) / sizeof(array[0]))

using namespace std;
using namespace conio;

volatile sig_atomic_t running = 1;

void TUI_Signal_Close_Handler(int sig)
{
    (void)sig;
    running = 0;
}

enum TUI_RuntimeType {
    RUNTIME_NONE,
    RUNTIME_MATCH,
    RUNTIME_CONTEST,
    RUNTIME_REPLAY_MATCH,
    RUNTIME_REPLAY_CONTEST,
};

struct TUI_Options {
    TUI_RuntimeType type;
    string ai1_path;
    string ai2_path;
    uint32_t games_per_match;
    uint8_t board_size;
};

struct TUI_Cursor {
    int row;
    int column;
};

static inline string TUI_Cursor_Goto(TUI_Cursor cursor)
{
    return gotoRowCol(cursor.row, cursor.column);
}

static inline string TUI_Line_Append(TUI_Cursor *cursor, const char *input)
{
    string str = TUI_Cursor_Goto(*cursor) + input;
    cursor->row++;
    return str;
}

static inline void TUI_WriteBuffer(string buffer)
{
    write(STDOUT_FILENO, buffer.data(), buffer.size());
}

enum TUI_KeyPress {
    KEY_NONE,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_ENTER,
    KEY_ESC,
};

TUI_KeyPress TUI_KeyPress_Get()
{
    struct pollfd pfd;
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;

    poll(&pfd, 1, -1);

    char input = '\0';
    if (pfd.revents & POLLIN)
    {
        read(STDIN_FILENO, &input, 1);
    }
    TUI_KeyPress key = KEY_NONE;
    switch (input)
    {
    case 'h':
        key = KEY_LEFT;
        break;
    case 'j':
        key = KEY_DOWN;
        break;
    case 'k':
        key = KEY_UP;
        break;
    case 'l':
        key = KEY_RIGHT;
        break;
    case 'q':
        key = KEY_ESC;
        break;
    case '\r':
    case '\n':
        key = KEY_ENTER;
        break;
    default:
        key = KEY_NONE;
    }
    if (input != '\x1b')
    {
        return key;
    }
    int rc = poll(&pfd, 1, 50);
    if (rc == 0)
    {
        return KEY_ESC;
    }
    
    char a = '\0', b = '\0';
    read(STDIN_FILENO, &a, 1);
    if (a != '[')
    {
        return KEY_NONE;
    }
    read(STDIN_FILENO, &b, 1);
    switch (b)
    {
    case 'A':
        key = KEY_UP;
        break;
    case 'B':
        key = KEY_DOWN;
        break;
    case 'C':
        key = KEY_RIGHT;
        break;
    case 'D':
        key = KEY_LEFT;
        break;
    default:
        key = KEY_NONE;
    }
    return key;
}

TUI_RuntimeType TUI_RuntimeType_Get(TUI_Cursor top, bool *should_exit)
{
    typedef struct {
        string text;
        TUI_RuntimeType type;
    } RuntimeTypeMap;
    RuntimeTypeMap runtimes[] = {
        {
            .text = "Test AI",
            .type = RUNTIME_MATCH,
        },
        {
            .text = "Run Contest",
            .type = RUNTIME_CONTEST,
        },
        {
            .text = "Replay Test",
            .type = RUNTIME_REPLAY_MATCH,
        },
        {
            .text = "Replay Contest",
            .type = RUNTIME_REPLAY_CONTEST,
        },
    };

    string print_buffer = "";
    size_t selection = 0;
    TUI_KeyPress key = KEY_NONE;
    TUI_Cursor cursor = top;

    string prompt = "Select a runtime";
    while (running)
    {
        cursor = top;
        print_buffer = "";
        print_buffer += TUI_Line_Append(&cursor, prompt.c_str());

        for (size_t i = 0; i < ARRAY_LENGTH(runtimes); i++)
        {
            print_buffer += TUI_Cursor_Goto(cursor);
            print_buffer += selection == i ? "  > " : "    ";
            print_buffer += runtimes[i].text;
            cursor.row++;
        }
        cursor.row++;
        print_buffer += TUI_Line_Append(&cursor, "↑ ↓ j/k  Move");
        print_buffer += TUI_Line_Append(&cursor, "Enter    Select");
        print_buffer += TUI_Line_Append(&cursor, "Esc/q    Quit");

        TUI_WriteBuffer(print_buffer);

        key = TUI_KeyPress_Get();

        bool selected = false;
        switch (key)
        {
        case KEY_ESC:
            *should_exit = true;
            return RUNTIME_NONE;
            break;
        case KEY_UP:
            if (selection > 0) selection--;
            break;
        case KEY_DOWN:
            if (selection < ARRAY_LENGTH(runtimes)-1) selection++;
            break;
        case KEY_ENTER:
            selected = true;
            break;
        default:
            break;
        }
        if (selected)
        {
            break;
        }
    }
    string cleanup_buffer = "";
    cursor.column = 1;
    for (cursor.row = cursor.row; cursor.row >= top.row; cursor.row--)
    {
        cleanup_buffer += TUI_Cursor_Goto(cursor) + clearRow();
    }
    cleanup_buffer += "Runtime: " + setTextStyle(BOLD) + runtimes[selection].text + resetAll();
    TUI_WriteBuffer(cleanup_buffer);

    return runtimes[selection].type;
}

uint8_t TUI_BoardSize_Get(TUI_Cursor top, bool *should_exit)
{
    string print_buffer = "";
    uint8_t selection = 10; // sets the default
    TUI_KeyPress key = KEY_NONE;
    TUI_Cursor cursor = top;

    string prompt = "Select a board size";
    while (running)
    {
        cursor = top;
        print_buffer = TUI_Line_Append(&cursor, prompt.c_str());

        string prefix = selection == BSHIP_BOARD_SIZE_MIN ? "      " : "    - ";
        string postfix = selection == BSHIP_BOARD_SIZE_MAX ? "" : " +";
        string string_selection = to_string(selection);
        if (string_selection.size() == 1) string_selection = " " + string_selection;
        print_buffer += TUI_Cursor_Goto(cursor) + clearRow() + prefix + string_selection + postfix;
        cursor.row += 2;

        print_buffer += TUI_Line_Append(&cursor, "← → h/l  Move");
        print_buffer += TUI_Line_Append(&cursor, "Enter    Select");
        print_buffer += TUI_Line_Append(&cursor, "Esc/q    Quit");

        TUI_WriteBuffer(print_buffer);

        key = TUI_KeyPress_Get();

        bool selected = false;
        switch (key)
        {
        case KEY_ESC:
            *should_exit = true;
            return selection;
            break;
        case KEY_LEFT:
            if (selection > BSHIP_BOARD_SIZE_MIN) selection--;
            break;
        case KEY_RIGHT:
            if (selection < BSHIP_BOARD_SIZE_MAX) selection++;
            break;
        case KEY_ENTER:
            selected = true;
            break;
        default:
            break;
        }
        if (selected)
        {
            break;
        }
    }
    string cleanup_buffer = "";
    cursor.column = 1;
    for (cursor.row = cursor.row; cursor.row >= top.row; cursor.row--)
    {
        cleanup_buffer += TUI_Cursor_Goto(cursor) + clearRow();
    }
    cleanup_buffer += "Board Size: " + setTextStyle(BOLD) + to_string(selection) + resetAll();
    TUI_WriteBuffer(cleanup_buffer);
    return selection;
}

TUI_Options TUI_Options_Get(bool debug, bool *should_exit)
{
    TUI_Options options = {};

    struct termios original = {};
    tcgetattr(STDIN_FILENO, &original);

    struct termios raw = original;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1; // Wait until at least 1 byte is available.
    raw.c_cc[VTIME] = 0; // No timeout
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    typedef struct {
        int signum;
        sighandler_t handler;
    } SignalHandler;

    SignalHandler signals[] = {
        {
            .signum = SIGINT,
            .handler = TUI_Signal_Close_Handler,
        },
        {
            .signum = SIGTERM,
            .handler = TUI_Signal_Close_Handler,
        },
        {
            .signum = SIGSEGV,
            .handler = TUI_Signal_Close_Handler,
        },
        {
            .signum = SIGQUIT,
            .handler = TUI_Signal_Close_Handler,
        },
    };

    TUI_Cursor cursor = {1, 1};
    string start_buffer = enterAltScreen() + clearScreen() + hideCursor() + TUI_Cursor_Goto(cursor);
    string banner = R"(
  ____        _   _   _           _
 | __ )  __ _| |_| |_| | ___  ___| |__  _ _ __  ___
 |  _ \ / _` | __| __| |/ _ \/ __| '_ \| | '_ \/ __|
 | |_) | (_| | |_| |_| |  __/\__ \ | | | | |_) \__ \
 |____/ \__,_|\__|\__|_|\___||___/_| |_|_| .__/|___/
                                         |_|

                B A T T L E S H I P S
----------------------------------------------------
)";
    start_buffer += banner;
    cursor.row += 10;
    if (debug)
    {
        start_buffer += TUI_Cursor_Goto(cursor) + "DEBUG MODE ENABLED";
    cursor.row++;
    }
    cursor.row++;

    for (size_t i = 0; i < ARRAY_LENGTH(signals); i++)
    {
        SignalHandler sig = signals[i];
        if (signal(sig.signum, sig.handler) == SIG_ERR)
        {
            PRINT_ERROR("signals failed");
            goto on_exit;
        }
    }
    TUI_WriteBuffer(start_buffer);

    options.type = TUI_RuntimeType_Get(cursor, should_exit);
    if (*should_exit || !running)
    {
        goto on_exit;
    }

    cursor.row++;
    options.board_size = TUI_BoardSize_Get(cursor, should_exit);
    if (*should_exit || !running)
    {
        goto on_exit;
    }


on_exit:
    string end_buffer = resetAll() + showCursor() + leaveAltScreen();
    TUI_WriteBuffer(end_buffer);
    for (size_t i = 0; i < ARRAY_LENGTH(signals); i++)
    {
        int signum = signals[i].signum;
        signal(signum, SIG_DFL);
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &original);

    if (!running)
    {
        *should_exit = true;
    }
    return options;
}

