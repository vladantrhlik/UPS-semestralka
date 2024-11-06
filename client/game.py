import pygame as pg
import pygame_gui as pgui
from scene import Scene
from game_data import GameData, Player
from game_view import GameView

class GameScene(Scene):
    def __init__(self, user_data):
        super().__init__(user_data)
        self.game_data = GameData(3, 3)
        self.game_view = GameView(self.game_data)

    def draw(self, screen):
        super().draw(screen)
        self.game_view.draw(screen)

    def update(self, delta_time):
        super().update(delta_time)
        res = self.socket.recv()
        print(res)

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

        mouse_pos[0] -= self.game_view.off_x
        mouse_pos[1] -= self.game_view.off_y
        tile = self.game_view.tile
        if (not 0 <= mouse_pos[0] < tile * self.game_data.w or
            not 0 <= mouse_pos[1] < tile * self.game_data.h):
            self.game_view.preview_coords = [-1, -1]
            return # out of bounds

        coords = self.calc_stick_pos(mouse_pos, tile)
        self.game_view.preview_coords = coords

        if event.type == pg.MOUSEBUTTONDOWN:
            self.game_data.set_stick(*coords, Player.ME)


