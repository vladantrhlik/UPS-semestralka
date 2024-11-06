import pygame as pg
import pygame_gui as pgui
from scene import Scene
from game import GameScene
from consts import Msg

class LobbyScene(Scene):
    def __init__(self, ud):
        super().__init__(ud)
        # dict of all available lobbies (key=name, val=index)
        self.lobbies = {}
        self.fetch()


        lobby_rect = pg.Rect(0, -30, 300, 300)
        self.lobby_list = pgui.elements.UISelectionList(relative_rect = lobby_rect, 
                                                        item_list = list(self.lobbies.keys()),
                                                        container = self.ui_container,
                                                        manager = self.ui_manager,
                                                        anchors={'center': 'center'})

        connect_rect = pg.Rect(0, 135, 300, 30)
        self.conn_but = pgui.elements.UIButton(relative_rect=connect_rect,
                                            text='Connect', manager=self.ui_manager,
                                            container = self.ui_container,
                                            anchors={'center': 'center'})

    def fetch(self):
        # TODO: fetch all available lobbies
        self.socket.send("LOAD\n");
        res = self.socket.recv().split("|")
        if (res[0] == Msg.OK):
            c = 0
            for i in res[1:]:
                self.lobbies[i] = c
                c += 1

        print(res)


    def process_event(self, event):
        super().process_event(event)
        # connect button press
        if event.type == pgui.UI_BUTTON_PRESSED:
            if event.ui_element == self.conn_but:
                # TODO: only when game is selected
                self.connect()

    def connect(self):
        lobby = self.lobby_list.get_single_selection()
        # nothing selected
        if (lobby == None): return
        self.socket.send(f"JOIN|{lobby}\n")
        res = self.socket.recv()
        if (res == Msg.OK):
            self.sm.set_scene(GameScene(self.user_data))
        else:
            print("Error while joining game")
        # TODO: check if room still exists

