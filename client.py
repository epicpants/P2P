import socket
import sys

SERVER_PORT = 7777
CHUNK_SIZE = 1024
CLIENT_IP = socket.gethostname()

if len(sys.argv) != 2:
    print "Incorrect usage. Correct usage = python client.py <server_address>"
    exit(1)
server_address = sys.argv[1]

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print "Connecting to server..."
sock.connect((server_address, SERVER_PORT))
print "Connected"
data = raw_input("Enter client command:\n")
sent = sock.send(data)
if sent == 0:
    print "Did not send"

count = 0
try:
    while True:
        received = sock.recv(CHUNK_SIZE)
        if not received:
            break
        print received
        count += 1
except KeyboardInterrupt:
    print
sock.close()
print count