
#include "IniFile.h"
#include <fstream>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#define MAX_INI_FILE_SIZE 1024*16
#pragma warning(disable:4244)
CIniFile::CIniFile()
{
	m_fileName = "";
}

CIniFile::~CIniFile(void)
{
}

const string & CIniFile::getFileName() const
{
	return m_fileName;
}

void CIniFile::setSection(const string &section)
{
	m_section = section;
}

bool CIniFile::write(const string &key, const string & value) const 
{
	return write_profile_string(m_section.c_str(),key.c_str(),value.c_str(),m_fileName.c_str())==1? true:false;
}

bool CIniFile::write(const string &key, int value) const 
{
	char tmp[64];
	sprintf(tmp,"%d",value);
	return write(key,tmp);
}

string CIniFile::readStr(const string &key,const string &default_value) const
{
	char buf[4096];
	read_profile_string(m_section.c_str(),key.c_str(),buf,sizeof(buf),default_value.c_str(),m_fileName.c_str());
	return buf;
}

int CIniFile::readInt(const string &key, int default_value) const
{
	return read_profile_int(m_section.c_str(),key.c_str(),default_value,m_fileName.c_str());
}

const string &CIniFile::getSection() const
{
	return m_section;
}

int CIniFile::load_ini_file(const char *file, char *buf,int *file_size)
{
	FILE *in = NULL;
	int i=0;
	*file_size =0;

	//assert(file !=NULL);
	//assert(buf !=NULL);

	in = fopen(file,"r");
	if( NULL == in) {
		return 0;
	}

	buf[i]=fgetc(in);

	//load initialization file
	while( buf[i]!= (char)EOF) {
		i++;
		//assert( i < MAX_INI_FILE_SIZE ); //file too big, you can redefine MAX_INI_FILE_SIZE to fit the big file 
		buf[i]=fgetc(in);
	}

	buf[i]='\0';
	*file_size = i;

	fclose(in);
	return 1;
}

int CIniFile::newline(char c)
{
	return ('\n' == c ||  '\r' == c )? 1 : 0;
}
int CIniFile::end_of_string(char c)
{
	return '\0'==c? 1 : 0;
}
int CIniFile::left_barce(char c)
{
	return '[' == c? 1 : 0;
}
int CIniFile::right_brace(char c )
{
	return ']' == c? 1 : 0;
}
int CIniFile::parse_file(const char *section, const char *key, const char *buf,int *sec_s,int *sec_e,
					  int *key_s,int *key_e, int *value_s, int *value_e)
{
	const char *p = buf;
	int i=0;

	//assert(buf!=NULL);
	//assert(section != NULL && strlen(section));
	//assert(key != NULL && strlen(key));

	*sec_e = *sec_s = *key_e = *key_s = *value_s = *value_e = -1;

	while( !end_of_string(p[i]) ) {
		//find the section
		if( ( 0==i ||  newline(p[i-1]) ) && left_barce(p[i]) )
		{
			int section_start=i+1;

			//find the ']'
			do {
				i++;
			} while( !right_brace(p[i]) && !end_of_string(p[i]));

			if( 0 == strncmp(p+section_start,section, i-section_start)) {
				int newline_start=0;

				i++;

				//Skip over space char after ']'
				while(isspace(p[i])) {
					i++;
				}

				//find the section
				*sec_s = section_start;
				*sec_e = i;

				while( ! (newline(p[i-1]) && left_barce(p[i])) 
					&& !end_of_string(p[i]) ) {
						int j=0;
						//get a new line
						newline_start = i;

						while( !newline(p[i]) &&  !end_of_string(p[i]) ) {
							i++;
						}

						//now i  is equal to end of the line
						j = newline_start;

						if(';' != p[j]) //skip over comment
						{
							while(j < i && p[j]!='=') {
								j++;
								if('=' == p[j]) {
									if(strncmp(key,p+newline_start,j-newline_start)==0)
									{
										//find the key ok
										*key_s = newline_start;
										*key_e = j-1;

										*value_s = j+1;
										*value_e = i;

										return 1;
									}
								}
							}
						}

						i++;
				}
			}
		}
		else
		{
			i++;
		}
	}
	return 0;
}

/**
*@brief read string in initialization file\n
* retrieves a string from the specified section in an initialization file
*@param section [in] name of the section containing the key name
*@param key [in] name of the key pairs to value 
*@param value [in] pointer to the buffer that receives the retrieved string
*@param size [in] size of result's buffer 
*@param default_value [in] default value of result
*@param file [in] path of the initialization file
*@return 1 : read success; \n 0 : read fail
*/
int CIniFile::read_profile_string( const char *section, const char *key,char *value, 
						int size, const char *default_value, const char *file)
{
	char buf[MAX_INI_FILE_SIZE]={0};
	int file_size;
	int sec_s,sec_e,key_s,key_e, value_s, value_e;

	//check parameters
	//assert(section != NULL && strlen(section));
	//assert(key != NULL && strlen(key));
	//assert(value != NULL);
	//assert(size > 0);
	//assert(file !=NULL &&strlen(key));

	if(!load_ini_file(file,buf,&file_size))
	{
		if(default_value!=NULL)
		{
			strncpy(value,default_value, size);
		}
		return 0;
	}

	if(!parse_file(section,key,buf,&sec_s,&sec_e,&key_s,&key_e,&value_s,&value_e))
	{
		if(default_value!=NULL)
		{
			strncpy(value,default_value, size);
		}
		return 0; //not find the key
	}
	else
	{
		int cpcount = value_e -value_s;

		if( size-1 < cpcount)
		{
			cpcount =  size-1;
		}

		memset(value, 0, size);
		memcpy(value,buf+value_s, cpcount );
		value[cpcount] = '\0';

		return 1;
	}
}

/**
*@brief read int value in initialization file\n
* retrieves int value from the specified section in an initialization file
*@param section [in] name of the section containing the key name
*@param key [in] name of the key pairs to value 
*@param default_value [in] default value of result
*@param file [in] path of the initialization file
*@return profile int value,if read fail, return default value
*/
int CIniFile::read_profile_int( const char *section, const char *key,int default_value, 
					 const char *file)
{
	char value[32] = {0};
	if(!read_profile_string(section,key,value, sizeof(value),NULL,file))
	{
		return default_value;
	}
	else
	{
		return atoi(value);
	}
}

/**
* @brief write a profile string to a ini file
* @param section [in] name of the section,can't be NULL and empty string
* @param key [in] name of the key pairs to value, can't be NULL and empty string
* @param value [in] profile string value
* @param file [in] path of ini file
* @return 1 : success\n 0 : failure
*/
int CIniFile::write_profile_string(const char *section, const char *key,
						 const char *value, const char *file)
{
	char buf[MAX_INI_FILE_SIZE]={0};
	char w_buf[MAX_INI_FILE_SIZE]={0};
	int sec_s,sec_e,key_s,key_e, value_s, value_e;
	int value_len = (int)strlen(value);
	int file_size;
	FILE *out;

	//check parameters
	//assert(section != NULL && strlen(section));
	//assert(key != NULL && strlen(key));
	//assert(value != NULL);
	//assert(file !=NULL &&strlen(key));

	if(!load_ini_file(file,buf,&file_size))
	{
		sec_s = -1;
	}
	else
	{
		parse_file(section,key,buf,&sec_s,&sec_e,&key_s,&key_e,&value_s,&value_e);
	}

	if( -1 == sec_s)
	{
		if(0==file_size)
		{
			sprintf(w_buf+file_size,"[%s]\n%s=%s\n",section,key,value);
		}
		else
		{
			//not find the section, then add the new section at end of the file
			memcpy(w_buf,buf,file_size);
			sprintf(w_buf+file_size,"\n[%s]\n%s=%s\n",section,key,value);
		}
	}
	else if(-1 == key_s)
	{
		//not find the key, then add the new key=value at end of the section
		memcpy(w_buf,buf,sec_e);
		sprintf(w_buf+sec_e,"%s=%s\n",key,value);
		sprintf(w_buf+sec_e+strlen(key)+strlen(value)+2,buf+sec_e, file_size - sec_e);
	}
	else
	{
		//update value with new value
		memcpy(w_buf,buf,value_s);
		memcpy(w_buf+value_s,value, value_len);
		memcpy(w_buf+value_s+value_len, buf+value_e, file_size - value_e);
	}

	out = fopen(file,"w");
	if(NULL == out)
	{
		return 0;
	}

	if(-1 == fputs(w_buf,out) )
	{
		fclose(out);
		return 0;
	}

	fclose(out);
	return 1;
}

