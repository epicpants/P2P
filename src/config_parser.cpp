#include "config_parser.h"

ConfigFile::ConfigFile(const char* filename)
{
  m_fileContents = NULL;

  if(filename)
  {
    m_error = parse(filename);
  }
}

ConfigFile::~ConfigFile()
{
  if(m_fileContents)
  {
    delete m_fileContents;
  }
}

bool ConfigFile::parse(const char* filename)
{
  //Read file into m_fileContents
  FILE* cfgFile = NULL;
  long size     = 0;
  size_t length = 0;

  //Open file
  cfgFile = fopen(filename,"r");
  //string cfgFileNameStr = fileDirectory + "/" + filename;
  //FILE* cfgFile = fopen(cfgFileNameStr.c_str(),"r");

  //Read File
  if(cfgFile!=NULL)
  {
    if((fseek(cfgFile, 0L, SEEK_END) == 0) &&
      ((size = ftell(cfgFile)) != -1))
    {
      m_fileContents = new char[size+1];
      if(fseek(cfgFile, 0L, SEEK_SET) == 0)
      {
        length = fread(m_fileContents, sizeof(char), size, cfgFile);

        if(length != 0) m_fileContents[size] = '\0';
        else m_error = true;
      }
      else m_error = true;
    }
    else m_error = true;
    fclose(cfgFile);
  }
  else m_error = true;

  return m_error;
}

bool ConfigFile::is_open() const
{
  return m_error;
}
