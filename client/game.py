import pygame as pg
import pygame_gui as pgui
from game_data import GameData, Player
from game_view import GameView
from consts import Msg, BarUtils
from scene import Scene, SceneType

class GameScene(Scene):
    def __init__(self, user_data):
        super().__init__(user_data)
        self.game_data = GameData(user_data, 3, 3)
        self.game_view = GameView(self.game_data)

        self.turning = False
        self.syncing = False
        self.leaving = False

        self.turn_coords = None
        self.user_data["on_turn"] = False
        self.user_data["in_game"] = True
    
        self.start_sync()

        # leave button
        login_but_rect = pg.Rect(5, BarUtils.HEIGHT + 5, 100, 30)
        self.leave_but = pgui.elements.UIButton(relative_rect=login_but_rect,
                                            text='Leave', manager=self.ui_manager,
                                            container = self.ui_container,
                                            anchors={'left': 'top'})

    def draw(self, screen):
        super().draw(screen)
        self.game_view.draw(screen)

    def update(self, delta_time):
        super().update(delta_time)

        # read potential message from msg queue
        res = self.socket.peek_last_msg()
        if res != None:
            print(f"received: {res}")
            # player on turn
            if res == Msg.ON_TURN:
                print("on turn")
                self.user_data["on_turn"] = True
                res = self.socket.get_last_msg()
            # oponent on turn
            elif res == Msg.OP_TURN:
                print("oponent on turn")
                self.user_data["on_turn"] = False
                res = self.socket.get_last_msg()
            # square acquirement
            elif res.startswith(Msg.ACQ) or res.startswith(Msg.OP_ACQ):
                # parse square data
                s = res.split("|")[1:]
                if len(s) % 2 != 0:
                    print("Invalid ACQ square data")
                else:
                    try:
                        s = [int(i) for i in s]
                        for i in range(len(s) // 2):
                            x, y = s[i*2], s[i*2 + 1]
                            self.game_data.set_square(x, y, Player.ME if res.startswith(Msg.ACQ) else Player.HIM)
                    except:
                        print(f"Error while parsing ACQ coords")
                res = self.socket.get_last_msg()
            # oponent joined the game
            elif res.startswith(Msg.OP_JOIN):
                self.user_data["oponent"] = res.split("|")[1]
                res = self.socket.get_last_msg()
            # oponent left the game
            elif res.startswith(Msg.OP_LEAVE):
                self.user_data["oponent"] += " (left)"
                res = self.socket.get_last_msg()
            # game end -> win / lose
            elif res in [Msg.WIN, Msg.LOSE]:
                self.user_data["on_turn"] = False
                self.user_data["in_game"] = False
                self.user_data["last_game_win"] = True if res == Msg.WIN else False
                res = self.socket.get_last_msg()

            # handle turning response
            elif self.turning:
                if res == Msg.OK:
                    self.game_data.set_stick(*self.turn_coords, Player.ME)
                    self.turning = False
                else:
                    print(f"game error: {res}")
                res = self.socket.get_last_msg()
                return
            # handle syncing response
            elif self.syncing:
                if res.startswith(Msg.OK):
                    self.syncing = False
                    self.sync(res)
                else:
                    print(f"sync error: {res}")
                self.socket.get_last_msg()
            # handle leaving response
            elif self.leaving:
                self.socket.get_last_msg()
                if res.startswith(Msg.OK):
                    self.leaving = False
                    #self.sm.set_scene(LobbyScene(self.user_data))
                    #self.sm.set_scene(SceneType.LOBBY, self.user_data)
                    return SceneType.LOBBY
                else:
                    print(f"leave error: {res}")

            # handle incoming turn messages
            elif res.startswith(Msg.TURN):
                try:
                    coords = [int(i) for i in res.split("|")[1:]]
                    if len(coords) != 2:
                        print("Invalid oponent coords")
                    else:
                        self.game_data.set_stick(*coords, Player.HIM)
                except:
                    print("Invalid oponent coords")
                res = self.socket.get_last_msg()
            # info about unhandled message
            if res == self.socket.peek_last_msg():
                print(f"Unhandled message: {res}")
                self.socket.get_last_msg()

    def calc_stick_pos(self, local_mouse_pos, tile) -> list[int]:
        '''
        Calculate stick (connection) position between two dots from mouse position
        '''
        # get square coords
        sq = [i//tile for i in local_mouse_pos]
        # get local square coords [0-1, 0-1]
        lsqx, lsqy = [(i % tile) / tile for i in local_mouse_pos]
        coords = [-1, -1]

        if lsqx > lsqy:
            # above diagonal
            if 1 - lsqx > lsqy:
                # top
                coords = [sq[0], (sq[1])*2]
            else:
                # right
                coords = [sq[0] + 1, (sq[1]*2) + 1]
        else:
            # under diagonal
            if 1 - lsqx > lsqy:
                # left
                coords = [sq[0], (sq[1]*2) + 1]
            else:
                # bottom
                coords = [sq[0], (sq[1]+1)*2]

        return coords

    def start_sync(self):
        '''
        Start synchronization of game data with server
        '''
        if self.turning: return
        self.socket.send(f"SYNC\n")
        self.syncing = True

    def sync(self, data: str):
        '''
        Synchronize game data with server
        '''
        print(f"syncing to: {data}")

        data_conv = {
                0: Player.NONE,
                1: Player.ME,
                2: Player.HIM
        }

        try:
            data = data.split("|")[1:]
            w, h = [int(i) for i in data[:2]]

            # stick data
            sticks = [int(i) for i in list(data[2])]
            pos = 0
            for y in range(h*2+1):
                for x in range(w + y%2):
                    self.game_data.set_stick(x, y, data_conv[sticks[pos]])
                    pos += 1

            # square data
            squares = [int(i) for i in list(data[3])]
            pos = 0
            for y in range(h):
                for x in range(w):
                    self.game_data.set_square(x, y, data_conv[squares[pos]])
                    pos += 1
        except:
            print("error while parsing sync data")

    def leave(self):
        '''
        Start leaving game
        '''
        self.leaving = True
        if not self.socket.send("LEAVE\n"):
            self.not_connected_err()
            return
        print("leaving")

    def process_event(self, event):
        super().process_event(event)

        # leave button press
        if event.type == pgui.UI_BUTTON_PRESSED:
            if event.ui_element == self.leave_but:
                self.leave()

        # handle mouse
        mouse_pos = list(pg.mouse.get_pos())

        mouse_pos[0] -= self.game_view.off_x
        mouse_pos[1] -= self.game_view.off_y
        tile = self.game_view.tile
        if (not 0 <= mouse_pos[0] < tile * self.game_data.w or
            not 0 <= mouse_pos[1] < tile * self.game_data.h):
            self.game_view.preview_coords = [-1, -1]
            return # out of bounds

        coords = self.calc_stick_pos(mouse_pos, tile)

        if self.user_data["on_turn"]:
            self.game_view.preview_coords = coords
        else:
            self.game_view.preview_coords = [-1, -1]

        if event.type == pg.MOUSEBUTTONDOWN and self.user_data["on_turn"]:
            if not self.socket.send(f"TURN|{coords[0]}|{coords[1]}\n"):
                self.not_connected_err()
                return
            self.turn_coords = coords
            self.turning = True
            #self.game_data.set_stick(*coords, Player.ME)

