#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <string>
#include <string.h>
#include <sstream>
#include <fstream>
#include <iostream>
using namespace std;

class ConfigFile
{
 public:
  ConfigFile(const char* filename = NULL);
  ~ConfigFile();

  bool parse(const char* filename);
  bool is_open() const;

  template<class T>
  bool find(const char* key, T& value);

 private:
  char* m_fileContents;
  bool m_error;
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
bool ConfigFile::find(const char* key, T& value)
{
  const char*  pos = strstr(m_fileContents,key);
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
