import pygame as pg
from game_data import GameData, Player
from game_view import GameView
from colors import Col


gd = GameData(3, 3)
gd.set_stick(0, 0, Player.ME)
gd.set_stick(1, 1, Player.HIM)
gd.set_stick(2, 1, Player.HIM)
gd.set_stick(3, 1, Player.ME)

gv = GameView(gd)

pg.init()
screen = pg.display.set_mode([500, 500], pg.RESIZABLE)

# Run until the user asks to quit
running = True
while running:
    for event in pg.event.get():
        if event.type == pg.QUIT:
            running = False

    # Fill the background with white
    screen.fill(Col.BACKGROUND)
    gv.draw(screen)
    pg.display.flip()

# Done! Time to quit.
pg.quit()
