import socket
import copy


SERVER_PORT = 7777
CLIENT_PORT = 8888
CHUNK_SIZE = 1024


class Comm():
    def __init__(self, src_port, dest_addr=None, dest_port=None):
        self.src_port = src_port
        self.dest_addr = dest_addr
        self.dest_port = dest_port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        if self.dest_port is None:
            self.dest_port = self.src_port
            self.sock.bind(('', self.src_port))
            if self.src_port == SERVER_PORT:
                print "Bound socket to port {}".format(SERVER_PORT)
            self.sock.listen(5)
            if self.src_port == SERVER_PORT:
                print "Listening for clients..."
        else:
            self.sock.connect((self.dest_addr, self.dest_port))

    def accept(self):
        (connection, addr) = self.sock.accept()
        self.sock = connection
        self.dest_addr = addr[0]
        return copy.deepcopy(self)

    def send(self, data):
        sent = self.sock.send(data)
        if sent == 0:
            print "Did not send"

    def recv(self):
        data = self.sock.recv(CHUNK_SIZE)
        return data

    def close(self):
        self.sock.close()
