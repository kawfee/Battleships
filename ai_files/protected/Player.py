#!/usr/bin/python3 -m
# -*- coding: utf-8 -*-

from dataclasses import dataclass
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
PLAYER_1_KEY       = "p1"
PLAYER_2_KEY       = "p2"
SHIP_KEY           = "sp"
SHOT_KEY           = "st"
NEXT_SHOT_KEY      = "ns"
GAME_RESULT_KEY    = "gr"
NUM_BOARD_SHOT_KEY = "nb"
NUM_HITS_KEY       = "nh"
NUM_MISSES_KEY     = "nm"
NUM_DUPLICATES_KEY = "nd"
SHIPS_KILLED_KEY   = "sk"

# SERVER MESSAGE TYPES -- used by server to create messages, used by player to parse messages
setup_match    = 2
start_game     = 3
place_ship     = 4
take_shot      = 6
shot_return    = 8
game_over      = 9
match_over     = 10

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
DUPLICATE_HIT  = 34
DUPLICATE_MISS = 35
DUPLICATE_KILL = 36

# GAME RESULTS
WIN  = 87
LOSS = 76
TIE  = 84

@dataclass
class Ship:
    length: int
    row: int
    col: int
    direction: int

    def __init__(self):
        self.row = 0
        self.col = 0
        self.length = 0 
        self.direction = HORIZONTAL
        return

@dataclass
class Shot:
    row: int
    col: int
    value: int

    def __init__(self):
        self.row = 0
        self.col = 0
        self.value = WATER
        return

@dataclass
class GameStats:
    num_board_shot: int
    hits: int
    misses: int
    duplicates: int
    ships_killed: int
    result: int

    def __init__(self):
        self.num_board_shot = 0
        self.hits = 0
        self.misses = 0
        self.duplicates = 0
        self.ships_killed = 0
        self.result = WIN # a little optimistic
        return

@dataclass
class MatchStats:
    total_num_board_shot: int
    total_hits: int
    total_misses: int
    total_duplicates: int
    total_ships_killed: int
    wins: int
    losses: int
    ties: int

    def __init__(self):
        self.total_num_board_shot = 0
        self.total_hits = 0
        self.total_misses = 0
        self.total_duplicates = 0
        self.total_ships_killed = 0
        self.wins = 0
        self.losses = 0
        self.ties = 0
        return

class Player:
    def __init__(self):
        self.sd = None
        self.msg: str = ""
        return
    
    def play_match(self, socket_path: str, ai_name: str, author_names: str):
        player_num: int = 0
        board_size: int = 0
        ship_length: int = 0
        match_stats: MatchStats = MatchStats()

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
        my_key: str = ""
        their_key: str = ""
        if player_num == 1:
            my_key = PLAYER_1_KEY
            their_key = PLAYER_2_KEY
        else:
            my_key = PLAYER_2_KEY
            their_key = PLAYER_1_KEY

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
                ship: Ship = self.choose_ship_place(ship_length)

                # send response
                self.create_ship_placed_msg(ship)
                self.send_msg()

            elif mt == take_shot:
                # choose a place to shoot
                shot: Shot = self.choose_shot()

                # send response
                self.create_shot_taken_msg(shot)
                self.send_msg()

            elif mt == shot_return:
                shot1: Shot = Shot()
                shot2: Shot = Shot()

                shot1.row = int(j[my_key][SHOT_KEY][ROW_KEY])
                shot1.col = int(j[my_key][SHOT_KEY][COL_KEY])
                shot1.value = int(j[my_key][SHOT_KEY][VALUE_KEY])

                shot2.row = int(j[their_key][SHOT_KEY][ROW_KEY])
                shot2.col = int(j[their_key][SHOT_KEY][COL_KEY])
                shot2.value = int(j[their_key][SHOT_KEY][VALUE_KEY])

                next_shot: bool = bool(j[NEXT_SHOT_KEY])

                if player_num == 1:
                    self.handle_shot_return(1, shot1)
                    self.handle_shot_return(2, shot2)
                else:
                    self.handle_shot_return(2, shot1)
                    self.handle_shot_return(1, shot2)

                if SHIP_KEY in j[my_key]:
                    ship: Ship = Ship()
                    ship.row = int(j[my_key][SHIP_KEY][ROW_KEY])
                    ship.col = int(j[my_key][SHIP_KEY][COL_KEY])
                    ship.length = int(j[my_key][SHIP_KEY][LEN_KEY])
                    ship.direction = int(j[my_key][SHIP_KEY][DIR_KEY])

                    self.handle_ship_dead(player_num, ship)
                
                if SHIP_KEY in j[their_key]:
                    ship: Ship = Ship()
                    ship.row = int(j[their_key][SHIP_KEY][ROW_KEY])
                    ship.col = int(j[their_key][SHIP_KEY][COL_KEY])
                    ship.length = int(j[their_key][SHIP_KEY][LEN_KEY])
                    ship.direction = int(j[their_key][SHIP_KEY][DIR_KEY])

                    if player_num == 1:
                        self.handle_ship_dead(2, ship)
                    else:
                        self.handle_ship_dead(1, ship)
                
                if next_shot:
                    # choose a place to shoot
                    shot: Shot = self.choose_shot()

                    # send response
                    self.create_shot_taken_msg(shot)
                    self.send_msg()

            elif mt == game_over:
                game_stats: GameStats = GameStats()
                game_stats.result = int(j[GAME_RESULT_KEY])
                game_stats.num_board_shot = int(j[NUM_BOARD_SHOT_KEY])
                game_stats.hits = int(j[NUM_HITS_KEY])
                game_stats.misses = int(j[NUM_MISSES_KEY])
                game_stats.duplicates = int(j[NUM_DUPLICATES_KEY])
                game_stats.ships_killed = int(j[SHIPS_KILLED_KEY])

                if game_stats.result == WIN:
                    match_stats.wins += 1
                elif game_stats.result == LOSS:
                    match_stats.losses += 1
                elif game_stats.result == TIE:
                    match_stats.ties += 1
                
                match_stats.total_num_board_shot += game_stats.num_board_shot
                match_stats.total_hits += game_stats.hits
                match_stats.total_misses += game_stats.misses
                match_stats.total_duplicates += game_stats.duplicates
                match_stats.total_ships_killed += game_stats.ships_killed

                self.handle_game_over(game_stats)

            elif mt == match_over:
                break
            else:
                break

            self.recv_msg()
            j = json.loads(self.msg)
            mt = int(j[MESSAGE_TYPE_KEY])
            
        self.handle_match_over(match_stats)
        return

    def handle_setup_match(self, board_size: int, player_num: int):
        pass

    def handle_start_game(self):
        pass

    def choose_ship_place(self, ship_len: int) -> Ship:
        pass

    def choose_shot(self) -> Shot:
        pass

    def handle_shot_return(self, player_num: int, shot: Shot):
        pass

    def handle_ship_dead(self, player_num: int, ship: Ship):
        pass

    def handle_game_over(self, game_stats: GameStats):
        pass

    def handle_match_over(self, match_stats: MatchStats):
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


    def create_ship_placed_msg(self, ship: Ship):
        j = {}
        j[MESSAGE_TYPE_KEY] = ship_placed
        j[ROW_KEY] = ship.row
        j[COL_KEY] = ship.col
        j[LEN_KEY] = ship.length
        j[DIR_KEY] = ship.direction
        self.msg = json.dumps(j)
        return


    def create_shot_taken_msg(self, shot: Shot):
        j = {}
        j[MESSAGE_TYPE_KEY] = shot_taken
        j[ROW_KEY] = shot.row
        j[COL_KEY] = shot.col
        self.msg = json.dumps(j)
        return


def print_error(ai_name: str, error: str, line: int):
    print("{} Error: {} (line: {})".format(ai_name, error, line))
    return

def LINE() -> int:
    return _getframe(1).f_lineno

