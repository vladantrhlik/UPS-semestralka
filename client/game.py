import pygame as pg
import pygame_gui as pgui
from scene import Scene
from game_data import GameData, Player
from game_view import GameView
from consts import Msg

class GameScene(Scene):
    def __init__(self, user_data):
        super().__init__(user_data)
        self.game_data = GameData(3, 3)
        self.game_view = GameView(self.game_data)

        self.turning = False
        self.turn_coords = None
        self.on_turn = False

    def draw(self, screen):
        super().draw(screen)
        self.game_view.draw(screen)

    def update(self, delta_time):
        super().update(delta_time)
        #res = self.socket.recv()
        #print(res)

    def calc_stick_pos(self, local_mouse_pos, tile):
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

    def process_event(self, event):
        super().process_event(event)
        mouse_pos = list(pg.mouse.get_pos())

        res = self.socket.get_last_msg()
        if res != None:
            print(f"received: {res}")
            if self.turning:
                if res == Msg.OK:
                    self.game_data.set_stick(*self.turn_coords, Player.ME)
                    self.turning = False
                else:
                    print("error while turning")
            if res.startswith("TURN"):
                try:
                    coords = [int(i) for i in res.split("|")[1:]]
                    if len(coords) != 2:
                        print("Invalid oponent coords")
                        return
                    self.game_data.set_stick(*coords, Player.HIM)
                except:
                    print("Invalid oponent coords")
            if res == Msg.ON_TURN:
                print("on turn")
                self.on_turn = True
            if res == Msg.OP_TURN:
                print("oponent on turn")
                self.on_turn = False
            if res.startswith(Msg.ACQ) or res.startswith(Msg.OP_ACQ):
                # parse square data
                s = res.split("|")[1:]
                if len(s) % 2 != 0:
                    print("Invalid ACQ square data")
                    return
                print(f"acq data: {s}")
                try:
                    for i in range(len(s) / 2):
                        self.game_data.set_square(int(s[i*2]), int(s[i*2+1]), Player.ME if res.startswith(Msg.ACQ) else Player.HIM)
                except:
                    print("Error while parsing ACQ coords")

        mouse_pos[0] -= self.game_view.off_x
        mouse_pos[1] -= self.game_view.off_y
        tile = self.game_view.tile
        if (not 0 <= mouse_pos[0] < tile * self.game_data.w or
            not 0 <= mouse_pos[1] < tile * self.game_data.h):
            self.game_view.preview_coords = [-1, -1]
            return # out of bounds

        coords = self.calc_stick_pos(mouse_pos, tile)

        if self.on_turn:
            self.game_view.preview_coords = coords
        else:
            self.game_view.preview_coords = [-1, -1]

        if event.type == pg.MOUSEBUTTONDOWN and self.on_turn:
            self.socket.send(f"TURN|{coords[0]}|{coords[1]}\n")
            self.turn_coords = coords
            self.turning = True
            #self.game_data.set_stick(*coords, Player.ME)


