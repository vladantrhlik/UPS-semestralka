import pygame as pg
import pygame_gui as pgui

class Scene:
    def __init__(self, user_data):
        self.user_data = user_data
        self.ui_manager = pgui.UIManager(pg.display.get_window_size())
        self.ui_container = pgui.core.UIContainer(pg.Rect(0, 0, *pg.display.get_window_size()), manager=self.ui_manager)

    def update(self, delta_time):
        self.ui_manager.update(delta_time)
        self.ui_container.set_dimensions(pg.display.get_window_size())

    def process_event(self, event):
        self.ui_manager.process_events(event)

    def draw(self, screen):
        self.ui_manager.draw_ui(screen)

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
        print(self.uname_box.get_text())
