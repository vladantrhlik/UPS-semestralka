from mysocket import Socket

class User:
    def __init__(self, socket: Socket):
        self.socket = socket
        self.uname = None
        self.oponent = None
        self.game = None
        self.on_turn = False
        self.in_game = False
        self.last_game_win = False
