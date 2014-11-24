import os
import re
import time
import hashlib
import socket

FILE_SUCC = 0
FILE_FAIL = 1
FILE_ERR = 2


class HostInfo():
    def __init__(self):
        self.ip_addr = ''
        self.port = None
        self.start_byte = None
        self.end_byte = None
        self.time_stamp = None


class TrackerFile():
    def __init__(self):
        self.host_TTL = 15
        self.tracker_directory = '.'
        self.file_directory = ''
        self.filename = ''
        self.file_size = long(0)
        self.description = ''
        self.md5 = ''
        self.hosts = []

    def parseTrackerFile(self, tracker_filename):
        error = False

        if not os.path.isfile(tracker_filename):
            error = True
            print "Tracker file does not exist"
            return error

        tracker_file = open(tracker_filename, 'r')
        file_contents = tracker_file.read()

        tracker_file.close()

        for line in file_contents.split('\n'):
            match = re.match(r"Filename:\s(.+)", line)
            if match:
                self.filename = match.group(1)

            match2 = re.match(r"Filesize:\s(.+)", line)
            if match2:
                self.file_size = long(match2.group(1))

            match3 = re.match(r"Description:\s(.*)", line)
            if match3:
                self.description = match3.group(1)

            match4 = re.match(r"MD5:\s(.+)", line)
            if match4:
                self.md5 = match4.group(1)

            host = re.match(r"^([^:\s]+):([^:\s]+):([^:\s]+):([^:\s]+):([^:\s]+)", line)
            if host:
                new_host = HostInfo()
                new_host.ip_addr = host.group(1)
                new_host.port = int(host.group(2))
                new_host.start_byte = long(host.group(3))
                new_host.end_byte = long(host.group(4))
                new_host.time_stamp = long(host.group(5))
                self.hosts.append(new_host)

        if error:
            print "There was an error reading file"
        return error


    def update(self, cmd=''):
        # If no command, remove old hosts
        if cmd == '':
            self._rewrite_file()
            return FILE_SUCC

        tokens = cmd.split(' ')
        if len(tokens) != 6:
            print "Improper update command given"
            return FILE_ERR
        tracker_filename = tokens[1]

        if tracker_filename.find('.track') == -1:
            tracker_filename += '.track'

        if not os.path.isfile(tracker_filename):
            print "Tracker file does not exist"
            return FILE_ERR

        start_byte = tokens[2]
        end_byte = tokens[3]
        ip_address = tokens[4]
        port = tokens[5]
        timestamp = long(time.time())  # whole number of seconds since epoch

        self.parseTrackerFile(tracker_filename)

        update_hosts = [host for host in self.hosts if host.ip_address == ip_address]
        if len(update_hosts) > 0:
            update_hosts[0].start_byte = start_byte
            update_hosts[0].end_byte = end_byte
            update_hosts[0].time_stamp = timestamp
        else:
            new_host = HostInfo()
            new_host.ip_addr = ip_address
            new_host.port = port
            new_host.start_byte = start_byte
            new_host.end_byte = end_byte
            new_host.time_stamp = timestamp
            self.hosts.append(new_host)

        self._rewrite_file()
        return FILE_SUCC

    def create(self, cmd):
        tokens = cmd.split(' ')
        if len(tokens) != 7:
            print "Improper create command given"
            return FILE_ERR

        tracker_filename = tokens[1]

        if tracker_filename.find('.track') == -1:
            tracker_filename += '.track'

        # Check if file exists
        if not os.path.isfile(tracker_filename):
            print "Tracker file does not exist"
            return FILE_ERR

        # Parse command
        self.file_size = tokens[2]
        self.description = tokens[3]
        self.md5 = tokens[4]
        ip_address = tokens[5]
        port = tokens[6]

        start_byte = 0
        end_byte = self.file_size
        time_stamp = long(time.time())

        self.description.replace('_', ' ')

        new_host = HostInfo()
        new_host.ip_addr = ip_address
        new_host.port = port
        new_host.start_byte = start_byte
        new_host.end_byte = end_byte
        new_host.time_stamp = time_stamp
        self.hosts.append(new_host)

        # Remove out-dated host info and rewrite file
        self._rewrite_file()

        return FILE_SUCC

    def updateCommand(self, filename, port, start_byte, end_byte):
        #Generate command to send to server
        cmd = "updatetracker " + filename + " "   #File name
        cmd += start_byte + " " + end_byte + " "  #File chunk

        #Get local IP address
        cmd += str(socket.gethostbyname(socket.gethostname())) + " "

        cmd += port + "\n" #Port number
        return cmd

    def createCommand(self, filename, port, description="_"):
        #Generate command to send to server
        cmd = "createtracker " + filename + " "      #File name
        cmd += str(os.stat(filename).st_size) + " "  #Filesize
        cmd += description.replace(" ","_") + " "    #Description

        #Calculate MD5 Checksum for file
        f = open(filename,'rb')
        block_size=2**20
        md5 = hashlib.md5()
        while True:
            data = f.read(block_size)
            if not data:
                break
            md5.update(data)
        cmd += str(md5.hexdigest()) + " "

        #Get local IP address
        cmd += str(socket.gethostbyname(socket.gethostname())) + " "

        cmd += port + "\n" #Port number

        return cmd

    def get_filename(self):
        return self.filename

    def get_file_size(self):
        return self.file_size

    def get_description(self):
        if self.description == '':
            self.description = '_'
        self.description.replace(' ', '_')
        return self.description

    def get_md5(self):
        return self.md5

    def __getitem__(self, item):
        return self.hosts[item]

    def get_num_hosts(self):
        return len(self.hosts)

    def _rewrite_file(self):
        if self.filename != '':
            # Remove outdated hosts
            self._remove_hosts()
            if os.path.isfile(self.filename + '.track'):
                os.remove(self.filename + '.track')
            tracker_file = open(self.filename + '.track', 'w')
            tracker_file.write('Filename: ' + self.filename + '\n')
            tracker_file.write('Filesize: ' + str(self.file_size) + '\n')
            tracker_file.write('Description: ' + self.description + '\n')
            tracker_file.write('MD5: ' + self.md5 + '\n')

            for host in self.hosts:
                tracker_file.write(host.ip_address + ":")
                tracker_file.write(str(host.port) + ":")
                tracker_file.write(str(host.start_byte) + ":")
                tracker_file.write(str(host.end_byte) + ":")
                tracker_file.write(str(host.time_stamp) + "\n")

            tracker_file.close()

    def _remove_hosts(self):
        current_time = long(time.time())
        for host in self.hosts[:]:
            # Remove hosts if timestamp is older than hostTTL minutes
            if self.host_TTL * 60 <= current_time - host.time_stamp:
                self.hosts.remove(host)
