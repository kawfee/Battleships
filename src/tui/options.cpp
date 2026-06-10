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
    MATCH,
    CONTEST,
    REPLAY_MATCH,
    REPLAY_CONTEST,
};

struct TUI_Options {
    TUI_RuntimeType type;
    char *ai1_path;
    char *ai2_path;
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

static inline void TUI_WriteBuffer(string buffer)
{
    write(STDOUT_FILENO, buffer.data(), buffer.size());
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
    string start_buffer = enterAltScreen() + clearScreen() + TUI_Cursor_Goto(cursor);
    start_buffer += "Welcome to the Battleships AI Contest and Tester!";
    cursor.row++;
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
    {
        typedef struct {
            string text;
            TUI_RuntimeType type;
        } RuntimeTypeMap;
        RuntimeTypeMap runtimes[] = {
            {
                .text = "Test AI",
                .type = MATCH,
            },
            {
                .text = "Run Contest",
                .type = CONTEST,
            },
            {
                .text = "Replay Test",
                .type = REPLAY_MATCH,
            },
            {
                .text = "Replay Contest",
                .type = REPLAY_CONTEST,
            },
        };

        TUI_Cursor top = cursor;
        string print_buffer = "";
        size_t selected = 0;

        string prompt = "Please choose one of the following: ";
        char input = '\0';
        while (running)
        {
            cursor = top;
            print_buffer = "";
            print_buffer += TUI_Cursor_Goto(cursor) + prompt;
            TUI_Cursor prompt_cursor = cursor;
            prompt_cursor.column += prompt.size();

            int input_number = (int)input - (int)'0';
            if (input_number >= 0 && input_number < (int)ARRAY_LENGTH(runtimes))
            {
                selected = input_number;
                print_buffer += input;
                prompt_cursor.column++;
            }
            else
            {
                print_buffer += " ";
            }
            cursor.row++;

            for (size_t i = 0; i < ARRAY_LENGTH(runtimes); i++)
            {
                print_buffer += TUI_Cursor_Goto(cursor) + "   ";
                if (selected == i)
                {
                    print_buffer += setTextStyle(BOLD) + "[" + to_string(i) + "] " + runtimes[i].text + resetAll();
                }
                else
                {
                    print_buffer += " " + to_string(i) + "  " + runtimes[i].text;
                }
                cursor.row++;
            }
            cursor.row++;
            print_buffer += TUI_Cursor_Goto(cursor) + setTextStyle(ITALIC) + fgColor(LIGHT_GRAY);
            print_buffer += "? Use arrow keys or type the number and hit ENTER to select" + resetAll() + TUI_Cursor_Goto(prompt_cursor);
            TUI_WriteBuffer(print_buffer);

            struct pollfd pfd;
            pfd.fd = STDIN_FILENO;
            pfd.events = POLLIN;

            poll(&pfd, 1, -1);

            if (pfd.revents & POLLIN)
            {
                read(STDIN_FILENO, &input, 1);
            }
        }
    }

on_exit:
    string end_buffer = resetAll() + leaveAltScreen();
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
