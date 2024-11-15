import pygame as pg
import pygame_gui as pgui
from scene import Scene, SceneType
from consts import Msg
from consts import NameChecker

class LobbyScene(Scene):
    def __init__(self, ud):
        super().__init__(ud)
        # dict of all available lobbies (key=name, val=index)
        self.lobbies = {}
        self.fetching = False
        self.connecting = False

        lobby_rect = pg.Rect(0, -30, 300, 300)
        self.lobby_list = pgui.elements.UISelectionList(relative_rect = lobby_rect, 
                                                        item_list = list(self.lobbies.keys()),
                                                        container = self.ui_container,
                                                        manager = self.ui_manager,
                                                        anchors={'center': 'center'})

        connect_rect = pg.Rect(82.5, 135, 135, 30)
        self.conn_but = pgui.elements.UIButton(relative_rect=connect_rect,
                                            text='Connect', manager=self.ui_manager,
                                            container = self.ui_container,
                                            anchors={'center': 'center'})

        refresh_rect = pg.Rect(-135, 135, 30, 30)
        self.rfrsh_but = pgui.elements.UIButton(relative_rect=refresh_rect,
                                            text='U', manager=self.ui_manager,
                                            container = self.ui_container,
                                            anchors={'center': 'center'})

        game_name_rect = pg.Rect(-52.5, 135, 135, 30)
        self.game_input = pgui.elements.UITextEntryLine(relative_rect=game_name_rect,
                                                        manager=self.ui_manager, container=self.ui_container,
                                                        placeholder_text="new game",
                                                        anchors={'center': 'center'})

        self.fetch()

    def fetch(self):
        if self.connecting: return

        self.socket.send("LOAD\n");
        self.fetching = True
        self.rfrsh_but.disable()

    def update(self, delta_time):
        super().update(delta_time)
        # handle fetching response
        res = self.socket.peek_last_msg()
        if res == None: return

        print(f"lobby msg: {res}")
        # handle fetching response
        if self.fetching:
            res = res.split("|")
            if (res[0] == Msg.OK):
                c = 0
                for i in res[1:]:
                    self.lobbies[i] = c
                    c += 1

                self.lobby_list.set_item_list(res[1:])
                self.rfrsh_but.enable()
            else:
                print(f"error while fetching: {res}")

            self.rfrsh_but.enable()
            self.fetching = False
            self.socket.get_last_msg()

        # handle connecting response
        if self.connecting:
            self.socket.get_last_msg()
            if (res == Msg.OK):
                #self.sm.set_scene(GameScene(self.user_data))
                #self.sm.set_scene(SceneType.GAME, self.user_data)
                return SceneType.GAME
            else:
                print(f"error while connecting: {res}")

            self.connecting = False
            self.conn_but.enable()

    def process_event(self, event):
        super().process_event(event)
        # connect button press
        if event.type == pgui.UI_BUTTON_PRESSED:
            if event.ui_element == self.conn_but:
                # TODO: only when game is selected
                self.connect()
            if event.ui_element == self.rfrsh_but:
                self.fetch()
        # connect when enter is pressed
        if event.type == pg.KEYDOWN and event.key == pg.K_RETURN:
            on_focus = self.ui_manager.get_focus_set()
            if on_focus != None:
                self.connect()

    def connect(self):
        if self.fetching: return

        lobby = self.lobby_list.get_single_selection()
        new_game = self.game_input.get_text()

        if NameChecker.is_name_valid(new_game):
            self.socket.send(f"CREATE|{new_game}\n")
        elif (lobby != None):
            self.socket.send(f"JOIN|{lobby}\n")
        else:
            if len(new_game) > 0:
                self.error(msg="Invalid game name")
            else:
                self.error(msg="No game selected")
            return

        self.conn_but.disable()
        self.connecting = True
