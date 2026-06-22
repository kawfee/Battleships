/**
 * @file tui.cpp
 * @author Matthew Getgen
 * @brief Battleships TUI implementation.
 * @date 2026-06-18
 */

#include <csignal>
#include <cerrno>
#include <iostream>
#include <poll.h>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

#include "conio.cpp"

#define TUI_ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

using namespace std;

volatile sig_atomic_t GLOBAL_RUNNING = 1;
volatile sig_atomic_t GLOBAL_RESIZED = 0;

void TUI_Signal_Close_Handler(int sig)
{
    (void)sig;
    GLOBAL_RUNNING = 0;
}

void TUI_Signal_Resize_Handler(int sig)
{
    (void)sig;
    GLOBAL_RESIZED = 1;
}

bool TUI_Should_Close(void)
{
    return !GLOBAL_RUNNING;
}

typedef struct {
    uint32_t width;
    uint32_t height;
} TUI_WindowSize;

bool TUI_Should_Resize(void)
{
    return GLOBAL_RESIZED;
}

bool TUI_WindowSize_Get(TUI_WindowSize *size)
{
    GLOBAL_RESIZED = 0;
    struct winsize ws = {};

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
    {
        perror("ioctl");
        return false;
    }
    size->width = ws.ws_col;
    size->height = ws.ws_row;
    return true;
}

typedef struct {
    int signum;
    sighandler_t handler;
} TUI_SignalHandler;

static const TUI_SignalHandler GLOBAL_TUI_SIGNALS[] = {
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
    {
        .signum = SIGWINCH,
        .handler = TUI_Signal_Resize_Handler,
    },
};

void TUI_Buffer_Write(string buffer)
{
    write(STDOUT_FILENO, buffer.data(), buffer.size());
}

typedef struct {
    string text;
    conio::TextStyle style;
    conio::Color fg;
    conio::Color bg;
} TUI_Text;

TUI_Text TUI_Text_Default(string str)
{
    TUI_Text text = {
        .text = str,
        .style = conio::NORMAL_INTENSITY,
        .fg = conio::RESET,
        .bg = conio::RESET,
    };
    return text;
}

typedef struct {
    vector<TUI_Text> text;
    uint32_t column;
} TUI_TextGroup;

void TUI_TextGroup_Add(TUI_TextGroup *group, TUI_Text text)
{
    group->text.push_back(text);
}

TUI_TextGroup TUI_TextGroup_Default(TUI_Text text)
{
    TUI_TextGroup group = {
        .text = {},
        .column = 1,
    };
    TUI_TextGroup_Add(&group, text);
    return group;
}

string TUI_String_From_TextGroup(TUI_TextGroup group, uint32_t space)
{
    string buffer = "";
    if (space < 3)
    {
        return buffer;
    }
    else if (space == 3)
    {
        buffer += "...";
        return buffer;
    }
    uint32_t text_width = 0;
    for (size_t i = 0; i < group.text.size(); i++)
    {
        TUI_Text text = group.text.at(i);
        if (text_width >= space)
        {
            break;
        }
        uint32_t leftover_space = space - text_width;
        if (text.text.size() >= leftover_space)
        {
            text.text.resize(leftover_space-3);
            text.text += "...";
        }
        buffer += text.style == conio::NORMAL_INTENSITY ? "" : conio::setTextStyle(text.style);
        buffer += text.fg == conio::RESET ? "" : conio::fgColor(text.fg);
        buffer += text.bg == conio::RESET ? "" : conio::bgColor(text.bg);
        buffer += text.text;
        buffer += text.bg == conio::RESET ? "" : conio::bgColor(conio::RESET);
        buffer += text.fg == conio::RESET ? "" : conio::fgColor(conio::RESET);
        buffer += text.style == conio::NORMAL_INTENSITY ? "" : conio::setTextStyle(conio::NORMAL_INTENSITY);
        text_width += text.text.size();
    }
    return buffer;
}

typedef struct {
    vector<TUI_TextGroup> line;
} TUI_Line;

void TUI_Line_Add(TUI_Line *line, TUI_TextGroup group)
{
    assert(group.column > 0);
    line->line.push_back(group);
}

TUI_Line TUI_Line_Default(TUI_TextGroup group)
{
    TUI_Line line = {};
    TUI_Line_Add(&line, group);
    return line;
}

string TUI_String_From_Line(TUI_Line line, uint32_t row, uint32_t window_width)
{
    assert(row > 0);
    assert(window_width > 0);
    string buffer = "";
    for (size_t i = 0; i < line.line.size(); i++)
    {
        TUI_TextGroup group = line.line.at(i);
        uint32_t space = window_width;
        if (line.line.size() > 1 && i+1 < line.line.size())
        {
            uint32_t next_column = line.line.at(i+1).column;
            space = next_column - 1;
        }
        buffer += conio::gotoRowCol(row, group.column) + TUI_String_From_TextGroup(group, space);
    }
    return buffer;
}

typedef struct {
    struct termios original;
    TUI_WindowSize size;
    vector<TUI_Line> lines;
    string buffer;
} TUI_Window;

bool TUI_Window_Enter(TUI_Window *window)
{
    tcgetattr(STDIN_FILENO, &window->original);

    struct termios raw = window->original;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;  // Don't wait for any bytes to be available.
    raw.c_cc[VTIME] = 0; // No timeout
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    for (size_t i = 0; i < TUI_ARRAY_LENGTH(GLOBAL_TUI_SIGNALS); i++)
    {
        TUI_SignalHandler sig = GLOBAL_TUI_SIGNALS[i];
        if (signal(sig.signum, sig.handler) == SIG_ERR)
        {
            perror("signal");
            return false;
        }
    }
    window->buffer = conio::enterAltScreen() + conio::clearScreen() + conio::hideCursor();
    TUI_Buffer_Write(window->buffer);

    return true;
}

void TUI_Window_Exit(TUI_Window *window)
{
    window->buffer = conio::resetAll() + conio::showCursor() + conio::leaveAltScreen();
    TUI_Buffer_Write(window->buffer);

    for (size_t i = 0; i < TUI_ARRAY_LENGTH(GLOBAL_TUI_SIGNALS); i++)
    {
        int signum = GLOBAL_TUI_SIGNALS[i].signum;
        signal(signum, SIG_DFL);
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &window->original);
}

void TUI_Window_Reset(TUI_Window *window)
{
    window->buffer.clear();
    window->lines.clear();
    window->buffer = conio::resetAll() + conio::clearScreen() + conio::gotoRowCol(1, 1);
}

void TUI_Window_Add(TUI_Window *window, TUI_Line line)
{
    window->lines.push_back(line);
}

void TUI_Window_Print(TUI_Window *window)
{
    window->buffer.clear();
    window->buffer = conio::resetAll() + conio::clearScreen() + conio::gotoRowCol(1, 1);
    size_t line_count = window->lines.size() > window->size.height ? window->size.height : window->lines.size();
    for (size_t i = 0, row = 1; i < line_count; i++, row++)
    {
        window->buffer += TUI_String_From_Line(window->lines.at(i), row, window->size.width);
    }
    TUI_Buffer_Write(window->buffer);
}

