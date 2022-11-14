#include "diagnostic.h"

#include <sys/stat.h>
#include <sys/statvfs.h>
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "Log.h"

using namespace rapidjson;

char* Diagnostic::TAG="Diagnostic";

Diagnostic Diagnostic::s_diagnostic;

const double CARRY_COUNT = 1024.0;

double Diagnostic::GetCPUUsage()
{
    std::string cmd =
        "top -n 2 -d 1 -b | grep Cpu | cut -d \",\" -f 1 | cut -d \":\" -f 2 ";//指令
    std::string strused = "0.0";
    FILE *freport = popen(cmd.c_str(), "r");//file 的类型是 string ，popen 第一个参数要求是const char *， 如果想传 string 给 popen
    //需要使用成员函数c_str() 将string 转化成const char *
    //popen依照popen参数中的type值建立管道连接到子进程的输入/输出设备中，通过返回的指针对子进程 的输入、输出设备进行操作。
    if (freport == NULL)
    {
        LogE("Failed to get cpu used reporter.\n");//logerror
        return 0.0;
    }

    char linebuf[1024];

    //usleep(1000 * 1000 * 2);//usleep函数能把线程挂起一段时间， 单位是千分之一毫秒。本函数可暂时使程序停止执行。参数 micro_seconds 为要暂停的微秒数(us)。
    int index = 0;

    while (fgets(linebuf, 1023, freport) != NULL)//从指定流中读取数据
    {
        if (++index < 2)
        {
            continue;
        }

        char* token = strtok(linebuf, ", ");//定义一个指针变量token
        std::list<std::string> buflist;
        buflist.clear();
        while (token != NULL)
        {
            buflist.push_back(token);
            token = strtok(NULL, ", ");
        }

        std::string lastvalue = "";
        for (std::list<std::string>::iterator i = buflist.begin();
            i != buflist.end(); ++i)
        {
            std::size_t namepos = (*i).find("us");
            if (namepos != std::string::npos && lastvalue.size() > 0)
            {
                strused = lastvalue;
            }

            lastvalue = (*i);
        }
    }

    pclose(freport);

    return atof (strused.c_str());
}

long Diagnostic::GetMemTotal()
{
    std::string cmd = "top -n 1 -d 1 -b |grep \"KiB Mem\" | cut -d \":\" -f 2 ";
    long totalmem = 0;
    FILE *freport = popen(cmd.c_str(), "r");
    if (freport == NULL)
    {
        LogE("Failed to get memory used reporter.\n");
        return 0;
    }

    char linebuf[1024];
    //usleep(1000 * 1000 * 1);
    while (fgets(linebuf, 1023, freport) != NULL)
    {
        char* token = strtok(linebuf, ", ");
        std::list<std::string> buflist;
        buflist.clear();
        while (token != NULL)
        {
            buflist.push_back(token);
            token = strtok(NULL, ", ");
        }

        std::string lastvalue = "";
        for (std::list<std::string>::iterator i = buflist.begin();
            i != buflist.end(); ++i)
        {
            std::size_t namepos = (*i).find("total");
            if (namepos != std::string::npos && lastvalue.size() > 0)
            {
                totalmem = strtol(lastvalue.c_str(), NULL, 10);
                break;
            }

            lastvalue = (*i);
        }
    }

    pclose(freport);

    return totalmem;
}

double Diagnostic::GetMemUsage()
{
    std::string cmd = "top -n 1 -d 1 -b |grep \"KiB Mem\" | cut -d \":\" -f 2 &";
    long totalmem = 0;
    long usedmem = 0;
    FILE *freport = popen(cmd.c_str(), "r");
    if (freport == NULL)
    {
        LogE("Failed to get memory used reporter.\n");
        return 0.0;
    }

    char linebuf[1024];
    bool getsuccess = false;
    usleep(1000 * 1000 * 2);
    while (fgets(linebuf, 1023, freport) != NULL)
    {
        char* token = strtok(linebuf, ", ");
        std::list<std::string> buflist;
        buflist.clear();
        while (token != NULL)
        {
            buflist.push_back(token);
            token = strtok(NULL, ", ");
        }

        std::string lastvalue = "";
        for (std::list<std::string>::iterator i = buflist.begin();
            i != buflist.end(); ++i)
        {
            std::size_t namepos = (*i).find("total");
            if (namepos != std::string::npos && lastvalue.size() > 0)
            {
                totalmem = strtol(lastvalue.c_str(), NULL, 10);

                lastvalue = (*i);
                continue;
            }

            namepos = (*i).find("used");
            if (namepos != std::string::npos && lastvalue.size() > 0)
            {
                usedmem = strtol(lastvalue.c_str(), NULL, 10);
            }

            lastvalue = (*i);
        }

        if (totalmem > 0)
        {
            getsuccess = true;
            sprintf(linebuf, "%.1f", (usedmem * 100.0) / totalmem);
        }
    }

    pclose(freport);

    if (getsuccess)
        return atof (linebuf);
    else
        return atof ("0.0");
}

int Diagnostic::GetHDDUsage(vector<HDD_INFO>& vec_hdd_info)
{
    long usage = 0;
    std::string cmd =
        "df -k --output=source,used,avail | grep /dev/ | awk '{print $2,$3}'";
    std::string strused = "0.0";
    FILE *f_usage = popen(cmd.c_str(), "r");
    if (f_usage == NULL)
    {
        LogE("Failed to get hdd info reporter.\n");
        return 0.0;
    }

    char line_use[1024];
    int index = 0;
    while (fgets(line_use, 1023, f_usage) != NULL)
    {
        char* token = strtok(line_use, " ");
        std::vector<std::string> buflist;
        buflist.clear();
        while (token != NULL)
        {
            buflist.push_back(token);
            token = strtok(NULL, " ");
        }

        if (buflist.size() == 2)
        {
            HDD_INFO hdd_info;
            hdd_info.use = atol(buflist[0].c_str());
            hdd_info.total = atol(buflist[1].c_str()) + hdd_info.use;
            vec_hdd_info.push_back(hdd_info);
        }
    }

    pclose(f_usage);

    return vec_hdd_info.size();
}

long Diagnostic::GetSpaceSize(const std::string &path)
{
    long retSize = 0; //KB

    std::string cmd("du -sm ");
    cmd.append(path);

    FILE *freport = popen(cmd.c_str(), "r");
    if (freport == NULL)
    {
        return false;
    }

    char linebuf[1024];
    char buf[128];

    if (fgets(linebuf, 1023, freport) != NULL)
        sscanf(linebuf, "%ld %s", &retSize, buf);

    pclose(freport);

    return retSize;
}

void Diagnostic::GetSoftwareMC(double& mem_usage, double& cpu_usage)
{
    mem_usage = 0.0;
    cpu_usage = 0.0;
    std::string cmd =
        "top -n 5 -d 0.5 -b | grep media_server";
    std::string strused = "0.0";
    FILE *f_usage = popen(cmd.c_str(), "r");
    if (f_usage == NULL)
    {
        LogE("Failed to get software mem & cpu info reporter.\n");
        return;
    }
    sleep(3);
    char line_use[1024];
    double index = 0.0;
    bool ret = false;
    while (fgets(line_use, 1023, f_usage) != NULL)
    {
        char* token = strtok(line_use, " ");
        std::vector<std::string> buflist;
        buflist.clear();
        while (token != NULL)
        {
            buflist.push_back(token);
            token = strtok(NULL, " ");
        }

        if (buflist.size() == 12)
        {
            ret = true;
            cpu_usage += atof(buflist[8].c_str());
            mem_usage += atof(buflist[9].c_str());
            index += 1.0;
        }
    }

    pclose(f_usage);
    
    if (ret)
    {
        cpu_usage = cpu_usage / index;
        mem_usage = mem_usage / index;
    }

    cpu_usage=( (double)( (int)( (cpu_usage+0.005)*100 ) ) )/100;
    mem_usage=( (double)( (int)( (mem_usage+0.005)*100 ) ) )/100;
}

string Diagnostic::GetServerVersion()
{
	char result_buf[1024];

	FILE *fp = popen("cat /etc/redhat-release", "r"); //popen

	if ( NULL == fp)
	{
		printf("popen ServerVersion failure.\n");
		return "unknown";
	}
	printf("popen ServerVersion successfully!\n");//判断是否打开成功

	if (fgets(result_buf, (int)sizeof(result_buf), fp)==NULL)
	{
		printf("Read from fp failure:%s\n", strerror(errno));
		return "unknown";
	} //判断fread有没有读取成功

	printf("result_buf=%s\n", result_buf); //输出

	pclose(fp);    //关闭文件指针

	return result_buf;
}

string Diagnostic::GetCpuVersion()
{
	char result_buf[1024];

	FILE *fp = popen("cat /proc/cpuinfo|grep 'model name'", "r"); //popen

	if ( NULL == fp)
	{
		printf("popen CpuVersion failure.\n");
		return "unknown";
	}
	printf("popen CpuVersion successfully!\n");//判断是否打开成功

	if (fgets(result_buf, (int)sizeof(result_buf), fp)==NULL)
	{
		printf("Read from fp failure:%s\n", strerror(errno));
		return "unknown";
	} //判断fread有没有读取成功

	  std::string str=result_buf;
	  std::size_t pos = str.find(":");      // position of ":" in str
	  std::string str1 ;
	if (pos!=str.npos)
	{
		str1 = str.substr (pos+1);     // get from ":" to the end
		std::cout << str1 ;
	}
else{
	printf("not found!");
        }
	  pclose(fp);    //关闭文件指针
	  return str1;
}

