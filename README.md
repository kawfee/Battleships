# Battleships Contest
#### _By Matthew Getgen, Joey Gorski, Matthew Bouch, and Stefan Brandle_
***

This project's goal is to handle a Battleships Contest between AIs through JSON messages and Unix Domain Sockets.


## Repository Information
---

Welcome to the Battleships Contest! This repository has all of the code
to make a new AI and compete against other Battleship AI.

### Definitions:
- `Game`: Starts by placing ships, then taking shots at the opponent's board. Ends when all of an AI's ships are killed (Like a game of battleships with your friends).
- `Match`: Set number of games between two AI players (Usually 500).
- `Contest`: Set of matches between all available players with a winner.

### Project Directories:
- `source/` -- Where game logic, message, display, and logging code is stored.
- `logs/` -- Where the log files are stored.
- `AI_Files/` -- Where you will store your AI code.
- `AI_Files/protected/` -- Where the protected AIs and protected AI functions are stored.

### Make Instructions:
- `make all` -- compiles the controller and all available C++ AIs.
- `make clean` -- cleans up the controller and AI executables, logs, etc.
- `make controller` -- compiles the controller only.
- `make player` -- compiles all available C++ AIs.
- `make clean_player` -- cleans up all AI executables.
- If you are using C++, you will need to run `make player` to compile any changes.
- If you are using Python, you won't have to worry about compiling, just make changes and re-run the controller.


## Install
---

### To install the battleships project:
- Download this repo to a relatively recent Linux computer (at Taylor University: matt, lab machine, or personal).
- `cd` into the main project directory.
- Run `make all` to make the controller code and all players.
    - You should only need to do this once, or after a `make clean`.


## Usage
---

### To use the battleships project:
- Run the controller either by `make run` or `./controller` and answer the runtime questions.
- In general, the defaults are great! Just hit enter on questions with defaults.

### Runtime Questions:
- `Test AI` -- Plays a match between two AI (you must choose the AI to play).
    - Use this in the development of your AI.
- `Run Contest` -- Plays a contest between all available AIs.
    - Use this when hosting a contest.
    - Can choose interactively which AIs will participate.
- `Replay Logs` -- Replays a log file for you.
    - Replays log files from either the last match or contest played.


## How to Create a new C++ or Python AI:
---

- Copy or rename the `player_example.cpp` and `player_example.h` for C++, or `player_example.py` for Python file.
- Replace the default `AI_NAME` variable with your AI's name.
- Replace the default `AUTHOR_NAMES` variable with your Teammates' names.
- In C++:
    - Look at `Player.cpp` and `Player.h` files to get a better understanding of your base class.
    - Rename the `PlayerExample` class name to your player files.
    - Add variables and functions to the private class's header file.
        - If you want to add any boards, make sure to add them to the `create_boards()`, `clear_boards()`, and `delete_boards()` functions (or any other board functions you create).
- In Python:
    - Make sure that your python file is an executable (run `chmod 755 your_file.py`).
    - Look at the `Player.py` file to get a better understanding of your base class.
    - Rename the `PlayerExample` class name in your player file.
    - Add values you want to track throughout the match under the `__init__()` function of the player class.
        - If you want to add any boards, make sure to add them to the `create_boards()`, `clear_boards()`, and `delete_boards()` functions (or any other board functions you create).

- Modify the ship placement, shot placement, and other game-logic functions to make a better AI!

### Defined Values
- Ship Directions:
    - Either `HORIZONTAL` or `VERTICAL`.
- Board Values:
    - `WATER` - Only used to clear a board, or check if a spot has been shot at.
    - `SHIP` - Only used to store the location of a ship onto a board.
    - `MISS` - Returned by `shot_return` when `WATER` was shot at. Store it to not shoot there again.
    - `HIT` - Returned by `shot_return` when a `SHIP` was shot at. Store it to shoot by it but not there again.
    - `KILL` - Only used to store where a `SHIP` was killed.
    - Duplicates - Returned by `shot_return` when a shot was made at the same location two or more times. You should prevent your AI from doing duplicate shots.


## Error Codes and potential issues:
---

- Error Codes **1-7**:
    - Run it on linux (TU: lab machine, matt, or a personal install).
    - Run `make clean`, then `make all`.
    - TU: Contact me on the CSE slack @ mattgetgen.
- Error Codes **8-11**:
    - Your AI may have exited early, segfaulted, or is in an infinite loop.
- Error Code **12**: (Invalid Ship)
    - Your AI may have:
        - placed a ship off of the board.
        - placed a ship where an existing one is.
        - placed a ship shorter than the expected length.
- Error Code **13**: (Invalid Shot)
    - Your AI may have placed a shot off of the board.

> TU: If you are still having an issue, contact me on the CSE slack @ mattgetgen


## Debugging an AI:
---

There is a debug mode for the controller that disables message timeouts. This can be accessed by adding a `1` to the controller's arguments.
> **Example:** `./controller 1`

### Debugging with **gdb**:
- Run `gdb -q controller` on the command line.
- In the gdb command line, type these commands:
    - `break main` or `b main`
        - This breakpoint will hit both the main function in controller, and the main function of the player you are debugging.
    - `set follow-fork-mode child`
        - `fork()` is used to create a new process.
        - The controller uses forks to run an AI in a separate process.
        - This will allow gdb to step into the forked process.
    - `set follow-exec-mode new`
        - `execve()` is used to execute a new program.
        - The new process runs this to execute the AI into the process.
        - This will allow gdb to attach to the AI.
    - `run 1` or `r 1`
        - Runs the controller with the debug mode set.
    - `continue` or `c`
        - Continues execution until prompted for runtime questions.
        > **NOTE:** -- The first AI you choose at runtime will be the AI you debug.
- After you choose the AI to play a match, it should stop at the AI's main function where you can:
    - set breakpoints, step through the program, etc.
- This does not work for Python AIs.


## How to Create a new AI in a language of your choice
---

### For a language to be compatible with this project, it must:
- Support JSON and Unix Domain Sockets.
- Be either compiled to an executable, or made executable with Unix permissions.
    - Python usually requires `python3 player_example.py` at the command line to run, but if you make it an executable, and add the `#!/usr/bin/python3` shebang at the top of the file, it can be executed at the command line without using `python3`.
    - A similar workaround for other interpreted languages might need to be found.

### For an AI to be compatible with this project, it must:
- Accept the socket file path as the program argument (exit otherwise).
- Connect to the socket using:
    - `AF_UNIX` as the TYPE.
    - `SOCK_STREAM` as the PROTOCOL.
- Send and receive JSON (converted into c-string) messages over the socket.
- Handle different message types:
    - Create messages to send to the server:
        - `hello`
        - `ship_placed`
        - `shot_taken`
    - Receive and process messages sent from the server:
        - `setup_match`
        - `start_game`
        - `place_ship`
        - `take_shot`
        - `shot_return`
        - `ship_dead`
        - `game_over`
        - `match_over`
    - The details of these messages are in the `project_design.md` file in this repo.
    - If you are really interested in making a new AI, make sure to implement everything in the base `Player` class files. Everything there is required.

