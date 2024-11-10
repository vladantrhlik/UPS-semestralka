
class Col:
    PLAYER = (0,0,255)
    ENEMY = (255,0,0)
    BACKGROUND = [232]*3
    DOT = (0,0,0)

class Msg:
    OK = "OK"
    ERR1 = "ERR1"
    ERR2 = "ERR2"
    ERR3 = "ERR3"
    ERR4 = "ERR4"
    ERR5 = "ERR5"
    ON_TURN = "ON_TURN"
    OP_TURN = "OP_TURN"
    OP_ACQ = "OP_ACQ"
    ACQ = "ACQ"
    OP_JOIN = "OP_JOIN"

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
