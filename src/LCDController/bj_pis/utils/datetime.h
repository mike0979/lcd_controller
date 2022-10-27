#pragma once
#include <ctime>
#include <string>
#include <stdexcept>

using namespace std;

class datetime
{
public:
    static datetime GetDateTime(int year, int month, int day);
    static datetime Parse(const string& timeStr, const string& format="%Y-%m-%d %H:%M:%S");
    static datetime Now();

    datetime AddSeconds(int i) const;
    datetime AddMinutes(int i) const;
    datetime AddHours(int i) const;
    datetime AddDays(int i) const;
    datetime AddMonths(int i) const;
    datetime AddYears(int i) const;

    int GetYear() const;
    int GetMonth() const;
    int GetDay() const;
    int GetHour() const;
    int GetMinute() const;
    int GetSecond() const;
    //星期天-星期六，0到6
    int GetWeek() const;
    string ToString(const string& format="%Y-%m-%d %H:%M:%S") const;
    
    bool operator==(datetime &dt);

    datetime& operator=(const datetime &dt);

    time_t operator-(datetime &dt);
    tm get_tm(){return m_tm;};
    time_t get_time(){return mktime(&m_tm);};
    static datetime from_time(time_t t);
private:
    tm m_tm;
    string Int2Str(const int i) const;

    static bool IsLeapYear(int year);

    static bool IsLegal(int year, int mon, int day);
};