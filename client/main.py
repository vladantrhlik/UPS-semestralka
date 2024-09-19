from pyray import *
from game_data import GameData, Player
from game_view import GameView


gd = GameData(3, 3)
gd.set_stick(0, 0, Player.ME)
gd.set_stick(1, 1, Player.HIM)
gd.set_stick(2, 1, Player.HIM)
gd.set_stick(3, 1, Player.ME)

gv = GameView(gd)


init_window(600, 600, "Dots and Boxes")

while not window_should_close():
    begin_drawing()
    
    clear_background(WHITE)
    gv.draw()

    end_drawing()
close_window()

