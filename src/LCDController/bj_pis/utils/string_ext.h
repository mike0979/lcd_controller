#pragma once
#include <string>
#include <vector>
#include <cstring>
using namespace std;

static vector<string> string_split(const string& source,const string& delimiter)
{
	int l_source=source.size();
	int l_delimiter=delimiter.size();
	const char* cs_source=source.c_str();
	const char* cs_delimiter=delimiter.c_str();
	std::vector<string> result;
	result.reserve(64);//预分配一个容量，避免在容量不够时重新分配内存，如果结果>64，也还是要重新分配的
	int start=0,i=0;
	for(;i<l_source;i++)
	{
		int j=0;
        for(;j<l_delimiter;j++)
        {
            if(cs_source[i+j]!=cs_delimiter[j])
            {
                break;
            }
        }
        if(j==l_delimiter)
        {
        	result.push_back(std::move(string(cs_source+start,i-start)));
        	i+=j-1;
        	start=i+1;
        }
	}
	if(i-start>0)
	{
		result.push_back(std::move(string(cs_source+start,i-start)));
	}
	return result;
}

static string string_replace(const string& source,const string& pattern,const string& replacement,int start=0)
{
	int l_source=source.size();
    char temp[l_source+1];
    memset(temp, '\0', l_source+1);

    int l_pattern=pattern.size();
    int l_replacement=replacement.size();
    const char* cs_source=source.c_str();
    const char* cs_pattern=pattern.c_str();
    const char* cs_replacement=replacement.c_str();
    int i_temp=start;
    for(int i=start;i<l_source;i++)
    {
    	int j=0;
        for(;j<l_pattern;j++)
        {
            if(cs_source[i+j]!=cs_pattern[j])
            {
                break;
            }
        }
        if(j==l_pattern)
        {
            memcpy(temp+i_temp,cs_replacement,l_replacement);
            i_temp+=l_replacement;
            i+=j-1;
        }
        else
        {
            temp[i_temp++]=cs_source[i];
        }
    }
    return string(temp);
}

static bool string_start_with(const string& str,const string& head)
{
    if(str.size()<head.size()) return false;
    const char* cs_str=str.c_str();
    const char* cs_head=head.c_str();
    for(int i=0,l=head.size();i<l;i++)
    {
        if(cs_head[i]!=cs_str[i])
        {
            return false;
        }
    }
    
    return true;
}

static bool string_end_with(const string& str,const string& tail)
{
    int start_index=str.size()-tail.size();
    if(start_index<0) return false;
    const char* cs_str=str.c_str()+start_index;
    const char* cs_tail=tail.c_str();
    for(int i=0,l=tail.size();i<l;i++)
    {
        if(cs_tail[i]!=cs_str[i])
        {
            return false;
        }
    }
    
    return true;
}

static void string_trim(char* s)
{
    int len=strlen(s);
    if(len==0)return;
    int start=-1,end=-1;
    for (int i = 0; i < len; ++i)
    {
        if(s[i]!=' ')
        {
            if(start<0)
            {start=i;}
            end=i;
        }
    }
    if(start<0) s[0]='\0';
    int resultlen=end-start+1;
    char temp[resultlen+1]={0};
    strncpy(temp,s+start,resultlen);
    temp[resultlen]='\0';
    strcpy(s,temp);
}

static void string_append(char* s,const char* content)
{
    int len = strlen(s);
    char* ss = s + len;
    strcpy(ss,content);
}

/* 参考资料:https://www.cnblogs.com/chenwenbiao/archive/2011/08/11/2134503.html
   下表总结了编码规则，字母x表示可用编码的位。

    Unicode符号范围 | UTF-8编码方式
    (十六进制) | （二进制）
    --------------------+---------------------------------------------
    0000 0000-0000 007F | 0xxxxxxx
    0000 0080-0000 07FF | 110xxxxx 10xxxxxx
    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
    0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

下面，还是以汉字“严”为例，演示如何实现UTF-8编码。
已知“严”的unicode是4E25（100111000100101），根据上表，可以发现4E25处在第三行的范围内（0000 0800-0000 FFFF），
因此“严”的UTF-8编码需要三个字节，即格式是“1110xxxx 10xxxxxx 10xxxxxx”。然后，从“严”的最后一个二进制位开始，
依次从后向前填入格式中的x，多出的位补0。这样就得到了，“严”的UTF-8编码是“11100100 10111000 10100101”，
转换成十六进制就是E4B8A5。
*/
//只适用于code文件是utf8的情况,[\u4e00-\u9fa5]
static void string_find_chs(const string& str, vector<string>& result)
{
    const char* ss= str.c_str();
    int start=-1;
    int end=-1;
    for (int i = 0,l=str.size(); i < l-2; ++i)
    {
        if((ss[i]&0xF0)==0XE0&&(ss[i+1]&0xC0)==0X80&&(ss[i+2]&0xC0)==0X80)
        {
            ushort x = ((ss[i]&0x0F)<<12)+((ss[i+1]&0x3F)<<6)+((ss[i+2]&0x3F));//x是Unicode值
            if(x>=0x4E00&&x<=0x9FA5)
            {
                if(start==-1)
                {
                    start=i;
                }
                end=i+3;            
                i+=2;
                continue;
            }
        }
        if(start!=-1)
        {
            result.push_back(string(ss+start,end-start));
            start=end=-1;
        }
    }
    if(start!=-1)
    {
        result.push_back(string(ss+start,end-start));
    }
}

//获取字符串第一个字符的长度，仅供utf-8，原理参考string_find_chs的注释
static int get_single_length(const string& str,size_t pos=0)
{
    size_t l=str.size()-pos;
    if(l==0)
        return 0;
    if((uint8_t)str.at(pos)>>7==0) return 1;
    if(l>=2&&(uint8_t)str.at(pos)>>5==6&&(uint8_t)str.at(pos+1)>>6==2)
    {
        return 2;
    }
    if(l>=3&&(uint8_t)str.at(pos)>>4==14&&(uint8_t)str.at(pos+1)>>6==2&&(uint8_t)str.at(pos+2)>>6==2)
    {
        return 3;
    }
    if(l>=4&&(uint8_t)str.at(pos)>>3==30&&(uint8_t)str.at(pos+1)>>6==2&&(uint8_t)str.at(pos+2)>>6==2&&(uint8_t)str.at(pos+3)>>6==2)
    {
        return 4;
    }

    return 1;
}

//查找delimiter中所有字符,应用场景string_split_any("ab,cd!你好", ",!")
static vector<string> string_split_any(const string& source,const string& delimiter)
{
    std::vector<string> strs;
    strs.reserve(8);
    size_t l=source.size();
    if(l>0)
    {
        for(size_t i=0;i<l;i++)
        {
            size_t found = source.find_first_of(delimiter, i);
            if(found != string::npos)
            {
                strs.push_back(source.substr(i,found-i));
                i=found;
                i+=get_single_length(source,i)-1;
            }
            else
            {
                strs.push_back(source.substr(i));
                i=l;
            }
        }
    }
    return strs;
}
