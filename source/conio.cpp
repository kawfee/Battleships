/** 
 * \file conio.cpp
 * \author Stefan Brandle
 * \version 0.1
 * \date 2008
 * \brief conio implements a partial clone of Borland's console I/O.
 *
 * Conio implements the text functionality of Borland's console I/O. That includes
 * cursor positioning, setting foreground/background colors, setting text styles,
 * resetting text attributes, and clearing the screen.
 *
 * Definition/implementation of the console I/O functions
 *     Reference: http://en.wikipedia.org/wiki/ANSI_escape_code
 */ 

#ifndef CONIO_CPP
#define CONIO_CPP

#include <iostream>
#include <sstream>
#include "conio.h"

using namespace std;

/** \namespace
 *   The conio functions are not part of a class. They are just a set of functions
 *   collected within the conio namespace.
 */
namespace conio {

/** \brief Positions cursor to the specified row, col location.
 *
 * Gotoxy will position the cursor at the specified row,col location. The upper left corner
 * has the coordinates 1,1. Note: this is row,col, not x,y (col,row) as is often done.
 *
 * \param row The row coordinate (1-based).
 * \param col The column coordinate (1-based).
 * \return Returns a string containing the escape sequence to send to the screen.
 * gotoRowCol = CSI r;c
 */
string gotoRowCol( const int row, const int col ) {
    std::ostringstream strm;        // create the string stream
    strm << CSI << row << ';' << col << 'H';    // insert the goodies
    return strm.str();            // return a string with the info
}

const int Foreground = 1;    // local implementation-specific values
const int Background = 2;

string getColorSequence( Color c, int fgOrBg ) {
    int BGOFFSET = 10;
    int offset = 0;
    if( fgOrBg == Background ) offset += BGOFFSET; 

    std::ostringstream strm;    // create the string stream

    switch( c ) {
        case BLACK:
        case RED:
        case GREEN:
        case YELLOW:
        case BLUE:
        case MAGENTA:
        case CYAN:
        case LIGHT_GRAY:
        case RESET:
        case GRAY:
        case LIGHT_RED:
        case LIGHT_GREEN:
        case LIGHT_YELLOW:
        case LIGHT_BLUE:
        case LIGHT_MAGENTA:
        case LIGHT_CYAN:
        case WHITE:
            strm << CSI << Color( c+offset ) << 'm';    // insert the goodies
            break;
        default:
            strm << "conio: invalid color: " << int(c) << endl;
    }
    return strm.str();		// return a string with the info
}

/** \brief Returns a string that contains the escape sequence to set the
 * foreground color to the specified color.
 * \param c The Color valoue to use for the text foreground color.
 * \return A string containing the entire escape sequence to be output
 *     to the terminal to set the foreground color.
 */
string fgColor( Color c ) {
    return getColorSequence( c, Foreground );	// insert the goodies
}

/** \brief Returns a string that contains the escape sequence to set the
 * foreground color to the specified color.
 * \param c The Color valoue to use for the text background color.
 * \return A string containing the entire escape sequence to be output
 *     to the terminal to set the background color.
 */
string bgColor( Color c ) {
    return getColorSequence( c, Background );	// insert the goodies
}

/** \brief Returns a string that contains the escape sequence to set the
 * text style to the specified TextStyle.
 * \param ts The TextStyle valoue to use for the text style.
 * \return A string containing the entire escape sequence to be output
 *     to the terminal to set the text style.
 */
string setTextStyle( TextStyle ts ) {
    std::ostringstream strm;	// create the string stream
    strm << CSI << ts << 'm';	// insert the goodies
    return strm.str();		// return a string with the info
}


/** \brief Returns a string that contains the escape sequence to reset 
 * all the text attributes to default.
 * \return A string containing the entire escape sequence to be output
 *     to the terminal to reset text output to the default.
 */
string resetAll( ) {
    std::ostringstream strm;	// create the string stream
    strm << CSI << "0m";		// insert the goodies
    return strm.str();		// return a string with the info
}

/** \brief Returns a string that contains the escape sequence to clear
 * the screen.
 * \return A string containing the entire escape sequence to be output
 *     to the terminal to clear the screen.
 */
string clrscr() {
    std::ostringstream strm;	// create the string stream
    strm << CSI << "H" << CSI << "2J";		// insert the goodies
    return strm.str();		// return a string with the info
}

}           // End of namespace conio

#endif		// ifdef CONIO_CPP

