import hashlib
import mmap
import os
import re
import socket
import sys
import StringIO
import threading
import time
import tracker_parser
import socket
import threading

threadLock = threading.lock()

SND = 0
RCV = 1
# TODO: Read from config
SERVER_PORT = 7777
PEER_PORT = 8888
CHUNK_SIZE = 1024
UPDATE_SLEEP_TIME = 10  # in seconds
LIST_SLEEP_TIME = 5  # in seconds
TARGET_FILE = 'picture_wallpaper.jpg'
TARGET_FILE_SIZE = 35738
CLIENT_IP = socket.gethostname()
CLIENT_DIR = '.'
CLIENT_PERC_DICT = {1: [0, 5, 10, 15, 20],
                    2: [21, 25, 30, 35, 40],
                    3: [41, 45, 50, 55, 60],
                    4: [61, 65, 70, 75, 80],
                    5: [81, 85, 90, 95, 100]}
PERC_BYTES_DICT = {}
for percent in range(101):  # [0, 100]
    mod = percent % 5
    if mod == 0:  # divisible by 5
        PERC_BYTES_DICT[percent] = long(float(percent)/100 * TARGET_FILE_SIZE)
    elif mod == 1:
        PERC_BYTES_DICT[percent] = PERC_BYTES_DICT[percent - 1] + 1


def advertise_info(time_slot):
    if time_slot == 0:
        percent_low = CLIENT_PERC_DICT[client_num][0]
    elif client_num != 1:
        percent_low = CLIENT_PERC_DICT[client_num][time_slot - 1] + 1
    else:
        percent_low = 0
    percent_high = CLIENT_PERC_DICT[client_num][time_slot]
    start_byte = PERC_BYTES_DICT[percent_low]
    end_byte = PERC_BYTES_DICT[percent_high]
    return percent_low, percent_high, start_byte, end_byte


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
    has_target_file = False
    num_files = 0
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((server_address, SERVER_PORT))
    sock.send('REQ LIST')
    response = sock.recv(CHUNK_SIZE)
    if not response:
        return has_target_file
    match = re.match(r"REP\sLIST\sBEGIN\s(\d+)", response)
    if match:
        num_files = match.group(1)
    count = 1
    while count <= num_files:
        response = sock.recv(CHUNK_SIZE)
        if not response:
            break
        if response.find(TARGET_FILE) != -1:
            has_target_file = True
        count += 1
    response = sock.recv(CHUNK_SIZE)
    sock.close()
    return has_target_file


def get_tracker_file():  # 'get' command for server, return true if got tracker file
    error = True
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((server_address, SERVER_PORT))
    sock.send('GET {0}.track'.format(TARGET_FILE))
    data = sock.recv(CHUNK_SIZE)
    if not data:
        return error
    elif data.find('REP GET BEGIN') == -1:
        return error

    tracker_filename = '{0}.track'.format(TARGET_FILE)
    if os.path.isfile(tracker_filename):
        os.remove(tracker_filename)
    tracker_file = open(tracker_filename, 'wb')
    calc_tracker_md5 = hashlib.md5()
    rec_tracker_md5 = ''

    while True:
        data = sock.recv(CHUNK_SIZE)
        if not data:
            break
        match = re.match(r"REP\sGET\sEND\s(.+)", data)
        if match:
            rec_tracker_md5 = match.group(1)
            break
        tracker_file.write(data)
        calc_tracker_md5.update(data)

    if str(calc_tracker_md5.hexdigest()) == rec_tracker_md5:
        error = False
    sock.close()
    tracker_file.close()
    return error


def thread_handler(peer_ip, start_byte, num_bytes):
    pass


def get_file():  # 'get' command for peers, return true if file download successful
    while get_tracker_file():
        pass  # loop until successfully have tracker file

    target_file = open(TARGET_FILE, 'wb+')
    writer = mmap.mmap(target_file.fileno(), TARGET_FILE_SIZE)  # create blank file of specified size
    tracker_file = tracker_parser.TrackerFile()
    while True:
        if tracker_file.parseTrackerFile('{0}.track'.format(TARGET_FILE)):  # true if error
            return False

    return True


def update_command(start_byte, end_byte):  # return whether command was successful
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
        match = re.match(r"get\s([^\s]+)\s(\d+)", req.lower())
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


def timer_routine(get_update=False):
    time_slot = 1
    while True:
        if client_type == SND:
            (percent_low, percent_high, start_byte, end_byte) = advertise_info(time_slot)
            if not update_command(start_byte, end_byte):  # an error occurred
                update_command(start_byte, end_byte)  # try one more time
            print "I am client_{0}, and I am advertising the following chunk of the file: {1}% to {2}%".format(client_num, percent_low, percent_high)
            time.sleep(UPDATE_SLEEP_TIME)
        elif not get_update:
            has_target_file = req_list()
            if has_target_file:
                break
            time.sleep(LIST_SLEEP_TIME)
        else:
            if not get_tracker_file():  # an error occurred
                get_tracker_file()  # try one more time
            time.sleep(UPDATE_SLEEP_TIME)
        time_slot += 1
        if time_slot > 4:
            time_slot = 4

# threadable method for acquiring file segment
def getSegment( socket, segNum, finalSeg ):
    bufferSize = 1024
    
    #read in data from socket
    buffer = socket.recv( bufferSize )
    
    #determine if byte count is valid
    if buffer == '':
        raise IOError("socket connection broken!")
        return
        
    if( ( len(buffer) < bufferSize ) and ( segNum != finalSeg ) ):
        raise IOError("Invalid segment size received, exiting thread!"
        return
	
    #call writeOut
    threadLock.acquire()
	
    #call write to file function
    # writeOut( segNum, finalSeg, buffer )
	
    threadLock.release()
	
    socket.close()
	
    return


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
        update_thread = threading.Thread(target=timer_routine)  # Thread to update server periodically
        update_thread.start()
        listen_for_peers()
    else:  # client_type == RCV
        list_thread = threading.Thread(target=timer_routine)  # Thread to request list from server periodically
        list_thread.start()
        list_thread.join()
        get_update_thread = threading.Thread(target=timer_routine, args=(True,))
        get_update_thread.start()
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
