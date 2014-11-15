import socket

#SERVER_IP = socket.gethostbyname(socket.getfqdn())
#SERVER_IP = socket.gethostname()
SERVER_PORT = 7777
CHUNK_SIZE = 1024

print "Starting server, hostname = {}".format(socket.gethostname())
#print SERVER_IP
sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print "Binding socket to port 7777"
sd.bind(('', SERVER_PORT))
print "Listening for clients..."
sd.listen(1)

(conn, addr) = sd.accept()
print 'Connection address:', addr
#while True:
data = conn.recv(CHUNK_SIZE)
    #if not data: break
print "received data:", data
    #conn.send(data)  # echo
conn.close()
