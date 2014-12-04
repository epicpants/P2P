## @package tracker_parser Implementation of the TrackerFile and HostInfo classes

import os
import re
import time
import hashlib
import socket

## Indicates successful status
FILE_SUCC = 0

## Indicates failed status
FILE_FAIL = 1

## Indicates an error status
FILE_ERR = 2

## HostInfo contains information about the client hosting a tracked file.
class HostInfo():
    ## @var ip_addr
    # IP address of hosting client
    ## @var port
    # Port hosting client is listening on
    ## @var start_byte
    # The start byte offset for the file segment available
    ## @var end_byte
    # The end byte offset for the file segment available
    ## @var time_stamp
    # The last time that the hosts information was updated

    ## Constructor for HostInfo.
    #  Assigns all variables empty values
    # @param self Reference to this object
    def __init__(self):
        self.ip_addr = ''
        self.port = int(0)
        self.start_byte = None
        self.end_byte = None
        self.time_stamp = None

## TrackerFile contains information about the files currently being tracked and
# the clients hosting segments of the tracked files. The TrackerFile class also
# contains functionality that creates and updates the tracker files, as well as
# functionality for getting information from the tracker file.
class TrackerFile():
    ## @var host_TTL
    # The time in minutes before information about a host is considered invalid.
    # The host's information will be removed from the tracker file after a 
    # specified number of minutes.
    ## @var tracker_directory
    # The directory containing the tracker files (defaults to this directory).
    ## @var file_directory
    # The directory containing the tracked files (defaults to this directory).
    ## @var filename
    # The name of the file being tracked.
    ## @var file_size
    # The size of the file being tracked (in bytes).
    ## @var description
    # A description about the file being tracked.
    ## @var md5
    # The MD5 hash of the file being tracked.
    ## @var hosts
    # A list of the clients hosting some portion of the file being tracked.

    ## Constructor for TrackerFile.
    # Assigns default values to member variables.
    # @param self Reference to this object
    def __init__(self):
        self.host_TTL = 15
        self.tracker_directory = '.'
        self.file_directory = ''
        self.filename = ''
        self.file_size = long(0)
        self.description = ''
        self.md5 = ''
        self.hosts = []

    ## Parse the tracker file and store in the member variables.
    # @param self Reference to this object
    # @param tracker_filename Name of the tracker file
    # @return The error status is returned (FILE_SUCC, FILE_FAIL, FILE_ERR)
    def parseTrackerFile(self, tracker_filename):
        error = False

        if not os.path.isfile(tracker_filename):
            error = True
            print "Tracker file {0} does not exist".format(tracker_filename)
            return error

        tracker_file = open(tracker_filename, 'r')
        file_contents = tracker_file.read()

        tracker_file.close()

        for line in file_contents.split('\n'):
            # Get the filename from the tracker file
            match = re.match(r"Filename:\s(.+)", line)
            if match:
                self.filename = match.group(1)

            # Get the file size from the tracker file
            match2 = re.match(r"Filesize:\s(.+)", line)
            if match2:
                self.file_size = long(match2.group(1))

            # Get the file description from the tracker file
            match3 = re.match(r"Description:\s(.*)", line)
            if match3:
                self.description = match3.group(1)

            # Get the MD5 hash from the tracker file
            match4 = re.match(r"MD5:\s(.+)", line)
            if match4:
                self.md5 = match4.group(1)

            # Get the information about the clients hosting the file
            host = re.match(r"^([^:\s]+):([^:\s]+):([^:\s]+):([^:\s]+):(\d+)", line)
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


    ## Updates the tracker file with the command from the client.
    # If no command is specified then invalid hosts are removed.
    # @param self Reference to this object
    # @param cmd The UPDATETRACKER command from the client
    # @return The error status is returned (FILE_SUCC, FILE_FAIL, FILE_ERR)
    def update(self, cmd=''):
        # If no command, remove old hosts
        # print "^^^^^^^^^^^^^^^^^^[tracker parser update] cmd = {0}".format(cmd)
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
            print "Tracker file {0} does not exist [update]".format(tracker_filename)
            return FILE_ERR

        # Collect information about the host from the command
        start_byte = tokens[2]
        end_byte = tokens[3]
        ip_address = tokens[4]
        port = int(tokens[5])
        timestamp = long(time.time())  # whole number of seconds since epoch

        self.parseTrackerFile(tracker_filename)

        # Update host if it exists or create new host if not found
        update_hosts = [host for host in self.hosts if host.ip_addr == ip_address and host.port == port]
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

    ## Creates the tracker file with the command from the client.
    # @param self Reference to this object
    # @param cmd The CREATETRACKER command from the client
    # @return The error status is returned (FILE_SUCC, FILE_FAIL, FILE_ERR)
    def create(self, cmd):
        tokens = cmd.split(' ')
        if len(tokens) != 7:
            print "Improper create command given"
            return FILE_ERR

        tracker_filename = tokens[1]
        self.filename = tracker_filename.replace(".track", "")

        if tracker_filename.find('.track') == -1:
            tracker_filename += '.track'

        """
        # Check if file exists
        if not os.path.isfile(tracker_filename):
            print "Tracker file {0} does not exist [create]".format(tracker_filename)
            return FILE_ERR
        """

        # Parse command
        self.file_size = tokens[2]
        self.description = tokens[3]
        self.md5 = tokens[4]
        ip_address = tokens[5]
        port = int(tokens[6])

        start_byte = 0
        end_byte = self.file_size
        time_stamp = long(time.time())

        self.description.replace('_', ' ')

        # Add new host to lists of hosts
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

    ## Generates the UPDATETRACKER command for the client
    # @param self Reference to this object
    # @param filename Name of file being tracked
    # @param port Port host machine will listen on
    # @param start_byte Start byte offset of file segment available on this host
    # @param end_byte End byte offset of file segment available on this host
    # @return UPDATETRACKER command to be used by this client
    def updateCommand(self, filename, port, start_byte, end_byte):
        #Generate command to send to server
        cmd = "updatetracker " + filename + " "   #File name
        cmd += str(start_byte) + " " + str(end_byte) + " "  #File chunk

        #Get local IP address
        cmd += str(socket.gethostbyname(socket.gethostname())) + " "

        cmd += str(port) + "\n" #Port number
        return cmd

    ## Generates the CREATETRACKER command for the client
    # @param self Reference to this object
    # @param filename Name of the file to be tracked
    # @param port Port host machine will listen on
    # @param description Description of the file to be tracked
    # @return CREATETRACKER command to be used by this client
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

        cmd += str(port) + "\n" #Port number
        return cmd

    ## Gets the filename of the tracked file.
    # @param self Reference to this object
    # @return Name of the file being tracked
    def get_filename(self):
        return self.filename

    ## Gets the file size of the tracked file.
    # @param self Reference to this object
    # @return Size of the file being tracked
    def get_file_size(self):
        return self.file_size

    ## Gets the description of the tracked file.
    # @param self Reference to this object
    # @return Description of the file being tracked
    def get_description(self):
        if self.description == '':
            self.description = '_'
        self.description.replace(' ', '_')
        return self.description

    ## Gets the MD5 hash of the tracked file.
    # @param self Reference to this object
    # @return MD5 hash of the file being tracked
    def get_md5(self):
        return self.md5

    ## Gets a HostInfo containing information about one of this file's hosts
    # @param self Reference to this object
    # @param item Index of host in tracker file's hosts list
    # @return An instance of HostInfo about one of the file's hosts
    def __getitem__(self, item):
        return self.hosts[item]

    ## Gets the number of clients hosting segments of tracked file
    # @param self Reference to this object
    # @return Number of clients hosting segments of tracked file
    def get_num_hosts(self):
        return len(self.hosts)

    ## Rewrites the information stored in this class back to the tracker file.
    # @param self Reference to this object
    def _rewrite_file(self):
        if self.filename != '':
            # Remove outdated hosts
            self._remove_hosts()
            """
            if os.path.isfile(self.filename + '.track'):
                os.remove(self.filename + '.track')
            """

            #Write tracked file info to tracker file
            tracker_file = open(self.filename + '.track', 'w+')
            # print "Made it to rewrite file"
            tracker_file.write('Filename: ' + self.filename + '\n')
            tracker_file.write('Filesize: ' + str(self.file_size) + '\n')
            tracker_file.write('Description: ' + self.description + '\n')
            tracker_file.write('MD5: ' + self.md5 + '\n')

            #Write valid host information to tracker file
            for host in self.hosts:
                # print "{0}:{1}:{2}:{3}:{4}:{5}".format(host.ip_addr, host.port, host.start_byte, host.end_byte, host.time_stamp)
                tracker_file.write(host.ip_addr + ":")
                tracker_file.write(str(host.port) + ":")
                tracker_file.write(str(host.start_byte) + ":")
                tracker_file.write(str(host.end_byte) + ":")
                tracker_file.write(str(host.time_stamp) + "\n")

            tracker_file.close()

    ## Removes invalid (or out-dated) hosts from the list of hosts.
    # @param self Reference to this object
    def _remove_hosts(self):
        current_time = long(time.time())
        for host in self.hosts[:]:
            # Remove hosts if timestamp is older than hostTTL minutes
            if self.host_TTL * 60 <= current_time - host.time_stamp:
                self.hosts.remove(host)
