import hashlib
import mmap
import os
import re
import socket
import sys
import StringIO
import time
import tracker_parser
import threading
from fileIO import fileIO

# Old Config Options
'''
SND = 0
RCV = 1
SERVER_PORT = 7777
PEER_PORT = 8888
CHUNK_SIZE = 1024
THREAD_TIMEOUT = 10
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
'''

# Read from config file
confFile = "./client.conf"
fileHandler = fileIO()

# Load configuration into reference list
config = fileHandler.loadConfig( confFile )

# Determine current hostname
config["CLIENT_IP"] = socket.gethostname()

# Multithreading resource lock
THREAD_LOCK = threading.Lock()

# List of (start_byte, end_byte) tuples indicating ranges of bytes not yet written
unwritten_bytes = [(0, config["TARGET_FILE_SIZE"])]

PERC_BYTES_DICT = {}
for percent in range(101):  # [0, 100]
    mod = percent % 5
    if mod == 0:  # divisible by 5
        PERC_BYTES_DICT[percent] = long(float(percent)/100 * config["TARGET_FILE_SIZE"])
    elif mod == 1:
        PERC_BYTES_DICT[percent] = PERC_BYTES_DICT[percent - 1] + 1


## Uses time_slot parameter to determine segment specifics
#   by means of a stored lookup dictionary: CLIENT_PERC_DICT
#   Returns percent and bytes ranges: percent_low, percent_high, start_byte, end_byte
# @param time_slot Integer defining which timeslot a client is using (1-5).
def advertise_info(time_slot):
    if time_slot == 0:
        percent_low = config["CLIENT_PERC_DICT"][client_num][0]
    elif client_num != 1:
        percent_low = config["CLIENT_PERC_DICT"][client_num][time_slot - 1] + 1
    else:
        percent_low = 0
    percent_high = config["CLIENT_PERC_DICT"][client_num][time_slot]
    start_byte = PERC_BYTES_DICT[percent_low]
    end_byte = PERC_BYTES_DICT[percent_high]
    return percent_low, percent_high, start_byte, end_byte


## Parses host line from tracker file to determine the starting
#   byte range for the given host, as well as configured number of bytes.
#   Returns; start_byte, num_bytes
# @param host Single Host line from Tracker File
def get_bytes_to_req(host):
    THREAD_LOCK.acquire()
    byte_range = [(start, end) for (start, end) in unwritten_bytes if host.start_byte < end and host.start_byte >= start]
    if len(byte_range) == 0:  # no bytes the host is offering need to be written
        THREAD_LOCK.release()
        return 0, 0
    start_byte = byte_range[0][0]  # take the start_byte of the first found tuple
    num_bytes = config["CHUNK_SIZE"]
    if start_byte + num_bytes > byte_range[0][1]:
        num_bytes = byte_range[0][1] - start_byte  # + 1?
    THREAD_LOCK.release()
    return start_byte, num_bytes


##
#   Note: Should only be called from within critical section
# @param start_byte
# @param num_bytes
def update_unwritten_bytes(start_byte, num_bytes):
    byte_range = [(start, end) for (start, end) in unwritten_bytes if start_byte < end and start_byte >= start]
    if len(byte_range) == 0:
        return
    (old_start, old_end) = byte_range[0]

    # split the range of bytes into new ranges that effectively removes the range of data we just wrote
    (start1, end1) = (old_start, start_byte - 1)
    (start2, end2) = (start_byte + num_bytes + 1, old_end)
    if end1 > start1:
        unwritten_bytes.append((start1, end1))
    if end2 > start2:
        unwritten_bytes.append((start2, end2))
    unwritten_bytes.remove(byte_range[0])


## Parser for CLI. Recognizes the following commands (case insensitive)...
#   createtracker "filename"    -   creates tracker file for specified file
#   list                        -   lists available tracker files
#   get "tracker-file"          -   requests file specified in tracker-file
#   updatetracker "tracker-file"    -   updates server with info in tracker-file
def command_line_interface():
    print "P2P CLI Started"
    user_command_input = None
    user_command = None
    tracker_file_name = None

    while 1:
        user_command_input = raw_input()
        #print user_command_input.type
        #user_command_StringIO = StringIO.StringIO(user_command_input)
        user_command = user_command_input.split()

        if(user_command[0] == "createtracker" or user_command[0] == "CREATETRACKER"):
            print "createtracker"
            #tracker_file_name = user_command[1]
            create_command()#tracker_file_name)
        elif(user_command[0] == "list" or user_command[0] == "LIST"):
            print "Getting list of trackers from server"
            req_list()
        elif(user_command[0] == "get" or user_command[0] == "GET"):
            print "GET"
            #tracker_file_name = user_command[1]
            get_tracker_file()#tracker_file_name)
        elif(user_command[0] == "updatetracker" or user_command[0] == "UPDATETRACKER"):
            print "updatetracker"
            #tracker_file_name = user_command[1]
            update_command()#tracker_file_name)
        else:
            print "Command not recognized"


## Requests listing of available tracker files on server.
#   Returns true if server responds with info, false otherwise.
def req_list():
    has_target_file = False
    num_files = 0
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((server_address, config["SERVER_PORT"]))
    sock.send('REQ LIST')
    response = sock.recv(config["CHUNK_SIZE"])
    if not response:
        return has_target_file
    match = re.match(r"REP\sLIST\sBEGIN\s(\d+)", response)
    if match:
        num_files = match.group(1)
    count = 1
    while count <= num_files:
        response = sock.recv(config["CHUNK_SIZE"])
        if not response:
            break
        if response.find(config["TARGET_FILE"]) != -1:
            has_target_file = True
        count += 1
    response = sock.recv(config["CHUNK_SIZE"])
    sock.close()
    return has_target_file


## Implementation of 'GET' command for server. Currently uses the
#   tracker file specified in the clients configuration file.
#   Returns true if there is an error downloading the file,
#   specifically if there is an MD5 mismatch with the stored value.
def get_tracker_file():
    error = True
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((server_address, config["SERVER_PORT"]))
    sock.send('GET {0}.track'.format(config["TARGET_FILE"]))
    data = sock.recv(config["CHUNK_SIZE"])
    if not data:
        return error
    elif data.find('REP GET BEGIN') == -1:
        return error

    tracker_filename = '{0}.track'.format(config["TARGET_FILE"])
    if os.path.isfile(tracker_filename):
        os.remove(tracker_filename)
    tracker_file = open(tracker_filename, 'wb')
    calc_tracker_md5 = hashlib.md5()
    rec_tracker_md5 = ''

    while True:
        data = sock.recv(config["CHUNK_SIZE"])
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


## Threadable GET implementation for acquiring data from peers.
#   Called by get_file(). Writes data to writer parameter.
# @param peer_address Specifies the peer to request data from.
# @param start_byte Specifies the beginning byte to request.
# @param num_bytes Specifies the number of bytes to request.
# @param writer mmap file handler, see: http://pymotw.com/2/mmap/
def thread_handler(peer_address, start_byte, num_bytes, writer):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((peer_address, config["PEER_PORT"]))
    sock.send('GET {0} {1} {2}'.format(config["TARGET_FILE"], start_byte, num_bytes))
    # Confirm 'get' request
    data = sock.recv(config["CHUNK_SIZE"])
    if not data:
        sock.close()
        return
    elif data != 'REP GET BEGIN':
        sock.close()
        return

    # Receive data
    data = sock.recv(num_bytes)
    if not data or len(data) != num_bytes:
        sock.close()
        return

    # Enter critical section
    THREAD_LOCK.acquire()
    # Write to file
    writer.seek(start_byte)
    writer.write(data)
    writer.flush()
    update_unwritten_bytes(start_byte, num_bytes)
    THREAD_LOCK.release()

    # Confirm 'get' complete
    data = sock.recv(config["CHUNK_SIZE"])
    if not data:
        sock.close()
        return
    sock.close()


## Initializes 'GET' command for peers, using thread_handler() to acquire data.
#   Uses target file specified by clients config file to determine request.
#   Returns true if file download successful.
def get_file():
    while get_tracker_file():
        pass  # loop until successfully have tracker file

    # Create file handle for target file
    target_file = open(config["TARGET_FILE"], 'wb+')

    # Create memory mapped writer to target file of specified size
    # For details, see: http://pymotw.com/2/mmap/
    writer = mmap.mmap(target_file.fileno(), config["TARGET_FILE_SIZE"])
    tracker_file = tracker_parser.TrackerFile()
    while True:
        if tracker_file.parseTrackerFile('{0}.track'.format(config["TARGET_FILE"])):  # true if error
            return False

        threads = []
        for host in tracker_file.hosts:  # spawn a new thread for each host
            (start_byte, num_bytes) = get_bytes_to_req(host)
            if num_bytes > 0:
                thread = threading.Thread(target=thread_handler, args=(host.ip_addr, start_byte, num_bytes, writer))
                thread.start()
                threads.append(thread)

        # wait until all threads complete or timeout occurs
        timeout = 0
        while len([thread for thread in threads if thread.is_alive()]) != 0 and timeout < config["THREAD_TIMEOUT"]:
            timeout += 1
            time.sleep(1)

        if len(unwritten_bytes) == 0:  # no bytes left to be written - we're done!
            break

    return True


def update_command(start_byte, end_byte):  # return whether command was successful
    error = False
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((server_address, config["SERVER_PORT"]))
    tracker_file = tracker_parser.TrackerFile()
    command = tracker_file.updateCommand(config["TARGET_FILE"], config["PEER_PORT"], start_byte, end_byte)
    sock.send(command)
    response = sock.recv(config["CHUNK_SIZE"])
    if not response:
        error = True
    elif response.find('succ') == -1:
        error = True
    sock.close()
    return error


def create_command():  # return whether command was successful
    error = False
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((server_address, config["SERVER_PORT"]))
    tracker_file = tracker_parser.TrackerFile()
    command = tracker_file.createCommand(config["TARGET_FILE"], config["PEER_PORT"], '_')
    sock.send(command)
    response = sock.recv(config["CHUNK_SIZE"])
    if not response:
        error = True
    elif response.find('succ') == -1:
        error = True
    sock.close()
    return error


def listen_for_peers():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('', config["PEER_PORT"]))
    sock.listen(5)
    while True:
        (connection, addr) = sock.accept()
        filename = ''
        start_byte = None
        req = connection.recv(config["CHUNK_SIZE"])
        if not req:
            continue
        match = re.match(r"get\s([^\s]+)\s(\d+)\s(\d+)", req.lower())
        if match:
            filename = match.group(1)
            start_byte = long(match.group(2))
            num_bytes = long(match.group(3))
            if os.path.isfile(filename):
                connection.send('REP GET BEGIN')
            else:
                connection.send('ERROR FILE DNE')
                continue
            if num_bytes > config["CHUNK_SIZE"]:
                connection.send('GET invalid')
                continue
        else:
            connection.send('ERROR NO FILENAME')
            continue

        req_file = open(filename, 'rb')
        req_file.seek(start_byte)
        data = req_file.read(num_bytes)
        connection.send(data)
        connection.send('REP GET END')


def timer_routine(get_update=False):
    time_slot = 1
    while True:
        if client_type == config["SND"]:
            (percent_low, percent_high, start_byte, end_byte) = advertise_info(time_slot)
            if not update_command(start_byte, end_byte):  # an error occurred
                update_command(start_byte, end_byte)  # try one more time
            print "I am client_{0}, and I am advertising the following chunk of the file: {1}% to {2}%".format(client_num, percent_low, percent_high)
            time.sleep(config["UPDATE_SLEEP_TIME"])
        elif not get_update:
            has_target_file = req_list()
            if has_target_file:
                break
            time.sleep(config["LIST_SLEEP_TIME"])
        else:
            if not get_tracker_file():  # an error occurred
                get_tracker_file()  # try one more time
            time.sleep(config["UPDATE_SLEEP_TIME"])
        time_slot += 1
        if time_slot > 4:
            time_slot = 4

"""
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
    if client_type == config["SND"]:
        while create_command():
            pass
        update_thread = threading.Thread(target=timer_routine)  # Thread to update server periodically
        update_thread.start()
        listen_for_peers()
    else:  # client_type == config["RCV"]
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
