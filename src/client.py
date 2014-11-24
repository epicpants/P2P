import hashlib
import os
import re
import socket
import sys
import StringIO
from threading import Thread
import time
import tracker_parser

SND = 0
RCV = 1
# TODO: Read from config
SERVER_PORT = 7777
PEER_PORT = 8888
CHUNK_SIZE = 1024
UPDATE_SLEEP_TIME = 10  # in seconds
LIST_SLEEP_TIME = 5  # in seconds
TARGET_FILE = 'picture_wallpaper.jpg'
CLIENT_IP = socket.gethostname()


def command_line_interface():
    print "P2P CLI Started"
    user_command_input = None
    user_command = None
    tracker_file_name = None

    while 1:
        user_command_input = raw_input()
        print user_command_input.type
        user_command_StringIO = StringIO.StringIO(user_command_input)


def req_list():  # return whether list contains TARGET_FILE
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((server_address, SERVER_PORT))
    sock.close()
    return False


def get_tracker_file():  # get command for server
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((server_address, SERVER_PORT))
    sock.close()


def get_file():  # get command for peers, return true if file download successful
    return True


def update_command(start_byte, end_byte):
    error = False
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((server_address, SERVER_PORT))
    tracker_file = tracker_parser.TrackerFile()
    command = tracker_file.updateCommand(TARGET_FILE, PEER_PORT, start_byte, end_byte)
    sock.send(command)
    response = sock.recv(CHUNK_SIZE)
    if not response:
        error = True
    elif response.find('succ') == -1:
        error = True
    sock.close()
    return error


def create_command():  # return whether command was successful
    error = False
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((server_address, SERVER_PORT))
    tracker_file = tracker_parser.TrackerFile()
    command = tracker_file.createCommand(TARGET_FILE, PEER_PORT, '_')
    sock.send(command)
    response = sock.recv(CHUNK_SIZE)
    if not response:
        error = True
    elif response.find('succ') == -1:
        error = True
    sock.close()
    return error


def listen_for_peers():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('', PEER_PORT))
    sock.listen(5)
    while True:
        (connection, addr) = sock.accept()
        filename = ''
        chunk_num = None
        req = connection.recv(CHUNK_SIZE)
        if not req:
            continue
        match = re.match(r"get\s([^\s]+)\s(.+)", req.lower())
        if match:
            filename = match.group(1)
            chunk_num = long(match.group(2))
            if os.path.isfile(filename):
                connection.send('REP GET BEGIN')
            else:
                connection.send('ERROR FILE DNE')
        else:
            connection.send('ERROR NO FILENAME')

        req_file = open(filename, 'rb')
        req_file.seek(chunk_num * CHUNK_SIZE)
        data = req_file.read(CHUNK_SIZE)
        connection.send(data)
        connection.send('REP GET END')


def timer_routine():
    while True:
        if client_type == SND:
            # TODO: Make start_byte and end_byte dynamic based on client_num and time
            start_byte = 0
            end_byte = 0
            if not update_command(start_byte, end_byte):  # an error occurred
                update_command(start_byte, end_byte) # try one more time
            time.sleep(UPDATE_SLEEP_TIME)
        else:
            has_target_file = req_list()
            time.sleep(LIST_SLEEP_TIME)
            if has_target_file:
                break


"""
Entry point
"""
if len(sys.argv) != 4:
    print "Incorrect usage. Correct usage = python client.py <server_address> <0/1 for snd/rcv> <client num>"
    exit(1)
server_address = sys.argv[1]
client_type = sys.argv[2]
client_num = sys.argv[3]

try:
    if client_type == SND:
        while create_command():
            pass
        update_thread = Thread(target=timer_routine)  # Thread to update server periodically
        listen_for_peers()
    else:  # client_type == RCV
        list_thread = Thread(target=timer_routine)  # Thread to request list from server periodically
        list_thread.join()
        download_succ = get_file()
        if download_succ:
            print "I am client_{0} and I received the file correctly!".format(client_num)
except KeyboardInterrupt:
    print



"""
data = raw_input("Enter client command:\n")
assert isinstance(data, str)
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
"""