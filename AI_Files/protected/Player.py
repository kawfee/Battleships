#!/usr/bin/python3 -m
# -*- coding: utf-8 -*-

from sys import _getframe
from socket import socket, AF_UNIX, SOCK_STREAM
import json

MAX_MSG_SIZE = 256
MAX_NAME_SIZE = 64

# JSON MESSAGE KEYS -- used by the player and server to create and parse messages
MESSAGE_TYPE_KEY   = "mt"
PLAYER_NUM_KEY     = "pn"
AI_NAME_KEY        = "ai"
AUTHOR_NAMES_KEY   = "au"
BOARD_SIZE_KEY     = "bs"
LEN_KEY            = "l"
ROW_KEY            = "r"
COL_KEY            = "c"
DIR_KEY            = "d"
VALUE_KEY          = "v"
OUTCOME_KEY        = "o" 

# SERVER MESSAGE TYPES -- used by server to create messages, used by player to parse messages
setup_match    = 2
start_game     = 3
place_ship     = 4
take_shot      = 6
shot_return    = 8
ship_dead      = 9
game_over      = 10
match_over     = 11

# PLAYER MESSAGE TYPES -- used by player to create messages, used by server to parse messages
hello          = 1
ship_placed    = 5
shot_taken     = 7

# DIRECTION VALUES
HORIZONTAL = 72      # H in ascii
VERTICAL   = 86      # V in ascii

# BOARD VALUES
WATER          = 126 # ~ in ascii
SHIP           = 83  # S in ascii
HIT            = 88  # X in ascii
MISS           = 42  # M in ascii
KILL           = 75  # K in ascii
DUPLICATE      = 33  # ! in ascii
DUPLICATE_HIT  = 34
DUPLICATE_MISS = 35
DUPLICATE_KILL = 36

class Player:
    def __init__(self):
        self.sd = None
        self.msg: str = ""
        return
    
    def play_match(self, socket_path: str, ai_name: str, author_names: str):
        player_num: int = 0
        board_size: int = 0
        ship_length: int = 0

        self.msg = ""

        self.connect_to_socket(socket_path)

        self.create_hello_msg(ai_name, author_names)

        # send hello message
        self.send_msg()

        # recv setup_match message
        self.recv_msg()

        j = json.loads(self.msg)
        mt: int = int(j[MESSAGE_TYPE_KEY])
        if mt != setup_match:
            print_error(ai_name, "Incorrect setup_match message!", LINE())
            return
        
        board_size = int(j[BOARD_SIZE_KEY])
        player_num = int(j[PLAYER_NUM_KEY])

        self.handle_setup_match(board_size, player_num)

        # get first message
        self.recv_msg()
        j = json.loads(self.msg)
        mt = int(j[MESSAGE_TYPE_KEY])

        while mt != match_over:
            
            if mt == start_game:
                self.handle_start_game()

            elif mt == place_ship:
                # get ship length
                ship_length = int(j[LEN_KEY])

                # choose a place to place a ship
                ship = self.choose_ship_place(ship_length)

                # send response
                self.create_ship_placed_msg(ship)
                self.send_msg()

            elif mt == take_shot:
                # choose a place to shoot
                shot = self.choose_shot()

                # send response
                self.create_shot_taken_msg(shot)
                self.send_msg()

            elif mt == shot_return:
                row: int = int(j[ROW_KEY])
                col: int =  int(j[COL_KEY])
                value: int = int(j[VALUE_KEY])
                num: int = int(j[PLAYER_NUM_KEY])

                self.handle_shot_return(num, row, col, value)

            elif mt == ship_dead:
                row: int = int(j[ROW_KEY])
                col: int = int(j[COL_KEY])
                len: int = int(j[LEN_KEY])
                dir: int = int(j[DIR_KEY])
                num: int = int(j[PLAYER_NUM_KEY])

                self.handle_ship_dead(num, row, col, len, dir)

            elif mt == game_over:
                self.handle_game_over()

            elif mt == match_over:
                break
            else:
                break

            self.recv_msg()
            j = json.loads(self.msg)
            mt = int(j[MESSAGE_TYPE_KEY])
            
        self.handle_match_over()
        return

    def handle_setup_match(self, board_size: int, player_num: int):
        pass

    def handle_start_game(self):
        pass

    def choose_ship_place(self, ship_len: int) -> dict:
        pass

    def choose_shot(self) -> dict:
        pass

    def handle_shot_return(self, player_num: int, row: int, col: int, value: int):
        pass

    def handle_ship_dead(self, player_num: int, row: int, col: int, len: int, dir: int):
        pass

    def handle_game_over(self):
        pass

    def handle_match_over(self):
        pass

    def connect_to_socket(self, socket_path: str):
        self.sd = socket(AF_UNIX, SOCK_STREAM)
        self.sd.connect(socket_path)
        return

    def send_msg(self):
        self.msg = self.msg.encode()
        self.sd.sendall(self.msg)
        return

    def recv_msg(self):
        self.msg = self.sd.recv(MAX_MSG_SIZE)
        self.msg = self.msg.decode()
        self.msg = self.msg.rstrip("\x00")
        return

    def create_hello_msg(self, ai_name: str, author_names: str):
        j = {}
        j[MESSAGE_TYPE_KEY] = hello
        j[AI_NAME_KEY]      = ai_name
        j[AUTHOR_NAMES_KEY] = author_names
        self.msg = json.dumps(j)
        return


    def create_ship_placed_msg(self, ship: dict):
        ship[MESSAGE_TYPE_KEY] = ship_placed
        self.msg = json.dumps(ship)
        return


    def create_shot_taken_msg(self, shot: dict):
        shot[MESSAGE_TYPE_KEY] = shot_taken
        self.msg = json.dumps(shot)
        return


def print_error(ai_name: str, error: str, line: int):
    print("{} Error: {} (line: {})".format(ai_name, error, line))
    return

def LINE() -> int:
    return _getframe(1).f_lineno