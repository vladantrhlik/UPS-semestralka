import asyncio
from time import sleep
from random import randint

async def rect():
    sleep(randint(0,5))
    return "OK"


async def login():
    print("loggin in...")
    if (await rect() == "OK"):
        game()
    else:
        print("failed to login")

def game():
    print("game")

asyncio.run(login())
print("uwu")
