import pygame as pg
from game_data import GameData, Player
from game_view import GameView
from colors import Col
from login import *

pg.init()
screen = pg.display.set_mode([500, 500], pg.RESIZABLE)
clock = pg.time.Clock()

# start game with login screen
curr_scene: Scene = LoginScene({})

running = True
while running:
    delta_time = clock.tick(60)/1000.0

    # process all events
    for event in pg.event.get():
        if event.type == pg.QUIT:
            running = False

        curr_scene.process_event(event)

    # update current scene
    curr_scene.update(delta_time)

    # draw current scene
    screen.fill(Col.BACKGROUND)
    curr_scene.draw(screen)

    pg.display.flip()

pg.quit()
