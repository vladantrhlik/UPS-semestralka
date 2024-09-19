from game_data import *
from pyray import *

class GameView:
    def __init__(self, game_data: GameData):
        self.game_data = game_data
        self.min_padding = 20
        self.dot_radius = 10

        # set window resizeable
        set_config_flags(FLAG_WINDOW_RESIZABLE)
        # aliasing
        set_config_flags(FLAG_MSAA_4X_HINT)

    def draw(self):
        # screen size
        w = get_screen_width()
        h = get_screen_height()

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

                col = BLANK 
                if val == Player.HIM: col = RED
                if val == Player.ME: col = BLUE

                if y % 2 == 0:
                    # horizontal
                    draw_line(off_x + x * tile, off_y + (y // 2) * tile,
                              off_x + (x+1) * tile, off_y + (y // 2) * tile, col)
                else:
                    # vertical
                    draw_line(off_x + x * tile, off_y + (y // 2) * tile,
                              off_x + x * tile, off_y + (y // 2 + 1) * tile, col)




        # draw dots
        for y in range(self.game_data.h + 1):
            for x in range(self.game_data.w + 1):
                draw_circle(off_x + x * tile, off_y + y * tile, self.dot_radius, BLACK)
