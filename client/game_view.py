from game_data import *
import pygame as pg
from pygame import gfxdraw
import pygame_gui as pgui
from consts import *

class GameView:
    def __init__(self, game_data: GameData):
        self.game_data = game_data

        self.min_padding = 20
        self.dot_radius = 7
        self.stick_width = 4

        self.bar_height = 20
        self.bar_x_pad = 5

        self.tile = -1
        self.off_x, self.off_y = 0, 0
        self.preview_coords = [-1, -1]

        self.font = pg.font.SysFont('Consolas', 14)

    def draw_circle(self, surface, x, y, radius, color):
        gfxdraw.aacircle(surface, x, y, radius, color)
        gfxdraw.filled_circle(surface, x, y, radius, color)

    def get_font_n_rect(self, txt: str, col):
        txt = str(txt)
        g = self.font.render(txt, False, col)
        rect = self.font.size(txt)
        return (g, rect)

    def draw(self, screen):
        # screen size
        w,h = pg.display.get_window_size()

        padding = max(min(w, h) * 0.05, self.min_padding)

        tile = min(w / self.game_data.w - 2 * padding, 
                        h / self.game_data.h - 2 * padding)
        self.tile = int(tile)

        self.off_x = int((w - self.game_data.w * self.tile) / 2)
        self.off_y = int((h - self.game_data.h * self.tile) / 2)

        # draw squares
        for y in range(self.game_data.h):
            for x in range(self.game_data.w):
                val = self.game_data.square_data[y][x]
                if val == Player.NONE: continue
                col = Col.ENEMY if val == Player.HIM else Col.PLAYER
                col = [min(255, i + 128) for i in col]
                pg.draw.rect(screen, col,
                             pg.Rect(self.off_x + x*tile, self.off_y + y*tile, tile, tile))

        # draw sticks
        for y in range(len(self.game_data.stick_data)):
            for x in range(len(self.game_data.stick_data[y])):
                val = self.game_data.stick_data[y][x]
                preview = x == self.preview_coords[0] and y == self.preview_coords[1]

                if val == Player.NONE and not preview: continue

                if val == Player.NONE: col = Col.PREVIEW
                elif val == Player.HIM: col = Col.ENEMY
                else: col = Col.PLAYER

                    # horizontal
                if y % 2 == 0:
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

        '''
        # print userdata
        data = "\n".join([f"{i}: {self.game_data.user_data[i]}" for i in self.game_data.user_data.keys()])
        text_surface = self.font.render(data, False, (0, 0, 0))
        screen.blit(text_surface, (0,0))
        '''

        # bottom bar
        my_bg, other_bg = Col.BAR, Col.BAR
        my_font, other_font = Col.PLAYER, Col.ENEMY
        is_op = self.game_data.user_data["oponent"] != None

        if self.game_data.user_data["on_turn"]: 
            my_bg = Col.PLAYER
            my_font = Col.FONT
        elif is_op: 
            other_bg = Col.ENEMY
            other_font = Col.FONT
        # bg rects
        pg.draw.rect(screen, my_bg, pg.Rect(0, h - self.bar_height, w / 2, self.bar_height))
        pg.draw.rect(screen, other_bg, pg.Rect(w / 2, h - self.bar_height, w / 2, self.bar_height))
        # nicknames
        my_nick, my_nick_rect = self.get_font_n_rect(self.game_data.user_data["uname"], my_font)
        other_nick, other_nick_rect = self.get_font_n_rect(self.game_data.user_data["oponent"] if is_op else "", other_font)
        # render to screen
        y_off = (self.bar_height - my_nick_rect[1]) / 2
        screen.blit(my_nick, (self.bar_x_pad, h - self.bar_height + y_off))
        screen.blit(other_nick, (w - self.bar_x_pad - other_nick_rect[0], h - self.bar_height + y_off))
        # scores
        my_score, my_score_rect = self.get_font_n_rect(self.game_data.score[Player.ME], my_font)
        other_score, other_score_rect = self.get_font_n_rect(self.game_data.score[Player.HIM], other_font)
        screen.blit(my_score, (w / 2 - self.bar_x_pad - my_score_rect[0], h - self.bar_height + y_off))
        screen.blit(other_score, (w / 2 + self.bar_x_pad, h - self.bar_height + y_off))



