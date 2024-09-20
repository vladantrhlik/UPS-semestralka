from game_data import *
import pygame as pg
from pygame import gfxdraw
from colors import Col

class GameView:
    def __init__(self, game_data: GameData):
        self.game_data = game_data

        self.min_padding = 20
        self.dot_radius = 7
        self.stick_width = 4

        '''
        # set window resizeable
        set_config_flags(FLAG_WINDOW_RESIZABLE)
        # aliasing
        set_config_flags(FLAG_MSAA_4X_HINT)
        '''
    def draw_circle(self, surface, x, y, radius, color):
        gfxdraw.aacircle(surface, x, y, radius, color)
        gfxdraw.filled_circle(surface, x, y, radius, color)

    def draw(self, screen):
        # screen size
        w,h = pg.display.get_window_size()

        padding = max(min(w, h) * 0.05, self.min_padding)

        tile = min(w / self.game_data.w - 2 * padding, 
                        h / self.game_data.h - 2 * padding)
        tile = int(tile)

        off_x = int((w - self.game_data.w * tile) / 2)
        off_y = int((h - self.game_data.w * tile) / 2)

        # draw sticks
        for y in range(len(self.game_data.stick_data)):
            for x in range(len(self.game_data.stick_data[y])):
                val = self.game_data.stick_data[y][x]
                if val == Player.NONE: continue
                col = Col.ENEMY if val == Player.HIM else Col.PLAYER

                if y % 2 == 0:
                    # horizontal
                    pg.draw.line(screen, col,
                                 (off_x + x * tile, off_y + (y // 2) * tile),
                                 (off_x + (x+1) * tile, off_y + (y // 2) * tile),
                                 width = self.stick_width)
                else:
                    # vertical
                    pg.draw.line(screen, col,
                                 (off_x + x * tile, off_y + (y // 2) * tile),
                                 (off_x + x * tile, off_y + (y // 2 + 1) * tile),
                                 width = self.stick_width)

        # draw dots
        for y in range(self.game_data.h + 1):
            for x in range(self.game_data.w + 1):
                # pg.draw.circle(screen, (0,0,0),
                #            (off_x + x * tile, off_y + y * tile), self.dot_radius)
                self.draw_circle(screen, off_x + x * tile, off_y + y * tile, self.dot_radius, Col.DOT)
