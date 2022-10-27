#include "FileSysUtils.h"
#include "SystemClock.h"
#include "Log.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/file.h>
#include <iostream>

FileSysUtils::DiskSpaceReport::~DiskSpaceReport()
{
    for (std::list<DiskSpaceReportInner *>::iterator i = mReport.begin();
            i != mReport.end(); i++)
    {
        delete (*i);
    }
}

void FileSysUtils::DiskSpaceReport::print()
{
    if (mReport.empty() == false)
    {
        for (std::list<DiskSpaceReportInner *>::iterator i = mReport.begin();
                i != mReport.end(); i++)
        {
            LogD(
                    "%-24s[%-12s] : Used = %10u Available = %10u : Total = %10u --- %3u%%\n",
                    (*i)->mMounted.c_str(), (*i)->mFilesystem.c_str(),
                    (*i)->mUsed, (*i)->mAvailable, (*i)->mTotal,
                    (*i)->mUsedPcg);
        }

        LogD("Total : Used = %u Available = %u : Total = %u --- %u%%\n", mUsed,
                mAvailable, mTotal, mUsedPcg);
    }
}

bool FileSysUtils::Accessible(const std::string &file, int mode)
{
    return access(file.c_str(), mode) == 0;
}

int FileSysUtils::MakeDir(const std::string &dir)
{
#if 0
    return mkdir(dir.c_str(), 0775) == 0;
#else
    int ret = -1;

    size_t si = 0, dsi = 0;
    std::string sd;
    while ((dsi = dir.find('/', si)) != std::string::npos)
    {
        sd = dir.substr(0, dsi).c_str();

        if (*sd.rbegin() != '/' && sd != ".")
        {
            ret = mkdir(sd.c_str(), 0775);
        }

        si = dsi + 1;
    }

    if (dir != ".")
    {
        ret = mkdir(dir.c_str(), 0775);
    }

    return ret;
#endif
}

int FileSysUtils::NewFile(const std::string &file)
{
    int fd = open(file.c_str(), O_RDWR | O_CREAT | O_TRUNC,
    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd > 0)
    {
        close(fd);
    } else
    {
        LogE("New File %s Failed.\n");
    }

    return fd;
}

std::string FileSysUtils::GetCurrPath()
{
    int nSize = 1024;
    char path[nSize];
    memset(path, nSize, 0x0);
    if (getcwd(path, nSize) != NULL)
        return path;
    else
        return "";
}

int FileSysUtils::ScanDirFiles(const std::string &dirpath,
        std::list<std::string> &list, bool recursive)
{
    DIR *dir = opendir(dirpath.c_str());
    if (dir == NULL)
    {
        LogD("Could not open %s, %s\n", dirpath.c_str(), strerror(errno));
        return -1;
    }

    int counter = 0;
    for (struct dirent *de = readdir(dir); de != NULL; de = readdir(dir))
    {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
        {
            if (de->d_type == DT_DIR)
            {
                if (recursive == true)
                {
                    counter += ScanDirFiles(dirpath + "/" + de->d_name, list,
                            recursive);
                }
            } else
            {
                list.push_back(dirpath + "/" + de->d_name);

                counter++;
            }
        }
    }

    if (dir != NULL)
        closedir(dir);

    return counter;
}

int FileSysUtils::HouseKeeping(const std::string &dir, unsigned outdated)
{
    int counter = -1;

    std::list<std::string> list;
    if (ScanDirFiles(dir, list, true) >= 0)
    {
        counter = 0;

        struct stat filestat;
        for (std::list<std::string>::iterator i = list.begin(); i != list.end();
                i++)
        {
            stat((*i).c_str(), &filestat);

            uint64_t passed = SystemClock::EpochSecPassed(filestat.st_mtime);
            struct tm *tmCurrtime = SystemClock::CurrentDateTime();

            if (passed
                    > (outdated - 1) * 24 * 60 * 60
                            + (tmCurrtime->tm_hour * 60 * 60)
                            + (tmCurrtime->tm_min * 60) + tmCurrtime->tm_sec)
            {

                LogI("File %s outdated. Deleted it...\n", (*i).c_str());

                unlink((*i).c_str());
                counter++;
            }
        }
    }

    return counter;
}

int FileSysUtils::HouseKeepingByCount(const std::string &dir, unsigned count)
{
	int counter = -1;

	std::list<std::string> list;
	if (ScanDirFiles(dir, list, true) >= 0)
	{
		counter = 0;

		struct stat filestat;
		for (std::list<std::string>::iterator i = list.begin(); i != list.end();
				i++)
		{

			if (list.size() > count)
			{
				LogI("File number - %d > %d. Deleted it...\n", list.size(),count);

				i = list.erase(i);

				unlink((*i).c_str());
				counter++;
			}
			else
			{
				break;
			}
		}
	}

	return counter;
}

int FileSysUtils::DiskSpaceReporter(const std::string &path,
        DiskSpaceReport &report)
{
    std::string cmd = "df --total ";
    //cmd.append(path);

    FILE *freport = popen(cmd.c_str(), "r");
    if (freport == NULL)
    {
        LogW("Failed to get disk space reporter.\n");
        return -1;
    }

    char linebuf[1024];
    char filesystem[256];
    char mounted[128];
    unsigned total;
    unsigned used;
    unsigned available;
    unsigned usedpcg;

    while (fgets(linebuf, sizeof(linebuf), freport) != NULL)
    {
        filesystem[0] = 0;
        mounted[0] = 0;

        int matched = sscanf(linebuf, "%s %u %u %u %u%% %s", filesystem, &total,
                &used, &available, &usedpcg, mounted);
        if (matched == 6)
        {
            if (strcmp(mounted, "-") == 0)
            {
                report.mTotal = total;
                report.mUsed = used;
                report.mAvailable = available;
                report.mUsedPcg = usedpcg;
            } else
            {
                DiskSpaceReportInner *inner = new DiskSpaceReportInner();
                inner->mFilesystem = filesystem;
                inner->mMounted = mounted;
                inner->mTotal = total;
                inner->mUsed = used;
                inner->mAvailable = available;
                inner->mUsedPcg = usedpcg;

                report.mReport.push_back(inner);
            }
        }
    }

    pclose(freport);

    return report.mUsedPcg;
}

std::string FileSysUtils::CpuUsedReporter()
{
    std::string cmd =
            "top -n 2 -d 1 -b |grep Cpu | cut -d \",\" -f 1 | cut -d \":\" -f 2 &";
    std::string strused = "";
    FILE *freport = popen(cmd.c_str(), "r");
    if (freport == NULL)
    {
        LogE("Failed to get cpu used reporter.\n");
        return strused;
    }

    char linebuf[1024];

    usleep(1000 * 1000 * 2);
    int index = 0;

    while (fgets(linebuf, sizeof(linebuf), freport) != NULL)
    {

        //LogE("##########  get cpu used: %s\n",linebuf);
        if (++index < 2)
        {
            continue;
        }

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
            std::size_t namepos = (*i).find("us");
            if (namepos != std::string::npos && lastvalue.size() > 0)
            {
                strused = lastvalue;
            }

            lastvalue = (*i);
        }
    }

    pclose(freport);

    return strused;
}

long FileSysUtils::MemTotal()
{
    std::string cmd = "top -n 1 -d 1 -b |grep \"KiB Mem\" | cut -d \":\" -f 2 &";
    long totalmem = 0;
    FILE *freport = popen(cmd.c_str(), "r");
    if (freport == NULL)
    {
        LogW("Failed to get memory used reporter.\n");
        return 0;
    }

    char linebuf[1024];
    usleep(1000 * 1000 * 1);
    while (fgets(linebuf, sizeof(linebuf), freport) != NULL)
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

long FileSysUtils::GetSpaceSize(const std::string &path)
{
    long retSize = 0; //KB

    std::string cmd("du -s -k ");
    cmd.append(path);

    FILE *freport = popen(cmd.c_str(), "r");
    if (freport == NULL)
    {
        return false;
    }

    char linebuf[1024];
    char buf[128];

    if (fgets(linebuf, sizeof(linebuf), freport) != NULL)
        sscanf(linebuf, "%ld %s", &retSize, buf);

    pclose(freport);
    return retSize;
}

std::string FileSysUtils::MemUsedReporter()
{
    std::string cmd = "top -n 1 -d 1 -b |grep \"KiB Mem\" | cut -d \":\" -f 2 &";
    long totalmem = 0;
    long usedmem = 0;
    FILE *freport = popen(cmd.c_str(), "r");
    if (freport == NULL)
    {
        LogW("Failed to get memory used reporter.\n");
        return " ";
    }

    char linebuf[1024];
    bool getsuccess = false;
    usleep(1000 * 1000 * 2);
    while (fgets(linebuf, sizeof(linebuf), freport) != NULL)
    {

        //LogE("##########  get memory used: %s\n",linebuf);

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
        return linebuf;
    else
        return "0";
}

std::string FileSysUtils::Path2Dir(const std::string &path)
{
    std::size_t namepos = path.find_last_of('/');
    if (namepos != std::string::npos)
    {
        return path.substr(0, namepos + 1);
    } else
    {
        return path;
    }
}

std::string FileSysUtils::Path2File(const std::string &path)
{
    std::size_t namepos = path.find_last_of('/');
    if (namepos != std::string::npos)
    {
        return path.substr(namepos + 1);
    } else
    {
        return path;
    }
}

bool FileSysUtils::checkProcessOpened(const std::string name)
{
    bool isopened = false;

    std::string lockfilename = "";
    lockfilename.append("./.");
    lockfilename.append(name);
    lockfilename.append("_proc.lock");

    int lock_file = open(lockfilename.c_str(), O_CREAT | O_RDWR, 0666);
    int rc = flock(lock_file, LOCK_EX | LOCK_NB);
    if (rc)
    {
        if (EWOULDBLOCK == errno)
        {
            isopened = true;
        }
    }

    return isopened;
}

const int FileSysUtils::FR_OK = R_OK;
const int FileSysUtils::FW_OK = W_OK;
const int FileSysUtils::FX_OK = X_OK;
const int FileSysUtils::FE_OK = F_OK;

const char *FileSysUtils::TAG = "FileSysUtils";
