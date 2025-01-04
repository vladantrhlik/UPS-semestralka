import pygame as pg
from game_data import GameData, Player
from game_view import GameView
from login import *
from consts import *
from scene_manager import SceneManager
from scene import SceneType
from game import GameScene
from mysocket import Socket
from sys import argv
from user import User

# read ip of server
ip = "localhost"
if len(argv) >= 2:
    ip = argv[1]
else:
    print(f"Missing IP, using '{ip}'")

# init pygame
pg.init()
screen = pg.display.set_mode([500, 500], pg.RESIZABLE)
clock = pg.time.Clock()

# start game with login screen
sm = SceneManager()

user_data = User(Socket(ip, 10000))
sm.set_scene(SceneType.LOGIN, user_data)

# main loop
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
    res = curr_scene.update(delta_time)
    if res != None:
        sm.set_scene(res, curr_scene.user_data)

    # draw current scene
    screen.fill(Col.BACKGROUND)
    curr_scene.draw(screen)

    pg.display.flip()

pg.quit()
