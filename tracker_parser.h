#ifndef TRACKER_PARSER_H
#define TRACKER_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <vector>
#include <ctime>
#include <iostream>
#include <fstream>
using namespace std;

#define FILE_SUCC 0
#define FILE_FAIL 1
#define FILE_ERR  2

struct HostInfo
{
  string ipaddr;
  int    port;
  long   startByte;
  long   endByte;
  long   timestamp;
};

class TrackerFile
{
 public:
  TrackerFile();

  bool parseTrackerFile(const char* trackerFileName);

  int update(const char* cmd);
  int create(const char* cmd);

  string getFilename();
  long getFilesize();
  string getDescription();
  string getMD5();

  const HostInfo& operator[](int i) const;
  int getNumHosts() const;

 private:
  string m_filename;
  long   m_filesize;
  string m_description;
  string m_md5;

  vector<HostInfo> m_hosts;

  template<class T>
  bool find(const char* key, const char* fileContents, T& value);
  bool parseHost(char* line);

};

template<class T>
void convert(string str, string& value)
{
  value = str;
}

template<class T>
void convert(string str, T& value)
{
  stringstream convert(str);
  convert >> value;
}


template<class T>
bool TrackerFile::find(const char* key, const char* fileContents, T& value)
{
  const char*  pos = strstr(fileContents,key);
  string       tmp = "";

  if(pos!=NULL)
  {
    pos += strlen(key);
    //Find assignment delimeter
    while(*pos!=':' && *pos!='=') pos++;
    pos++;

    //Skip white space
    while(*pos==' ' || *pos=='\t') pos++;

    //Read value
    while(*pos!='\n' && *pos!='\0' && *pos!='#')
    {
      tmp.push_back(*pos);
      pos++;
    }

    //Convert to type
    convert<T>(tmp,value);

    return true;
  }
  return false;
}
#endif
