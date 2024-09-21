import pygame as pg
import pygame_gui as pgui

class Scene:
    def __init__(self, user_data):
        self.user_data = user_data
        self.ui_manager = pgui.UIManager(pg.display.get_window_size())
        self.ui_container = pgui.core.UIContainer(pg.Rect(0, 0, *pg.display.get_window_size()), manager=self.ui_manager)
        self.sm: SceneManager = None
        self.err_popup = None

    def update(self, delta_time):
        self.ui_manager.update(delta_time)
        self.ui_container.set_dimensions(pg.display.get_window_size())
        self.ui_manager.set_window_resolution(pg.display.get_window_size())

    def process_event(self, event):
        self.ui_manager.process_events(event)
        # error popup close -> enable ui
        if event.type == pgui.UI_WINDOW_CLOSE:
            if event.ui_element == self.err_popup:
                self.ui_container.enable()

    def draw(self, screen):
        self.ui_manager.draw_ui(screen)

    def error(self, title="Error", msg="Message"):
        self.ui_container.disable()
        popup_rect = pg.Rect(0,0,300, 150)
        self.err_popup = pgui.windows.UIMessageWindow(rect= popup_rect,
                                                      html_message = msg,
                                                      window_title = title,
                                                      manager = self.ui_manager)
        dim = self.ui_container.get_relative_rect().size
        self.err_popup.set_position( (dim[0]/2 - 150, 10))

class SceneManager:
    def __init__(self):
        self.scene = None

    def set_scene(self, scene: Scene):
        self.scene = scene
        self.scene.sm = self
