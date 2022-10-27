#pragma once
#ifndef JSONOBJECT_H_
#define JSONOBJECT_H_
#include <string>
#include <vector>
#include <functional>

using namespace std;

class JsonObject
{
public:
    JsonObject();
    ~JsonObject();
    JsonObject(const JsonObject &o);
    JsonObject(JsonObject &&o)noexcept;//移动构造函数
    JsonObject(const int &v);
    JsonObject(const float &v);
    JsonObject(const double &v);
    JsonObject(const bool &v);
    JsonObject(const string &v);
    JsonObject(const char *v);
    JsonObject &operator[](const string &key);
    JsonObject &operator[](const char *key);
    JsonObject &operator[](int index);
    JsonObject &operator=(const int right);
    JsonObject &operator=(const float right);
    JsonObject &operator=(const double right);
    JsonObject &operator=(const string &right);
    JsonObject &operator=(const char *right);
    JsonObject &operator=(const bool right);
    JsonObject &operator=(const JsonObject &o);
    operator int()const;
    operator float()const;
    operator double()const;
    operator string()const;
    operator bool()const;
    const nullptr_t operator=(const nullptr_t &right);
    JsonObject &AddMember(const string &key, const JsonObject &object);
    JsonObject &Push(const JsonObject &object);
    JsonObject &SetArray();
    void RemoveMember(const string &key);
    void RemoveAt(int index);
    bool HasMember(const string &key) const;
    static void Parse(const string &jsonStr, JsonObject &jo);
    JsonObject &Parse(const string &jsonStr);
    string ToString() const;
    int Size() const;
    int GetPrecision() const;
    //设置浮点型的精度
    void SetPrecision(int p);
    const string& GetName() const;
    void SetName(const string& s);
    vector<JsonObject>::iterator begin();
    vector<JsonObject>::iterator end();
    void Sort(function<bool(JsonObject,JsonObject)> func);
    void FromFile(const string& path);

    friend void FRD_SetType(JsonObject& jo, int t);
    friend int FRD_GetType(const JsonObject& jo);
    friend void* FRD_GetValue(const JsonObject& jo);
private:
    void *m_pvalue;
    string m_key;
    int m_type;//0=null,1=int,2=bool,3=object,4=array,5=string,6=float,7=double
    void ChangeType(int t);
    void ResetPointer();
    int m_precision;//小数点后几位,默认-1(不做设置)
};

#endif
