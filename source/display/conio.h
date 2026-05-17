/** 
 * \brief Declarations for the console I/O functions namespace.
 * \author Stefan Brandle
 * \file conio.h
 */ 

#ifndef CONIO_H
#define CONIO_H

#include <string>

/** 
 * \brief The conio namespace is a collection of functions that perform
 * console I/O functions.
 */
namespace conio {

    /** 
     * \brief The Color enumeration lists all the valid colors (foreground and background). 
     */
    enum Color { 
        // Normal intensity
        BLACK=30, RED=31, GREEN=32, YELLOW=33, BLUE=34, MAGENTA=35, 
        CYAN=36, LIGHT_GRAY=37, RESET=39,
        
        // High intensity
        GRAY=90, LIGHT_RED=91, LIGHT_GREEN=92, LIGHT_YELLOW=93, LIGHT_BLUE=94, 
        LIGHT_MAGENTA=95, LIGHT_CYAN=96, WHITE=97 
    };

    /**
     * \brief The TextStyle enumeration lists all the valid text styles. 
     * Many are only partially supported by most terminal emulators, so 
     * don't be surprised if a particular style -- such as DOUBLE_UNDERLINE 
     * -- does not show up correctly.
     */
    enum TextStyle { BOLD=1, FAINT=2, ITALIC=3, UNDERLINE=4, SLOW_BLINK=5, RAPID_BLINK=6, 
        NEGATIVE_IMAGE=7, CONCEAL=8, DOUBLE_UNDERLINE=21, NORMAL_INTENSITY=22, 
        REVEAL=28 
    };

    /**
     * \brief The CSI C-string is at the start of all the escape code sequences.
     * The CSI C-string contains the escape code sequence that must be printed first
     * for the various sequences. Output it, then the rest of the codes.
     */
    const char CSI[] = {'\033', '[', '\0'};

    std::string gotoRowCol( const int x, const int y );
    std::string gotoNextRow();
    std::string gotoPrevRow();
    std::string gotoNextCol();
    std::string gotoPrevCol();
    std::string fgColor( Color c );
    std::string bgColor( Color c );
    std::string setTextStyle( TextStyle ts );
    std::string resetAll( );
    std::string clearScreen();
    std::string clearRow();
}

#endif        // ifdef CONIO_H
