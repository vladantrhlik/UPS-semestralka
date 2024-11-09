import socket
import queue
import time
import threading

MSGLEN = 64
MAX_WAIT = 2

class Socket():
    def __init__(self, ip: str, port: int):
        self.msg_queue = queue.Queue()
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((ip, port))
        self.sock.setblocking(False)

        self.waiting = False
        self.waiting_from = time.time()

        self.thread_i = threading.Thread(target=self.recv_loop, daemon=True)
        self.thread_i.start()


    def send(self, msg):
        totalsent = 0
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
                            self.msg_queue.put(i)  # Put data into the queue for the main thread to process
            except BlockingIOError:
                # No data available, continue
                pass
            except Exception as e:
                print(f"Network error: {e}")
                break

            if self.waiting and time.time() - self.waiting_from > MAX_WAIT:
                self.waiting = False
                self.msg_queue.put("Timeout")

            time.sleep(.01)  # Small delay to avoid high CPU usage

        self.sock.close()       

    def get_last_msg(self):
        if self.msg_queue.empty():
            return None
        else:
            self.waiting = False
            return self.msg_queue.get()
    '''
    while True:
            chunks = []
            bytes_recd = 0
            while bytes_recd < MSGLEN:
                chunk = self.sock.recv(min(MSGLEN - bytes_recd, 2048))
                if chunk == b'':
                    raise RuntimeError("socket connection broken")
                chunks.append(chunk)
                bytes_recd = bytes_recd + len(chunk)
                # read until \n
                if (chunks[-1][-1] == 10): break
            self.msg_queue.put(b''.join(chunks).decode('ascii')[:-1])
    '''
