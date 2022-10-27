/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : main.cpp
* @author : wangfuqiang
* @date : 2017/8/8 14:29
* @brief : main process
*/
#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include "Looper.h"
#include "LCDController.h"
#include "CommonDef.h"
#include "bj_pis/thread/tcp_server_thread.h"
#include "bj_pis/ats/task_get_ats.h"

int MAIN_ARGC;
char **MAIN_ARGV;

bool checkProcessOpened(const std::string name)
{
    bool isopened = false;

    std::string lockfilename="";
    lockfilename.append("./.");
    lockfilename.append(name);
    lockfilename.append("_proc.lock");

    int lock_file = open(lockfilename.c_str(),O_CREAT|O_RDWR,0666);
    int rc = flock(lock_file,LOCK_EX|LOCK_NB);
    if(rc)
    {
        if(EWOULDBLOCK == errno)
        {
            isopened = true;
        }
    }

    return isopened;
}

int main(int argc, char *argv[]) {

    MAIN_ARGC = argc;
    MAIN_ARGV = argv;

    if(checkProcessOpened(MODULEPROCESSNAME))
    {
        std::string name = MODULEPROCESSNAME;
        printf("Process-%s had been opened.\n",name.c_str());
        return 1;
    }

    TcpServerThread tcp_server_thread("", 7879);
    tcp_server_thread.Start();
    task_get_ats::GetInstance().Start();

    Looper *mainLooper = Looper::CreateLooper(true);

    LCDController controller;

    mainLooper->loop();

    task_get_ats::GetInstance().Stop();
    tcp_server_thread.Dispose();

    return 0;
}
//
