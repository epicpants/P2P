#include "tracker_parser.h"

int TrackerFile::hostTTL = 15; //Default 15 min until delete hosts
string TrackerFile::trackerDirectory = ".";
string TrackerFile::fileDirectory = ".";

TrackerFile::TrackerFile()
{
  m_filename    = "";
  m_filesize    = 0;
  m_description = "";
  m_md5         = "";
}

string TrackerFile::getFilename()
{
  return m_filename;
}

long TrackerFile::getFilesize()
{
  return m_filesize;
}

string TrackerFile::getDescription()
{
  string retVal = m_description;

  for(int i=0,size=retVal.size(); i<size; i++)
    if(retVal[i] == ' ')
      retVal[i] = '_';

  if(retVal.size()==0)
    retVal = '_'; 
 
  return retVal;
}

string TrackerFile::getMD5()
{
  return m_md5;
}

bool TrackerFile::parseTrackerFile(const char* trackerFileName)
{
  //Read file into fileContents
  long   size         = 0;
  size_t length       = 0;
  char*  fileContents = NULL;
  FILE*  trkrFile     = NULL;
  bool   error        = false;

  //Open file
  string trackerFileNameStr = trackerDirectory + "/" + trackerFileName;
  trkrFile = fopen(trackerFileNameStr.c_str(),"r");

  //Read File
  if(trkrFile!=NULL)
  {
    if((fseek(trkrFile, 0L, SEEK_END) == 0) &&
      ((size = ftell(trkrFile)) != -1))
    {
      fileContents = new char[size+1];
      if(fseek(trkrFile, 0L, SEEK_SET) == 0)
      {
        length = fread(fileContents, sizeof(char), size, trkrFile);

        if(length != 0) fileContents[size] = '\0';
        else error = true;
      }
      else error = true;
    }
    else error = true;
    fclose(trkrFile);
  }
  else error = true;

  //Parse File
  stringstream convert;
  char* pch  = NULL;
  char* line = fileContents;

  if(error == 0)
  {
    find<string>("Filename",fileContents,m_filename);
    find<long>("Filesize",fileContents,m_filesize);
    find<string>("Description",fileContents,m_description);
    find<string>("MD5",fileContents,m_md5);

    pch = strtok(fileContents,"\n\0");
    while(pch != NULL)
    {
      parseHost(pch);
      line += strlen(pch)+1;
      pch = strtok(line,"\n\0");
    }
  }
  else
  {
    printf("TrackerFile::parseTrackerFile   There was an error reading file\n");
  }

  if(fileContents!=NULL) delete [] fileContents;
  return error;
}

bool TrackerFile::parseHost(char* line)
{
  vector<string> host;
  char* pch = NULL;

  //Parse 
  pch = strtok(line,":\n\0");
  while(pch != NULL)
  {
    if(strlen(pch) > 0)
      host.push_back(string(pch));
    pch = strtok(NULL,":\n\0");
  }

  //Add host to global
  if(host.size() == 5)
  {
    HostInfo hi;
    stringstream convert;

    for(int i=0,size=host.size(); i<size; i++)
      convert << host[i] << " ";

    convert >> hi.ipaddr
            >> hi.port
            >> hi.startByte
            >> hi.endByte
            >> hi.timestamp;

    m_hosts.push_back(hi);
    return true;
  }

  return false;
}

int TrackerFile::update(const char* cmd)
{
  //If no command then remove old hosts;
  if(cmd == NULL)
  {
    rewriteFile();
    return FILE_SUCC;
  }

  stringstream ss(cmd);
  string trackFile;
  FILE* existingFile;

  long startByte, endByte, timestamp;
  string ipaddr;
  int port;

  int index = -1;

  //Get rid of updateTracker command and copy filename
  ss >> trackFile >> trackFile;
  trackFile = trackerDirectory + "/" + trackFile + ".track";

  //Check if file exists
  if(existingFile = fopen(trackFile.c_str(), "r"))
  {
    fclose(existingFile);

    //Parse Command
    ss >> startByte >> endByte >> ipaddr >> port;
    timestamp = time(NULL);

    parseTrackerFile(trackFile.c_str());

    //Modify existing tracker information
    for(int i=0,size=m_hosts.size(); i<size; i++)
      if(m_hosts[i].ipaddr == ipaddr)// && m_hosts[i].port == port)
        index = i;

    if(index != -1)
    {
      m_hosts[index].startByte = startByte;
      m_hosts[index].endByte   = endByte;
      m_hosts[index].timestamp = timestamp;
    }
    else
    {
      HostInfo newHost;
      newHost.ipaddr    = ipaddr;
      newHost.port      = port;
      newHost.startByte = startByte;
      newHost.endByte   = endByte;
      newHost.timestamp = timestamp;
      m_hosts.push_back(newHost);
    }

    //Remove out-dated host info and rewrite file
    rewriteFile();

    return FILE_SUCC;  
  }
  return FILE_ERR;
}

int TrackerFile::create(const char* cmd)
{
  stringstream ss(cmd);
  string trackFile;
  FILE* existingFile;

  long startByte, endByte, timestamp;
  string ipaddr;
  int port;

  int index = -1;

  //Get rid of updateTracker command and copy filename
  ss >> m_filename >> m_filename;
  trackFile = trackerDirectory + "/" + m_filename + ".track";

  //Check if file exists
  if(existingFile = fopen(trackFile.c_str(), "r"))
  {
    fclose(existingFile);
    return FILE_ERR;
  }

  //Parse Command
  ss >> m_filesize >> m_description >> m_md5 >> ipaddr >> port;

  startByte = 0;
  endByte   = m_filesize;
  timestamp = time(NULL);

  for(int i=0,size=m_description.size(); i<size; i++)
    if(m_description[i] == '_')
      m_description[i] = ' ';
  
  HostInfo hi;
  hi.ipaddr    = ipaddr;
  hi.port      = port;
  hi.startByte = startByte;
  hi.endByte   = endByte;
  hi.timestamp = timestamp;
  m_hosts.push_back(hi);

  //Remove out-dated host info and rewrite file
  rewriteFile();

  return FILE_SUCC;
}

const HostInfo& TrackerFile::operator[](int i) const
{
  return m_hosts[i];
}

int TrackerFile::getNumHosts() const
{
  return m_hosts.size();
}

void TrackerFile::rewriteFile()
{
  if(m_filename.size() > 0)
  {
    //Remove outdated hosts
    removeHosts();

    //Rewrite tracker file
    string trackFile = trackerDirectory + "/" + m_filename + ".track";
    ofstream outFile(trackFile.c_str());

    outFile << "Filename: "      << m_filename
            << "\nFilesize: "    << m_filesize
            << "\nDescription: " << m_description
            << "\nMD5: "         << m_md5 << endl;
    for(int i=0,size=m_hosts.size(); i<size; i++)
    {
      outFile << m_hosts[i].ipaddr    << ":"
              << m_hosts[i].port      << ":"
              << m_hosts[i].startByte << ":"
              << m_hosts[i].endByte   << ":"
              << m_hosts[i].timestamp << endl;
    }

    outFile.close();
  }
}

void TrackerFile::removeHosts()
{
  long currentTime = time(NULL);
  vector<HostInfo>::iterator it;
  double dt;

  //Remove hosts is timestamp older than hostTTL minutes
  for(it=m_hosts.begin(); it!=m_hosts.end();)
  {
    if((hostTTL*60.0) <= (dt = difftime(currentTime,(*it).timestamp)))
    {
      it = m_hosts.erase(it);
    }
    else
    {
      it++;
    }
  }
}

const char* TrackerFile::updateCommand(const char* filename, int port)
{
  string md5Val = "0";
  stringstream cmd;
  cmd << "<updatetracker ";

  //Add filename
  cmd << filename << " ";

  //Add start and end bytes
  ifstream in(filename, ifstream::ate | ifstream::binary);
  cmd << "0 " << in.tellg() << " ";
  in.close();

  //Determine and Add IP Address
  struct ifreq ifr;
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  ifr.ifr_addr.sa_family = AF_INET; //IPv4
  strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);
  cmd << inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr) << " ";

  //Add Port
  cmd << port << ">\n";

  return cmd.str().c_str();
}

const char* TrackerFile::createCommand(const char* filename, int port, const char* description)
{
  string md5Val      = "0";
  string descriptStr = description;;
  stringstream cmd;
  cmd << "<createtracker ";

  //Add filename
  cmd << filename << " ";

  //Add filesize
  ifstream in(filename, ifstream::ate | ifstream::binary);
  cmd << in.tellg() << " ";
  in.close();

  //Add description
  for(int i=0,size=descriptStr.size(); i<size; i++)
    if(descriptStr[i]==' ')
      descriptStr[i] = '_';
  cmd << descriptStr << " ";

  //Calculate MD5 Checksum (Linux Specific)
  string md5cmd = "md5sum " + fileDirectory + "/" + filename;
  FILE* pipe = popen(md5cmd.c_str(),"r");
  if(pipe != NULL)
  {
    unsigned int md5Size = 32;
    char md5Res[md5Size+1];
    md5Res[md5Size] = '\0';
    if(md5Size == fread((void*)md5Res,sizeof(char),md5Size,pipe))
      md5Val = md5Res;
    fclose(pipe);
  }
  cmd << md5Val << " ";

  //Determine and Add IP Address
  struct ifreq ifr;
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  ifr.ifr_addr.sa_family = AF_INET; //IPv4
  strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);
  cmd << inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr) << " ";

  //Add Port
  cmd << port << ">\n";

  return cmd.str().c_str();
}
