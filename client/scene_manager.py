from login import LoginScene
from lobby import LobbyScene
from game import GameScene
from scene import SceneType

class SceneManager:
    def __init__(self):
        self.scene = None

    def set_scene(self, scene: int, userdata):
        if scene == SceneType.LOBBY:
            self.scene = LobbyScene(userdata)
        elif scene == SceneType.LOGIN:
            self.scene = LoginScene(userdata)
        elif scene == SceneType.GAME:
            self.scene = GameScene(userdata)
        else:
            print("SceneManager: Invalid scene index")

        self.scene.sm = self
