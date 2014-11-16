import os
import re
import time

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
        self.tracker_directory = ''
        self.file_directory = ''
        self.filename = ''
        self.file_size = 0
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

        match = re.match(r"Filename:\s([^\n]+)", file_contents)
        if match:
            self.filename = match.group(1)
        match = re.match(r"Filesize:\s([^\n]+)", file_contents)
        if match:
            self.file_size = long(match.group(1))
        match = re.match(r"Description:\s([^\n]*)", file_contents)
        if match:
            self.description = match.group(1)
        match = re.match(r"MD5:\s([^\n]+)", file_contents)
        if match:
            self.md5 = match.group(1)

        for line in file_contents.split('\n'):
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
        pass

    def updateCommand(self, filename, port):
        pass

    def createCommand(self, filename, port, description="_"):
        pass

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

    def _find(self, key, file_contents, value):
        pass

    def _parse_host(self, line):
        pass

    def _rewrite_file(self):
        pass

    def _remove_hosts(self):
        pass



