import pygame as pg

pg.font.init()
class BarUtils:
    HEIGHT = 20
    FONT = pg.font.SysFont('Consolas', 14)
    PADDING = 5

    def get_font_n_rect(txt: str, col):
        txt = str(txt)
        g = BarUtils.FONT.render(txt, False, col)
        rect = BarUtils.FONT.size(txt)
        return (g, rect)


class NameChecker:
    MIN_NAME_LEN = 3
    MAX_NAME_LEN = 12

    def is_name_valid(name: str) -> bool:
        if len(name) < 3 or len(name) > 12: return False

        for c in name:
            if (not c.isalnum() or not c.isascii()) and c != "_":
                return False

        return True

class Col:
    PLAYER = (58, 224, 208)
    ENEMY = (219, 55, 99)
    PREVIEW = (253, 255, 122)
    BACKGROUND = [232]*3
    DOT = (0,0,0)
    BAR = [27]*3
    FONT = [255]*3
    CONNECTED = [70, 240, 115]
    DISCONNECTED = [255,0,0]

class Msg:
    OK = "OK"
    ERR = "ERR"
    ERR1 = "ERR1"
    ERR2 = "ERR2"
    ERR3 = "ERR3"
    ERR4 = "ERR4"
    ERR5 = "ERR5"
    TURN = "TURN"
    ON_TURN = "ON_TURN"
    OP_TURN = "OP_TURN"
    OP_LEAVE = "OP_LEAVE"
    OP_ACQ = "OP_ACQ"
    ACQ = "ACQ"
    OP_JOIN = "OP_JOIN"
    SYNC = "SYNC"
    WIN = "WIN"
    LOSE = "LOSE"

    msgs = {
        OK: "OK",
        ERR1: "Invalid",
        ERR2: "Name already exists",
        ERR3: "Not on turn",
        ERR4: "Not in game",
        ERR5: "Already in game",
    }
    
    def txt(msg: str) -> str:
        if msg in Msg.msgs.keys():
            return Msg.msgs[msg]
        else:
            return "Message not found"
