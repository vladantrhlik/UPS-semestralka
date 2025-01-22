from mysocket import Socket

class User:
    '''
    Client data
    '''
    def __init__(self, socket: Socket):
        self.socket = socket
        self.socket.user_data = self
        self.uname = None
        self.oponent = None
        self.game = None
        self.on_turn = False
        self.in_game = False
        self.last_game_win = False
