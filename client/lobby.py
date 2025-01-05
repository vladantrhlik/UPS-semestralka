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
        self.selected_game = None

        # list of available games
        lobby_rect = pg.Rect(0, -30, 300, 300)
        self.lobby_list = pgui.elements.UISelectionList(relative_rect = lobby_rect, 
                                                        item_list = list(self.lobbies.keys()),
                                                        container = self.ui_container,
                                                        manager = self.ui_manager,
                                                        anchors={'center': 'center'})

        # connect button
        connect_rect = pg.Rect(82.5, 135, 135, 30)
        self.conn_but = pgui.elements.UIButton(relative_rect=connect_rect,
                                            text='Connect', manager=self.ui_manager,
                                            container = self.ui_container,
                                            anchors={'center': 'center'})

        # refresh button
        refresh_rect = pg.Rect(-135, 135, 30, 30)
        self.rfrsh_but = pgui.elements.UIButton(relative_rect=refresh_rect,
                                            text='U', manager=self.ui_manager,
                                            container = self.ui_container,
                                            anchors={'center': 'center'})

        # text input for new game
        game_name_rect = pg.Rect(-52.5, 135, 135, 30)
        self.game_input = pgui.elements.UITextEntryLine(relative_rect=game_name_rect,
                                                        manager=self.ui_manager, container=self.ui_container,
                                                        placeholder_text="new game",
                                                        anchors={'center': 'center'})

        self.fetch()

    def fetch(self):
        '''
        Update all available games
        '''
        if self.connecting: return

        if not self.socket.send("LOAD\n"):
            self.not_connected_err()
            return
        self.fetching = True
        self.rfrsh_but.disable()

    def update(self, delta_time):
        res = super().update(delta_time)
        if res != None: return res

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
                self.user_data.game = self.selected_game
                return SceneType.GAME
            elif res == Msg.ERR6:
                self.error(msg="Max game limit exceeded"); 
            else:
                print(f"error while connecting: {res}")

            self.connecting = False
            self.conn_but.enable()

        if res == self.socket.peek_last_msg():
            print(f"Lobby: unhandled message: {res}")
            self.socket.get_last_msg()

    def process_event(self, event):
        super().process_event(event)
        # connect button press
        if event.type == pgui.UI_BUTTON_PRESSED:
            if event.ui_element == self.conn_but:
                self.connect()
            if event.ui_element == self.rfrsh_but:
                self.fetch()
        # connect when enter is pressed
        if event.type == pg.KEYDOWN and event.key == pg.K_RETURN:
            on_focus = self.ui_manager.get_focus_set()
            if on_focus != None:
                self.connect()

    def connect(self):
        '''
        Create/Connect to selected game
        '''
        if self.fetching: return

        lobby = self.lobby_list.get_single_selection()
        new_game = self.game_input.get_text()

        if NameChecker.is_name_valid(new_game):
            if not self.socket.send(f"CREATE|{new_game}\n"):
                self.not_connected_err()
                return
            self.selected_game = new_game
        elif (lobby != None):
            if not self.socket.send(f"JOIN|{lobby}\n"):
                self.not_connected_err()
                return
            self.selected_game = lobby
        else:
            if len(new_game) > 0:
                self.error(msg="Invalid game name")
            else:
                self.error(msg="No game selected")
            return

        self.conn_but.disable()
        self.connecting = True
