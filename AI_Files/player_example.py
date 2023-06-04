#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
from protected.Player import *

# Write your AI's name here. Please don't make it more than 64 characters.
AI_NAME = "Player Example Python"

# Write your name(s) here. Please don't make it more than 64 characters.
AUTHOR_NAMES = "Mamthew Gemchin & Goey Jorski"


class PlayerExample(Player):
    def __init__(self):
        # the super() function inherits everything from the parent class.
        super().__init__()
        # This is where you can store any variables you want to track
        # type annotations are nice, but not necessary
        self.board_size: int = None
        self.player_num: int = None
        self.ship_board: list = None
        self.shot_board: list = None
        return


    def handle_setup_match(self, board_size: int, player_num: int):
        # You can add any other start-of-match logic here!
        self.board_size = board_size
        self.player_num = player_num
        self.create_boards()
        return


    def handle_start_game(self):
        # You can add any other start-of-game logic here!
        self.clear_boards()
        return


    def choose_ship_place(self, ship_len: int) -> dict:
        ship: dict = {}
        # store a ship in a dictionary like this
        ship[ROW_KEY] = 0
        ship[COL_KEY] = 0
        ship[LEN_KEY] = ship_len
        ship[DIR_KEY] = HORIZONTAL

        ship_okay = False

        for row in range(self.board_size):
            for col in range(self.board_size-(ship_len-1)):
                if self.ship_board[row][col] == WATER:
                    ship_okay = True
                    for len in range(ship_len):
                        if self.ship_board[row][col+len] != WATER:
                            ship_okay = False
                            break
                    if ship_okay:
                        for len in range(ship_len):
                            self.ship_board[row][col+len] = SHIP
                        ship[ROW_KEY] = row
                        ship[COL_KEY] = col
                        return ship

        return ship


    def choose_shot(self) -> dict:
        shot: dict = {}
        # store a shot in a dictionary like this, and k
        shot[ROW_KEY] = 0
        shot[COL_KEY] = 0

        for r in range(self.board_size):
            for c in range(self.board_size):
                if self.shot_board[r][c] == WATER:
                    shot[ROW_KEY] = r
                    shot[COL_KEY] = c
                    return shot

        return shot


    def handle_shot_return(self, player_num: int, row: int, col: int, value: int):
        # You can add any other shot-returned game logic here!

        if player_num == self.player_num: # your shot was returned, store it
            self.shot_board[row][col] = value
        else:                       # their shot was returned, store it
            self.ship_board[row][col] = value
        return


    def handle_ship_dead(self, player_num: int, row: int, col: int, len: int, dir: int):
        # You can add any other ship-died game logic here!

        # store the ship that was killed
        for i in range(len):
            if player_num == self.player_num: # your ship is dead, store it
                if dir == HORIZONTAL:
                    self.ship_board[row][col+i] = KILL
                else:
                    self.ship_board[row+i][col] = KILL
            else:                       # their ship is dead, store it
                if dir == HORIZONTAL:
                    self.shot_board[row][col+i] = KILL
                else:
                    self.shot_board[row+i][col] = KILL
        return


    def handle_game_over(self):
        # You can add any other game over game logic here!
        return
    

    def handle_match_over(self):
        # You can add any other match over game logic here!
        self.delete_boards()
        return


    def create_boards(self):
        self.ship_board = []
        self.shot_board = []
        for i in range(self.board_size):
            # please add any other boards you want to add here!
            self.ship_board.append([])
            self.shot_board.append([])
            for j in range(self.board_size):
                self.ship_board[i].append(WATER)
                self.shot_board[i].append(WATER)
        return


    def clear_boards(self):
        for i in range(self.board_size):
            # please add any other boards you want to track here!
            for j in range(self.board_size):
                self.ship_board[i][j] = WATER
                self.shot_board[i][j] = WATER
        return


    def delete_boards(self):
        # please add any other boards you want to track here!
        del self.ship_board
        del self.shot_board
        return


def main():
    if len(sys.argv) != 2:
        print_error("Requres socket name!", LINE())
        return
    socket_path = sys.argv[1]

    player = PlayerExample()

    player.play_match(socket_path, AI_NAME, AUTHOR_NAMES)

    del player

    return


main()

