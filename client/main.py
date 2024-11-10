import pygame as pg
from game_data import GameData, Player
from game_view import GameView
from login import *
from consts import *
from scene import SceneManager
from game import GameScene
from mysocket import Socket

pg.init()
pg.font.init() 
screen = pg.display.set_mode([500, 500], pg.RESIZABLE)
clock = pg.time.Clock()

# start game with login screen
sm = SceneManager()

user_data = {"socket": Socket("localhost", 10000),
             "oponent": None,
             "on_turn": False}
sm.set_scene(LoginScene(user_data))
#sm.set_scene(GameScene(user_data))

running = True
while running:
    curr_scene = sm.scene
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
