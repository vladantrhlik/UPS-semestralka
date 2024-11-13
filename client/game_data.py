from enum import Enum

class Player(Enum):
    NONE = 0
    ME = 1
    HIM = 2

class GameData:
    def __init__(self, user_data, width: int, height: int):
        self.w = width
        self.h = height
        self.user_data = user_data

        # even ... horizontal (width)
        # odd ... vertical (width + 1)
        # (height * 2) + 1 rows
        self.stick_data = [[Player.NONE for x in range(width + y % 2)] for y in range(height * 2 + 1)]
        # width * height matrix
        self.square_data = [[Player.NONE for i in range(width)] for j in range(height)]
        self.score = {
                Player.ME: 0,
                Player.HIM: 0
        }

    def set_square(self, x: int, y: int, val: Player):
        # validate coordinates
        if x < 0 or x >= self.w or y < 0 or y >= self.h:
            print(f"error: invalid square coordinates [{x}, {y}]")
            return
        self.square_data[y][x] = val;
        if val != Player.NONE: self.score[val] += 1

    def set_stick(self, x: int, y: int, val: Player):
        # validate coordinates
        if y < 0 or y >= len(self.stick_data) or x < 0 or x >= len(self.stick_data[y]):
            print(f"error: invalid stick coordinates [{x}, {y}]")
            return
        self.stick_data[y][x] = val
