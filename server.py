import socket
from threading import Thread
#from communication import Comm

SERVER_PORT = 7777
CHUNK_SIZE = 1024


def list_command(conn, addr, command):
    print "Received list command from {}".format(addr[0])


def get_command(conn, addr, command):
    print "Received get command from {}".format(addr[0])


def createtracker(conn, addr, command):
    print "Received createtracker command from {}".format(addr[0])


def updatetracker(conn, addr, command):
    print "Received updatetracker command from {}".format(addr[0])


def handle_client(client_conn, client_addr):
    data = client_conn.recv(CHUNK_SIZE)
    if data:
        if data.lower() == "req list" or data.lower() == "<req list>":
            list_command(client_conn, client_addr, data)
        elif data.lower().find("get") != -1:
            get_command(client_conn, client_addr, data)
        elif data.lower().find("createtracker") != -1:
            createtracker(client_conn, client_addr, data)
        elif data.lower().find("updatetracker") != -1:
            updatetracker(client_conn, client_addr, data)
        else:
            print "Server received unrecognized command from {0}: {1}".format(client_addr[0], data)
    client_conn.close()


print "Starting server, hostname = {}".format(socket.gethostname())
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(('', SERVER_PORT))
print "Bound socket to port {}".format(SERVER_PORT)
sock.listen(5)
print "Listening for clients..."
try:
    while True:
        (connection, addr) = sock.accept()
        print "Connection address: {0}".format(addr)
        thread = Thread(target=handle_client, args=(connection, addr))
        thread.start()
except KeyboardInterrupt:
    print
    sock.close()