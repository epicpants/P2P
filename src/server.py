## @package server Implementation of file sharing server

import hashlib
import socket
import os
import re
import sys
from threading import Thread
import tracker_parser

##Port that the file sharing server listens on
SERVER_PORT = 7777 
##Maximum size of data segments to send in bytes
CHUNK_SIZE = 1024
##Default directory to search for tracker files
SERVER_DIR = '.'


## Obtains a list of names of the tracker files.
# @return List of names of tracker files
def get_tracker_files():
    dir = os.listdir(SERVER_DIR)
    tracker_files = [tracker for tracker in dir if os.path.isfile(tracker) and tracker.find('.track') != -1]
    return tracker_files


## Handles the server-side communication with the client for the LIST command.
# @param conn Socket object connected to client
# @param addr Address of the remote client connected to the server
# @param command LIST command submitted by the client
def list_command(conn, addr, command):
    print "Received list command from {}".format(addr[0])
    tracker_files = get_tracker_files()
    num_files = len(tracker_files)
    conn.sendall("REP LIST {0}".format(str(num_files)))
    count = 1

    # Obtains filename, filesize, and md5 for each tracker file to send to client
    for tracker in tracker_files:
        tracker_file = tracker_parser.TrackerFile()
        if not tracker_file.parseTrackerFile(tracker):
            file_size = tracker_file.get_file_size()
            md5 = tracker_file.get_md5()
            file_name = tracker_file.get_filename()
            data = "{0} {1} {2} {3}".format(str(count), file_name, str(file_size), md5)
            conn.sendall(data)
            print "+=+=+=data {0}".format(data)
            count += 1
        else:
            print 'Error parsing tracker file'
            conn.sendall('ERROR')

    conn.sendall("REP LIST END")
    conn.close()


## Handles the server-side communication with the client for the GET command.
# @param conn Socket object connected to client
# @param addr Address of the remote client connected to the server
# @param command GET command submitted by the client
def get_command(conn, addr, command):
    print "Received get command from {}".format(addr[0])
    tracker_filename = ''
    match = re.match(r"get\s(.+)", command.lower())

    # Open file or send error message to client
    if match:
        tracker_filename = match.group(1)
        if os.path.isfile(tracker_filename):
            conn.send('REP GET BEGIN')
        else:
            conn.send('ERROR FILE DNE')
    else:
        conn.send('ERROR NO FILENAME')

    #Calculate MD5 Hash of tracker file
    tracker_file = open(tracker_filename, 'rb')
    block_size=2**20
    md5 = hashlib.md5()
    while True:
        data = tracker_file.read(CHUNK_SIZE)
        if not data:
            break
        conn.send(data)
        md5.update(data)
    tracker_file.close()
    conn.send('REP GET END {0}'.format(md5.hexdigest()))


## Handles the server-side communication with the client for the CREATETRACKER command.
# @param conn Socket object connected to client
# @param addr Address of the remote client connected to the server
# @param command CREATETRACKER command submitted by the client
def createtracker(conn, addr, command):
    print "Received createtracker command from {}".format(addr[0])
    tracker_file = tracker_parser.TrackerFile()
    status = tracker_parser.FILE_FAIL

    # Create tracker file or send error message back to client
    try:
        status = tracker_file.create(command)
    except Exception as e:
        print e

    if status == tracker_parser.FILE_SUCC:
        conn.send('createtracker succ')
    elif status == tracker_parser.FILE_ERR:
        conn.send('createtracker ferr')
    else:
        conn.send('createtracker fail')


## Handles the server-side communication with the client for the UPDATETRACKER command.
# @param conn Socket object connected to client
# @param addr Address of the remote client connected to the server
# @param command UPDATETRACKER command submitted by the client
def updatetracker(conn, addr, command):
    print "Received updatetracker command from {}".format(addr[0])
    tracker_file = tracker_parser.TrackerFile()
    status = tracker_parser.FILE_FAIL

    # Update tracker file or send error message back to client
    try:
        status = tracker_file.update(command)
    except:
        pass

    if status == tracker_parser.FILE_SUCC:
        conn.send('updatetracker succ')
    elif status == tracker_parser.FILE_ERR:
        conn.send('updatetracker ferr')
    else:
        conn.send('updatetracker fail')


## Handle commands from client.
# @param client_conn Socket object connected to client
# @param client_addr Address of the remote client connected to the server
def handle_client(client_conn, client_addr):
    try:
        data = client_conn.recv(CHUNK_SIZE)

        # Call appropriate command-handling function or send error message to client
        if data:
            if data.lower() == "req list" or data.lower() == "<req list>":
                list_command(client_conn, client_addr, data)
            elif data.lower().find("get") != -1:
                get_command(client_conn, client_addr, data)
            elif data.lower().find("createtracker") != -1:
                print "[Server]: create command {0}".format(data)
                createtracker(client_conn, client_addr, data)
            elif data.lower().find("updatetracker") != -1:
                updatetracker(client_conn, client_addr, data)
            else:
                print "Server received unrecognized command from {0}: {1}".format(client_addr[0], data)
    except:
        pass

if len(sys.argv) != 2:
  print "Incorrect usage. Correct usage = python server.py <path to server directory"
  exit(1)

os.chdir(sys.argv[1])

print "Starting server, hostname = {}".format(socket.gethostname())
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(('', SERVER_PORT))
print "Bound socket to port {}".format(SERVER_PORT)
sock.listen(5)
print "Listening for clients..."

# Open socket and listen for commands from client
try:
    while True:
        (connection, addr) = sock.accept()
        print "Connection address: {0}".format(addr)
        thread = Thread(target=handle_client, args=(connection, addr))
        thread.start()
except KeyboardInterrupt:
    print
    sock.close()
