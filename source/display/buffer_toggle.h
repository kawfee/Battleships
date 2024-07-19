/**
 * @file buffer_toggle.h
 * @author Matthew Getgen, Luc Lieber
 * @brief Used to turn buffered input on or off in the Linux Terminal.
 * @date 2022-12-08
 * 
 * BufferToggle from [C++ Forum](https://cplusplus.com/forum/general/29137/)
 */

 #ifndef BUFFER_TOGGLE_H
 #define BUFFER_TOGGLE_H

#include <termios.h>
#include <unistd.h>

class BufferToggle {
    private:
        termios t;

    public:
        /**
         * @brief Disables buffered input.
         */
        void off(void) {
            tcgetattr(STDIN_FILENO, &t);    // get the current terminal I/O structure.
            t.c_lflag &= ~ICANON;           // Manipulate the flag bits to do what you want it to do.
            tcsetattr(STDIN_FILENO, TCSANOW, &t);   // Apply the new settings;
            return;
        }

        /**
         * @brief Enables buffered input.
         */
        void on(void) {
            tcgetattr(STDIN_FILENO, &t);    // get the current terminal I/O structure
            t.c_lflag |= ICANON;            // Manipulate the flag bits to do what you want it to do
            tcsetattr(STDIN_FILENO, TCSANOW, &t);   // Apply the new settings
        }
};

#endif

