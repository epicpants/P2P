## @package fileIO Class implementation for handling config files

import json
import ast
import os

## Handles basic File I/O for saving/loading configuration dictionaries
class fileIO:
	
	## Default filename to use
	defFile = "./project.conf"
	
	## Sample config data
	config = {"config1" : "A", "config2" : "B", "config3" : "C"}

	## Unused class constructor
	def __init__(self): pass

	## Saves dictionary to file
	# @param self Reference to this class instance
	# @param confData Python dictionary containing config data
	# @param filename String defining filename and path
	def saveConfig(self, confData, fileName = defFile):
		dir = os.path.dirname(fileName)
		if not os.path.exists(dir):
			os.makedirs(dir)
		file = open( fileName, "w" )
		file.write(json.dumps(confData))
		file.close()
		return

	## Loads configuration data from file
	# @return a dictionary containing configuration pairs
	# @param self Reference to this class instance
	# @param filename String defining filename and path
	def loadConfig(self, fileName):
		file = open( fileName, "r" )
		
		# contains unwanted 'u' chars
		unicodeData = json.load( file )
		
		file.close()
		
		# parses data, stripping out 'u' chars
		configData = ast.literal_eval( json.dumps( unicodeData ) )
		
		return configData
