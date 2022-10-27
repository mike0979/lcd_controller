#pragma once
#include <string>
#include <vector>
#include <boost/thread.hpp>
using std::vector;
using std::string;

typedef struct CPU_OCCUPY         //定义一个cpu occupy的结构体
{
    char name[20];      //定义一个char类型的数组名name有20个元素
    unsigned int user; //定义一个无符号的int类型的user
    unsigned int nice; //定义一个无符号的int类型的nice
    unsigned int system;//定义一个无符号的int类型的system
    unsigned int idle; //定义一个无符号的int类型的idle
}CPU_OCCUPY;

typedef struct MEM_OCCUPY         //定义一个mem occupy的结构体
{
    char name[20];      //定义一个char类型的数组名name有20个元素
    unsigned long total; 
    char name2[20];
    unsigned long free;                       
}MEM_OCCUPY;

typedef struct HDD_INFO
{
    string name;
    long total;
    long use;
}HDD_INFO;

class Diagnostic
{
public:
    static Diagnostic & Instance() { return s_diagnostic; }
    static Diagnostic& GetObjecter() { return s_diagnostic; }
    void InitDiagnostic();
    void FinitDiagnostic();
public:
    Diagnostic(void){};
    ~Diagnostic(void){};


    long GetMemTotal();
    double GetMemUsage();
    int GetHDDUsage(vector<HDD_INFO>& vec_hdd_info);
    double GetCPUUsage();
    long GetSpaceSize(const std::string &path);
    void GetSoftwareMC(double& mem_usage, double& cpu_usage);
    string GetServerVersion();
    string GetCpuVersion();


public:
    static Diagnostic s_diagnostic;
    static char* TAG;

private:
};

