#include "bj_file_info.h"
#include "tcp_msg_help.h"
#include <thread>
#include <algorithm>
#include "Log.h"
#include "FileSysUtils.h"
#include <bsd/md5.h>

bj_file_info::bj_file_info():Type(0)
{
    HasDownloaded = false;
    FailCount = 0;
}

int bj_file_info::Parse(const char *data)
{
    int index = 0;
    Url = TcpGetString(data, index, 128);
    FileName = TcpGetStringGB2312(data, index, 80);
    Type = TcpGetInt(data, index, 2);
    Md5 = TcpGetString(data, index, 32);
    transform(Md5.begin(),Md5.end(),Md5.begin(),::tolower);
    if(Md5.size()==31)
  {
    	Md5="0"+Md5;
  }
    return index;
}

int bj_file_info::Write(char *data)
{
    int index=0;
    TcpSetString(data, index, Url, 128);
    TcpSetString(data, index, FileName, 80);
    TcpSetInt(data, index, Type, 2);
    TcpSetString(data, index, FileName, 32);
    return index;
}
string bj_file_info::GetMd5(const string& filePath)
{
	char md5sum[MD5_DIGEST_STRING_LENGTH];
	char *sum = MD5File(filePath.c_str(), md5sum);
	if (sum != NULL)
	{
		return string(sum);
	}
	return string();
}
void bj_file_info::Download(const string& dst, function<void(bj_file_info *)> callback)
{
    if(dst.back() == '/')
    {
        LocalPath = dst + FileName;
    }
    else
    {
        LocalPath = dst;
    }

    thread t([callback](bj_file_info file_info)
    {
    	bj_file_info* _this=&file_info;
      if (FileSysUtils::Accessible(_this->LocalPath, FileSysUtils::FR_OK)&&GetMd5(_this->LocalPath)==_this->Md5)
        {
    	  _this->HasDownloaded = true;
    	  _this->FailCount = 0;
    	  callback(_this);
        	return;
        }
    	/*while(!_this->HasDownloaded&&_this->FailCount<3)//如果下载失败，最多重试3次
    	{
    		_this->DownloadInner();
    	}*/
      if(!_this->HasDownloaded)
    	  _this->DownloadInner();
    	callback(_this);
    },*this);
    t.detach();
}

const char* bj_file_info::TAG="bj_file_info";

void bj_file_info::DownloadInner()
{
	//ftp://tccpis:123@10.255.170.223/PlanFormat
	string ext="";
	string url="";

	string fileName=code_convent(FileName.c_str(),FileName.size(),"UTF-8","GB2312");
	char fileNameBuffer[fileName.size()*3+1];
	memset(fileNameBuffer,'\0',fileName.size()*3+1);
	for(size_t i=0,l=fileName.size();i<l;i++)
	{
		fileNameBuffer[i*3]='%';
		sprintf(fileNameBuffer+(i*3)+1,"%02X",(unsigned char)fileName.at(i));
	}

	if (Url.find('@') != string::npos)
	{
		int indexUser = Url.find("//") + 2;
		string userAndPwd = Url.substr(indexUser, Url.find('@', indexUser) - indexUser);
		url = string("ftp://") + Url.substr(Url.find('@', indexUser) + 1) + "/" + fileNameBuffer;
		ext="--ftp-user="+userAndPwd.substr(0,userAndPwd.find(':'))+" --ftp-password="+userAndPwd.substr(userAndPwd.find(':')+1);
	}
	else
	{
		url = Url + "/" + fileNameBuffer;
	}
	string cmd=string("wget -qO ") + LocalPath + " " + url +" "+ ext;
	printf("cmd=%s\n",cmd.c_str());
	FILE *fp = popen(cmd.c_str(),"r");
	pclose(fp);

	string md5=GetMd5(LocalPath);
	LogD("download file:%s,url:%s,md5:%s\n", FileName.c_str(), Url.c_str(), md5.c_str());
	if (md5 == Md5) //默认接口给的md5是32位小写字母
	{
		HasDownloaded = true;
		FailCount = 0;
	}
	else
	{
		LogE("download file md5 '%s'!='%s'\n", md5.c_str(), Md5.c_str());
		FailCount++;
	}
}
