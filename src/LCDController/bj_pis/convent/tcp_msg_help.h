#ifndef TCP_MSG_HELP_H_
#define TCP_MSG_HELP_H_

#include <string>
#include "bj_pis/utils/datetime.h"
#include <cstring>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <exception>
#include <iconv.h>
#include <errno.h>
#include <iostream>

static std::string TcpGetString(const char* data,int& index,int len)
{
	std::string s = std::string(data + index,len);
	index += len;
	if (!s.empty()) 
    {
    	s.erase(s.find_last_not_of(' ') + 1);
    }
	return s;
}

static string code_convent(const char* from_str,unsigned long from_len,const string& from_charset,const string& to_charset)
{
	iconv_t cd;
	cd = iconv_open(to_charset.c_str(),from_charset.c_str());
	if(cd==(iconv_t)-1) throw runtime_error("iconv_open error");

	char tempInput[from_len];
	memcpy(tempInput,from_str,from_len);
	/*printf("input:\n");
	for (unsigned long i = 0; i < from_len; ++i) {
		printf(" %02x",(unsigned char)tempInput[i]);
		if(tempInput[i]=='\r' || tempInput[i]=='\n')
		{
			tempInput[i]=' ';
		}
	}
	FILE* fd = fopen("ttt.txt","wb");
	fwrite(tempInput,sizeof(char),from_len,fd);
	fclose(fd);

	printf("\n");*/
	char* pInput=tempInput;
	unsigned long inputLen=from_len;

	unsigned long outlen=from_len*4;
	char output[outlen];
	char* pOutput=output;
	int ret = iconv(cd,&pInput,&inputLen,&pOutput,&outlen);
	if(ret==-1)
	{
		printf("----%d----%s----\n",errno,strerror(errno));
		iconv_close(cd);
		throw runtime_error("iconv error");
	}
	else
	{
		iconv_close(cd);
	}
	/*printf("output:\n");
	for (unsigned long i = 0,l=from_len*4-outlen; i < l; ++i) {
		printf(" %02x",(unsigned char)output[i]);
	}
	printf("\n");*/
	string result(output,from_len*4-outlen);
	//printf("------convent from from:%s, to:%s,result:%s!!__\n",from_charset.c_str(),to_charset.c_str(),result.c_str());
	return result;
}

static string TcpGetStringUnicode(const char* data,int& index,int len)
{
	std::string s = code_convent(data+index,len,"UCS-2","UTF-8");
	index += len;
	if (!s.empty())
	{
		s.erase(s.find_last_not_of(' ') + 1);
	}
	return s;
}

static string TcpGetStringGB2312(const char* data,int& index,int len)
{
	std::string s = code_convent(data+index,len,"GB2312","UTF-8");
	index += len;
	if (!s.empty())
	{
		s.erase(s.find_last_not_of(' ') + 1);
	}
	return s;
}

static string TcpGetStringUTF16(const char* data,int& index,int len)
{
	string s=code_convent(data+index,len,"UTF-16LE","UTF-8");
	index += len;
	if (!s.empty())
	{
		s.erase(s.find_last_not_of(' ') + 1);
	}
	return s;
}

static time_t TcpGetDateTime(const char* data,int& index,int len)
{
	std::string s = std::string(data + index,len);
	index += len;
	return datetime::Parse(s,"%Y%m%d%H%M%S").get_time();
}

static time_t TcpGetDateTime(const char* data,int& index)
{
	return TcpGetDateTime(data,index,14);
}

static int TcpGetInt(const char* data,int& index,int len)
{
	std::string s = std::string(data + index,len);
	index += len;
    int ret=0;
    try
    {
        ret=std::stoi(s);
    }
    catch(...)
    {      
        (std::cout<<"stoi err:").write(data+index-len,len)<<"__"<<std::endl;
        printf("index:%d,len:%d\n",index,len);
        printf("%s(0): %p\n", __func__, __builtin_return_address(0));
        throw;
    }
    
	return ret;
}

static void TcpSetString(char* data,int& index,const std::string& s,int len)
{
    int l = s.size();
    if(l > len)
    {
        throw runtime_error(s + ".size() > len, " + std::to_string(len));
    }
	strcpy(data+index, s.c_str());
    if(len - l > 0)
    {
        memset(data+index+l, 32, len - l); //32是空格的ascii码
    }
	index+=len;
}
static void TcpSetStringGB2312(char* data,int& index,const std::string& s,int len)
{
	string gbStr=code_convent(s.c_str(),s.size(),"UTF-8","GB2312");
	int l = gbStr.size();
	if(l > len)
	{
		throw runtime_error(s + ".size() > len, " + std::to_string(len));
	}
	strcpy(data+index, gbStr.c_str());
	if(len - l > 0)
	{
		memset(data+index+l, 32, len - l); //32是空格的ascii码
	}
	index+=len;
}

static void TcpSetDateTime(char* data,int& index,time_t t,int len)
{
	memcpy(data+index,datetime::from_time(t).ToString("%Y%m%d%H%M%S").c_str(),len);
	index+=len;
}

static void TcpSetDateTime(char* data,int& index,time_t t)
{
	TcpSetDateTime(data,index,t,14);
}


static void TcpSetInt(char* data,int& index,int i,int len)
{
	sprintf(data+index, (std::string("%0")+std::to_string(len)+"d").c_str(), i);
	index+=len;
}

static void GetFiles(string dir_path,std::vector<std::string>& result,bool recursive=false)
{
    struct dirent *ptr;    
    DIR *dir;
    if(dir_path.back()=='/')dir_path.pop_back();
    dir=opendir(dir_path.c_str());
    if(!dir) return;
    struct stat sb;
    while((ptr=readdir(dir))!=NULL)
    {
        if(ptr->d_name[0] == '.')
            continue;
        stat(ptr->d_name, &sb);
        if(recursive&&S_ISDIR(sb.st_mode))//dir
        {
            GetFiles(dir_path+'/'+ptr->d_name,result,recursive);
        }
        else//file
        {
            result.push_back(dir_path +'/'+ ptr->d_name);
        }
    }
    closedir(dir);
}

static string GetLatestFile(string dir_path)
{
	struct dirent *ptr;    
    DIR *dir;
    if(dir_path.back()=='/')dir_path.pop_back();
    dir=opendir(dir_path.c_str());
    if(!dir) return "";
    string result;
    struct stat sb;
    time_t t = 0;
    while((ptr=readdir(dir))!=NULL)
    {
        if(ptr->d_name[0] == '.')
            continue;
        stat(ptr->d_name, &sb);
        if(!S_ISDIR(sb.st_mode))//file
        {
        	if(sb.st_mtime > t)
        	{
        		result = ptr->d_name;
        	}
        }
    }
    closedir(dir);
    return result;
}

static string ReadTextFile(const string& path)
{
	FILE* fd=fopen(path.c_str(),"r");
	if(fd==NULL) throw runtime_error(string("can't find file ")+path);
	fseek(fd, 0, SEEK_END);
	long int size=ftell(fd);
	char* buffer=new char[size];//为什么不用临时变量，临时变量数组长度比较小，只有1-2M
	fseek(fd, 0, SEEK_SET);
	fread(buffer,size,1,fd);
	fclose(fd);
	string s(buffer,size);
	delete[] buffer;
	return s;
}


#endif // TCP_MSG_HELP_H_
