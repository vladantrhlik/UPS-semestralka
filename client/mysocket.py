import socket
import queue
import time
import threading
import pygame as pg

MSGLEN = 64
MAX_WAIT = 3
PING_INTERVAL = 3

class Socket():
    ip: str
    port: int

    def __init__(self, ip: str, port: int):
        self.msg_queue = queue.Queue()

        self.ip = ip
        self.port = port
        self.connect()

        self.waiting = False
        self.pinging = False
        self.last_ping = time.time()
        self.waiting_from = time.time()
        self.connected = True

        self.thread_i = threading.Thread(target=self.recv_loop, daemon=True)
        self.thread_i.start()

    def connect(self): 
        print("Connecting...", end="")
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((self.ip, self.port))
            self.sock.setblocking(False)
            self.connected = True
            self.pinging = False
            print("succussfull")
        except:
            print("not successfull")

    def send(self, msg):
        totalsent = 0
        if not self.connected: return
        while totalsent < len(msg):
            sent = self.sock.send(msg[totalsent:].encode())
            if sent == 0:
                raise RuntimeError("socket connection broken")
            totalsent = totalsent + sent
            self.waiting = True
            self.waiting_from = time.time()

    def recv_loop(self):
        while True:
            try:
                # Try receiving data from the server
                data = self.sock.recv(1024)
                if data:
                    for i in data.decode('utf-8').split('\n'):
                        if len(i) > 0:
                            self.connected = True
                            if self.pinging and i == "PONG":
                                self.pinging = False
                                self.waiting = False
                            else:
                                self.msg_queue.put(i)  # Put data into the queue for the main thread to process
            except BlockingIOError:
                # No data available, continue
                pass
            except Exception as e:
                #print(f"Network error: {e}")
                self.connected = False
                #break

            # check timeout
            if self.waiting and time.time() - self.waiting_from > MAX_WAIT:
                self.msg_queue.put("Timeout")

                self.connected = False

            # ping every X seconds
            if time.time() - self.last_ping > PING_INTERVAL:
                self.last_ping = time.time()
                if not self.pinging:
                    self.pinging = True
                    self.send("PING\n")
                else:
                    self.connect()

            time.sleep(.01)  # Small delay to avoid high CPU usage

        self.sock.close()       

    def peek_last_msg(self):
        if self.msg_queue.empty():
            return None
        else:
            return self.msg_queue.queue[0]

    def get_last_msg(self):
        if self.msg_queue.empty():
            return None
        else:
            self.waiting = False
            return self.msg_queue.get()
