#ifndef BJ_FILE_INFO_H_
#define BJ_FILE_INFO_H_

#include <string>
#include <functional>

using namespace std;

class bj_file_info
{
public:	
	bj_file_info();
	string Url;
	string FileName;
	int Type;//1图片2视频3文本4其它
	string Md5;

	int Parse(const char* data);
	int Write(char* data);

	/* 下载文件
	   dst:目的地，可以是目录，也可以是文件名
	   callback：回调函数，发生在成功或失败之后。成功代表下载完成并MD5检测通过(HasDownloaded==true)，其它为失败
	   函数调用后，立即可以获取LocalPath用于插库等操作，不需要等待下载完成。
	*/
	void Download(const string& dst,function<void(bj_file_info*)> callback);
	static string GetMd5(const string& filePath);
	bool HasDownloaded;//是否下载成功
	string LocalPath;//文件本地路径
	static const char* TAG;

private:
	void DownloadInner();
	int FailCount;
};

#endif // BJ_FILE_INFO_H_
