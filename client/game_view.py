from game_data import *
import pygame as pg
from pygame import gfxdraw
from consts import *

class GameView:
    def __init__(self, game_data: GameData):
        self.game_data = game_data

        self.min_padding = 20
        self.dot_radius = 7
        self.stick_width = 4

        self.tile = -1
        self.off_x, self.off_y = 0, 0
        self.preview_coords = [-1, -1]

    def draw_circle(self, surface, x, y, radius, color):
        gfxdraw.aacircle(surface, x, y, radius, color)
        gfxdraw.filled_circle(surface, x, y, radius, color)

    def draw(self, screen):
        # screen size
        w,h = pg.display.get_window_size()

        padding = max(min(w, h) * 0.05, self.min_padding)

        tile = min(w / self.game_data.w - 2 * padding, 
                        h / self.game_data.h - 2 * padding)
        self.tile = int(tile)

        self.off_x = int((w - self.game_data.w * self.tile) / 2)
        self.off_y = int((h - self.game_data.h * self.tile) / 2)

        # draw sticks
        for y in range(len(self.game_data.stick_data)):
            for x in range(len(self.game_data.stick_data[y])):
                val = self.game_data.stick_data[y][x]
                preview = x == self.preview_coords[0] and y == self.preview_coords[1]

                if val == Player.NONE and not preview: continue

                if val == Player.NONE: col = (255, 255, 0)
                elif val == Player.HIM: col = Col.ENEMY
                else: col = Col.PLAYER

                if y % 2 == 0:
                    # horizontal
                    pg.draw.line(screen, col,
                                 (self.off_x + x * self.tile, self.off_y + (y // 2) * self.tile),
                                 (self.off_x + (x+1) * self.tile, self.off_y + (y // 2) * self.tile),
                                 width = self.stick_width)
                else:
                    # vertical
                    pg.draw.line(screen, col,
                                 (self.off_x + x * self.tile, self.off_y + (y // 2) * self.tile),
                                 (self.off_x + x * self.tile, self.off_y + (y // 2 + 1) * self.tile),
                                 width = self.stick_width)

        # draw dots
        for y in range(self.game_data.h + 1):
            for x in range(self.game_data.w + 1):
                radius = self.dot_radius
                # pg.draw.circle(screen, (0,0,0),
                #            (self.off_x + x * self.tile, self.off_y + y * self.tile), self.dot_radius)
                self.draw_circle(screen, self.off_x + x * self.tile, self.off_y + y * self.tile, radius, Col.DOT)
