#include "datetime.h"
#include <cstring>

datetime datetime::GetDateTime(int year, int month, int day)
{
    if(!datetime::IsLegal(year, month, day))
    {
        throw runtime_error("result is invalid.");
    }
    datetime r;
    r.m_tm.tm_year = year - 1900;
    r.m_tm.tm_mon = month - 1;
    r.m_tm.tm_mday = day;
    return r;
};

datetime datetime::Parse(const string& timeStr, const string& format)
{
    if(format.size()>0)
    {   
        datetime dt;
        strptime(timeStr.c_str(),format.c_str(),&(dt.m_tm));
        return dt;
    }
    int l=timeStr.size();
    if(l<8)
    {
        throw runtime_error("timeStr is short, at least 8.");
    }
    int timeLenArray[6]={4,2,2,2,2,2};
    int timeArray[6]={0,0,0,0,0,0};
    char * cstr = new char [5];
    int stri=0,arri=0;
    const char * timeC = timeStr.c_str();
    while(stri<l)
    {
        if(timeC[stri]>='0'&&timeC[stri]<='9')
        {
            memcpy(cstr, timeC+stri,timeLenArray[arri]);
            stri+=timeLenArray[arri];
            cstr[timeLenArray[arri]]='\0';
            timeArray[arri++] = atoi(cstr);
        }
        else
        {
            stri++;
        }
    }
    delete[] cstr;

    if(!datetime::IsLegal(timeArray[0], timeArray[1], timeArray[2])||timeArray[3]>23||timeArray[4]>59||timeArray[5]>59)
    {
        throw runtime_error("timeStr \""+timeStr+"\" isn't a valid time.");
    }

    datetime r;
    r.m_tm.tm_year = timeArray[0] - 1900;
    r.m_tm.tm_mon = timeArray[1] - 1;
    r.m_tm.tm_mday = timeArray[2];
    r.m_tm.tm_hour = timeArray[3];
    r.m_tm.tm_min = timeArray[4];
    r.m_tm.tm_sec = timeArray[5];
    return r;
}

datetime datetime::Now()
{
    datetime r;
    time_t t = time(0);

#ifdef _WIN32
    localtime_s(&r.m_tm, &t);
#else
    localtime_r(&t, &r.m_tm);
#endif

    return r;
};

datetime datetime::AddSeconds(int i) const
{
    datetime r;
    r.m_tm = m_tm;
    time_t t = mktime(&r.m_tm);
    long seconds = t;
    seconds += i;
    t = seconds;
#ifdef _WIN32
    localtime_s(&r.m_tm, &t);
#else
    localtime_r(&t, &r.m_tm);
#endif
    return r;
};
datetime datetime::AddMinutes(int i) const
{
    return AddSeconds(i * 60);
};
datetime datetime::AddHours(int i) const
{
    return AddSeconds(i * 3600);
};
datetime datetime::AddDays(int i) const
{
    return AddSeconds(i * 86400);
};
datetime datetime::AddMonths(int i) const
{
    datetime r;
    r.m_tm = m_tm;
    r.m_tm.tm_mon += i;
    while(r.m_tm.tm_mon > 11)
    {
        r.m_tm.tm_mon -= 12;
        r.m_tm.tm_year++;
    }
    while(r.m_tm.tm_mon < 0)
    {
        r.m_tm.tm_mon += 12;
        r.m_tm.tm_year--;
    }
    if(!datetime::IsLegal(r.GetYear(), r.GetMonth(), r.GetDay()))
    {
        throw runtime_error("result is invalid.");
    }

    return r;
};
datetime datetime::AddYears(int i) const
{
    datetime r;
    r.m_tm = m_tm;
    r.m_tm.tm_year += i;
    return r;
};

int datetime::GetYear() const
{
    return 1900 + m_tm.tm_year;
};
int datetime::GetMonth() const
{
    return m_tm.tm_mon + 1;
};
int datetime::GetDay() const
{
    return m_tm.tm_mday;
};
int datetime::GetHour() const
{
    return m_tm.tm_hour;
};
int datetime::GetMinute() const
{
    return m_tm.tm_min;
};
int datetime::GetSecond() const
{
    return m_tm.tm_sec;
};
int datetime::GetWeek() const
{
    return m_tm.tm_wday;
};
string datetime::ToString(const string& format) const
{
    char str[50];
    strftime(str,50,format.c_str(),&m_tm);
    return string(str);
};

bool datetime::operator==(datetime &dt)
{
    return mktime(&m_tm) == mktime(&(dt.m_tm));
};

datetime& datetime::operator=(const datetime &dt)
{
    m_tm = dt.m_tm;
    return *this;
};

time_t datetime::operator-(datetime &dt)
{
    time_t this_t = mktime(&m_tm);
    time_t arg_t = mktime(&(dt.m_tm));
    return this_t - arg_t;
};

string datetime::Int2Str(const int i) const
{
    if(i > 9)
    {
        return std::to_string(i);
    }
    else
    {
        return string("0") + std::to_string(i);
    }
};

bool datetime::IsLeapYear(int year)
{
    if((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0)))
        return true;
    return false;
};

bool datetime::IsLegal(int year, int mon, int day)
{
    if(year < 0 || mon <= 0 || mon > 12 || day <= 0 || day > 31)return false;

    switch(mon)
    {
    case 4:
    case 6:
    case 9:
    case 11:
        return day <= 30;
    case 2:
        return datetime::IsLeapYear(year) ? (day <= 29) : (day <= 28);
    }
    return true;
};

datetime datetime::from_time(time_t t)
{
    datetime r;
#ifdef _WIN32
    localtime_s(&r.m_tm, &t);
#else
    localtime_r(&t, &r.m_tm);
#endif
    return r;
}