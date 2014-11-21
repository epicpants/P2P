'''
	Desc: Handle Basic File I/O for saving/loading
		configuration dictionaries
'''
import json, ast

class fileIO:
	
	# default filename
	defFile = "s:/project.conf"
	
	# sample config data
	config = { "config1" : "A", "config2" : "B", "config3" : "C" }

	# empty constructor
	def __init__( self ): pass

	''' Desc: Saves dictionary to file
		Params: dictionary containing config data, filename string			
	'''
	def saveConfig( confData, fileName = defFile ):
		file = open( fileName, "w" )
		file.write(json.dumps(confData))
		file.close()
		return

	''' Desc: Loads configuration data from file, returns a dictionary
		Params: filename string
	'''
	def loadConfig( fileName ):
		file = open( fileName, "r" )
		
		# contains unwanted 'u' chars
		unicodeData = json.load( file )
		
		file.close()
		
		# parses data, stripping out 'u' chars
		configData = ast.literal_eval( json.dumps( unicodeData ) )
		
		return configData
