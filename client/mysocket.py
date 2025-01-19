import socket
import select
import queue
import time
import threading
import pygame as pg
from consts import Msg

MSGLEN = 256
MAX_WAIT = 3
PING_INTERVAL = 1
MAX_PING_WAIT = 5
CONN_INTERVAL = 1

class Socket():
    '''
    Client socket scene
    '''
    ip: str
    port: int

    def __init__(self, ip: str, port: int):
        self.msg_queue = queue.Queue()

        self.ip = ip
        self.port = port

        from user import User
        self.user_data: User = None

        self.waiting = False
        self.pinging = False
        self.last_ping = time.time()
        self.last_pong = time.time()
        self.waiting_from = time.time()
        self.last_conn = -1
        self.connected = True
        self.ever_connected = False
        self.reconnecting = False

        self.connect()

        self.thread_i = threading.Thread(target=self.recv_loop, daemon=True)
        self.thread_i.start()

    def connect(self): 
        '''
        Try connecting to sever
        '''
        print("Connecting...", end="")
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((self.ip, self.port))
            self.sock.setblocking(False)
            self.connected = True
            self.pinging = False
            self.last_pong = time.time()
            print("succussfull")

            if self.ever_connected == True:
                uname = self.user_data.uname
                if uname == None: uname = ""

                game = self.user_data.game
                if game == None: game = ""

                print(f"reconnecting ({uname}, {game})")
                self.send(f"RECONNECT|{uname}|{game}\n")
                self.reconnecting = True

            self.ever_connected = True

        except:
            print("not successfull")

    def disconnect(self):
        # self.sock.close()
        self.sock.shutdown(socket.SHUT_RDWR)
        self.connected = False

    def send(self, msg) -> bool:
        '''
        Send message to server
        '''
        if not self.connected:
            return False

        totalsent = 0
        while totalsent < len(msg):
            try:
                sent = self.sock.send(msg[totalsent:].encode())
            except BrokenPipeError as err:
                return False

            if sent == 0:
                raise RuntimeError("socket connection broken")
            totalsent = totalsent + sent
            if msg != "PING\n":
                self.waiting = True
                self.waiting_from = time.time()

        return True

    def recv_loop(self):
        '''
        Main loop for receiving messages (in 2nd thread)
        '''

        while True:
            try:
                # try receiving data from the server
                data = self.sock.recv(1024)
                if data:
                    for i in data.decode('utf-8').split('\n'):
                        print(len(i), i)
                        if len(i) > 0:
                            self.connected = True
                            # validate
                            valid = False
                            for t in Msg.types():
                                if i.startswith(t): 
                                    valid = True

                            if not valid:
                                print("Invalid message")
                                self.disconnect()
                                break
                                
                            if i == "PONG":
                                self.pinging = False
                                self.waiting = False
                                self.last_pong = time.time()
                            elif i == "PING":
                                self.send("PONG\n");
                            else:
                                self.msg_queue.put(i)  # put data into the queue for the main thread to process
                            self.pinging = False
                            self.last_pong = time.time()
            except BlockingIOError:
                # no data available, continue
                pass
            except Exception as e:
                self.connected = False

            # check timeout
            if self.waiting and time.time() - self.waiting_from > MAX_WAIT:
                self.msg_queue.put("Timeout")
                print("timeout err")
                self.connected = False

            # ping every X seconds
            if self.connected and time.time() - self.last_pong > PING_INTERVAL and time.time() - self.last_ping > PING_INTERVAL:
                self.last_ping = time.time()
                self.pinging = True
                #print("ping")
                self.send("PING\n")

            # no response to ping after X seconds
            if self.pinging and time.time() - self.last_pong > MAX_PING_WAIT:
                print(f"last pong before {time.time() - self.last_pong}")
                self.last_pong = time.time()
                self.connected = False

            # reconnect every X seconds if disconnected
            if not self.connected and time.time() - self.last_conn > CONN_INTERVAL:
                self.last_conn = time.time()
                self.connect()

            time.sleep(.01)  # small delay

        self.sock.close()       

    def peek_last_msg(self):
        '''
        Peek last message from message queue
        '''
        if self.msg_queue.empty():
            return None
        else:
            return self.msg_queue.queue[0]

    def get_last_msg(self):
        '''
        Dequeue last message from message queue
        '''
        if self.msg_queue.empty():
            return None
        else:
            self.waiting = False
            return self.msg_queue.get()
