import pygame as pg
import pygame_gui as pgui
from consts import *

class SceneType:
    LOBBY = 0
    LOGIN = 1
    GAME = 2

class Scene:
    def __init__(self, user_data):
        self.user_data = user_data
        self.socket = user_data["socket"]
        self.ui_manager = pgui.UIManager(pg.display.get_window_size())
        self.ui_container = pgui.core.UIContainer(pg.Rect(0, 0, *pg.display.get_window_size()), manager=self.ui_manager)
        self.sm: SceneManager = None
        self.err_popup = None
        self.next_scene: Scene = self

    def update(self, delta_time) -> int:
        self.ui_manager.update(delta_time)
        self.ui_container.set_dimensions(pg.display.get_window_size())
        self.ui_manager.set_window_resolution(pg.display.get_window_size())

    def process_event(self, event):
        self.ui_manager.process_events(event)
        # error popup close -> enable ui
        if event.type == pgui.UI_WINDOW_CLOSE:
            if event.ui_element == self.err_popup:
                self.ui_container.enable()
                self.err_popup = None

    def draw(self, screen):
        self.ui_manager.draw_ui(screen)
        # draw top bar
        w, h = pg.display.get_window_size()
        pg.draw.rect(screen, [0]*3, pg.Rect(0,0,w,BarUtils.HEIGHT))
        # server ip
        ip, ip_rect = BarUtils.get_font_n_rect(self.socket.ip, [0]*3)
        y_off = (BarUtils.HEIGHT - ip_rect[1])/2
        pg.draw.rect(screen, Col.CONNECTED if self.socket.connected else Col.DISCONNECTED, pg.Rect(0,0,ip_rect[0]+2*BarUtils.PADDING, BarUtils.HEIGHT))
        screen.blit(ip, (BarUtils.PADDING, y_off))
        # login info
        uname = "not logged"
        if "uname" in self.user_data: uname = f"login: {self.user_data['uname']}"
        login, login_rect = BarUtils.get_font_n_rect(uname, Col.FONT)
        screen.blit(login, (w - BarUtils.PADDING - login_rect[0], y_off))

    def not_connected_err(self):
        msg = "Not connected to server"
        self.ui_container.disable()
        self.error(msg=msg, title="Login error")

    def error(self, title="Error", msg="Message"):
        if self.err_popup != None: return
        self.ui_container.disable()
        popup_rect = pg.Rect(0,0,300, 150)
        self.err_popup = pgui.windows.UIMessageWindow(rect= popup_rect,
                                                      html_message = msg,
                                                      window_title = title,
                                                      manager = self.ui_manager)
        dim = self.ui_container.get_relative_rect().size
        self.err_popup.set_position( (dim[0]/2 - 150, 10))


