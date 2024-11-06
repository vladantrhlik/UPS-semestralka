import pygame as pg
import pygame_gui as pgui
from scene import Scene
from lobby import LobbyScene
from consts import Msg

class LoginScene(Scene):
    def __init__(self, ud):
        super().__init__(ud)
        # login button
        login_but_rect = pg.Rect(0, 20, 150, 30)
        self.login_but = pgui.elements.UIButton(relative_rect=login_but_rect,
                                            text='Login', manager=self.ui_manager,
                                            container = self.ui_container,
                                            anchors={'center': 'center'})
        # username textbox
        uname_box_rect = pg.Rect(0, -10, 150, 30)
        self.uname_box = pgui.elements.UITextEntryLine(relative_rect=uname_box_rect,
                                            manager=self.ui_manager,
                                            container = self.ui_container,
                                            anchors={'center': 'center'})
        # restrict input characters
        self.uname_box.set_allowed_characters('alpha_numeric')

        self.err_popup = None


    def process_event(self, event):
        super().process_event(event)
        # login button press
        if event.type == pgui.UI_BUTTON_PRESSED:
            if event.ui_element == self.login_but:
                self.login()
        # enter when text box is focused
        if event.type == pg.KEYDOWN and event.key == pg.K_RETURN:
            on_focus = self.ui_manager.get_focus_set()
            if on_focus != None and self.uname_box in on_focus:
                self.login()

    def login(self):
        uname = self.uname_box.get_text()
        msg = "" # err msg

        # validate uname (length between 3 and 32, only alpha numeric)
        if 3 <= len(uname) <= 32:
            self.user_data["uname"] = uname
            self.socket.send(f"LOGIN|{uname}\n")
            res = self.socket.recv()
            if (res == Msg.OK):
                self.sm.set_scene(LobbyScene(self.user_data))
                return
            else:
                msg = Msg.txt(res)
        else:
            msg = "Length of username must be between 3 and 32"

        self.ui_container.disable()
        self.error(msg=msg, title="Login error")
