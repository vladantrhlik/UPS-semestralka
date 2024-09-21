import pygame as pg
import pygame_gui as pgui
from scene import Scene
from game_data import GameData
from game_view import GameView

class GameScene(Scene):
    def __init__(self, user_data):
        super().__init__(user_data)
        self.game_data = GameData(3, 3)
        self.game_view = GameView(self.game_data)

    def draw(self, screen):
        super().draw(screen)
        self.game_view.draw(screen)
