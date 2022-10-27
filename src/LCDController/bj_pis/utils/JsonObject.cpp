#ifndef RAPIDJSON_ASSERT
#undef RAPIDJSON_ASSERT
#endif
#define RAPIDJSON_ASSERT(x) if(!(x)){throw std::runtime_error(std::string("json assert error: ") + __FUNCTION__+" "#x);}
#include "JsonObject.h"
#include <stdexcept>
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream>

JsonObject::JsonObject()
{
	m_pvalue = nullptr;
    m_precision = -1;
    ChangeType(0);
}
JsonObject::~JsonObject()
{
    ChangeType(0);
}
JsonObject::JsonObject(const JsonObject &o)
{
    m_pvalue = nullptr;
    m_type = 0;
    m_key = o.m_key;
    m_precision = o.m_precision;
    ChangeType(o.m_type);
    if(m_type == 3 || m_type == 4) //object||array
    {
        vector<JsonObject> *list = static_cast<vector<JsonObject>*>(m_pvalue);
        vector<JsonObject> *oldList = static_cast<vector<JsonObject>*>(o.m_pvalue);
        list->reserve(oldList->size());
        for(const auto &old : *oldList)
        {
            list->push_back(old);
        }
    }
    else if(m_type == 5)
    {
        string *nv = static_cast<string *>(m_pvalue);
        string *ov = static_cast<string *>(o.m_pvalue);
        *nv = *ov;
    }
    else if(m_type != 0)
    {
        double *nv = static_cast<double *>(m_pvalue);
        double *ov = static_cast<double *>(o.m_pvalue);
        *nv = *ov;
    }
}
JsonObject::JsonObject(JsonObject &&o)noexcept
{
    m_pvalue=o.m_pvalue;
    m_type=o.m_type;
    m_key=o.m_key;
    m_precision = o.m_precision;
    o.m_type=0;//这两行,别乱改了,之前改了报错
    o.m_pvalue = nullptr;
    o.m_key.clear();
}
JsonObject::JsonObject(const int &v){m_pvalue = nullptr;m_type = 0;ChangeType(1);*static_cast<double *>(m_pvalue)=v;}
JsonObject::JsonObject(const float &v){m_pvalue = nullptr;m_type = 0;ChangeType(6);*static_cast<double *>(m_pvalue)=v;}
JsonObject::JsonObject(const double &v){m_pvalue = nullptr;m_type = 0;m_precision = -1;ChangeType(7);*static_cast<double *>(m_pvalue)=v;}
JsonObject::JsonObject(const bool &v){m_pvalue = nullptr;m_type = 0;ChangeType(2);*static_cast<double *>(m_pvalue)=v?1:0;}
JsonObject::JsonObject(const string &v){m_pvalue = nullptr;m_type = 0;ChangeType(5);*static_cast<string *>(m_pvalue)=v;}
JsonObject::JsonObject(const char* v){m_pvalue = nullptr;m_type = 0;ChangeType(5);*static_cast<string *>(m_pvalue)=v;}
JsonObject& JsonObject::operator[](const string& key)
{
    ChangeType(3);//转为object
    vector<JsonObject>* list = static_cast<vector<JsonObject>*>(m_pvalue);
    for(auto &jo : *list)
    {
        if(jo.m_key == key)
        {
            return jo;
        }
    }

    JsonObject json;
    json.m_key = key;
    list->push_back(std::move(json));
    return list->back();
}
JsonObject& JsonObject::operator[](const char* key)
{
	return (*this)[string(key)];
}

JsonObject& JsonObject::operator[](int index)
{
    if(m_type!=3&&m_type!=4)
    {
        ChangeType(4);//转为array
    }
    vector<JsonObject> *list = static_cast<vector<JsonObject>*>(m_pvalue);
    int size = list->size();
    if(index < size)
    {
        return list->at(index);
    }
    if(index == size)
    {
        JsonObject json;
        list->push_back(json);
        return list->back();
    }
    throw runtime_error("json array index should not larger than array size");
}
JsonObject &JsonObject::operator=(const int right)
{
    ChangeType(1);
    *static_cast<double *>(m_pvalue) = right;
    return *this;
}
JsonObject &JsonObject::operator=(const float right)
{
    ChangeType(6);
    *static_cast<double *>(m_pvalue) = right;
    return *this;
}
JsonObject &JsonObject::operator=(const double right)
{
    ChangeType(7);
    *static_cast<double *>(m_pvalue) = right;
    return *this;
}
JsonObject &JsonObject::operator=(const string& right)
{
    ChangeType(5);
    *static_cast<string *>(m_pvalue) = right;
    return *this;
}
JsonObject &JsonObject::operator=(const char* right)
{
    ChangeType(5);
    *static_cast<string *>(m_pvalue) = right;
    return *this;
}
JsonObject &JsonObject::operator=(const bool right)
{
    ChangeType(2);
    *static_cast<double *>(m_pvalue) = right ? 1 : 0;
    return *this;
}
const nullptr_t JsonObject::operator=(const nullptr_t &right)
{
    ChangeType(0);
    return nullptr;
}
JsonObject& JsonObject::operator=(const JsonObject& o)
{
    if(o.m_key.size()>0)
    {m_key = o.m_key;}
    ChangeType(o.m_type);
    if(m_type == 3 || m_type == 4) //object||array
    {
        vector<JsonObject> *list = static_cast<vector<JsonObject>*>(m_pvalue);
        vector<JsonObject> *oldList = static_cast<vector<JsonObject>*>(o.m_pvalue);
        list->reserve(oldList->size());
        for(const auto &old : *oldList)
        {
            list->push_back(old);
        }
    }
    else if(m_type == 5)
    {
        string *nv = static_cast<string *>(m_pvalue);
        string *ov = static_cast<string *>(o.m_pvalue);
        *nv = *ov;
    }
    else if(m_type != 0)
    {
        double *nv = static_cast<double *>(m_pvalue);
        double *ov = static_cast<double *>(o.m_pvalue);
        *nv = *ov;
    }
	return *this;
}
static const char* JsonObjectTypeDesc[]={"Null","int","bool","object","array","string","float","double"};
#define JsonTypeAssert(x,t) if(m_type!=x) throw runtime_error(m_key + " type is "+JsonObjectTypeDesc[m_type]+", not " #t);
JsonObject::operator int()const
{
	JsonTypeAssert(1,int);
    return (int)(*static_cast<double *>(m_pvalue));
}
JsonObject::operator float()const
{
	JsonTypeAssert(6,float);
    return (float)(*static_cast<double *>(m_pvalue));
}
JsonObject::operator double()const
{
	JsonTypeAssert(7,double);
    return *static_cast<double *>(m_pvalue);
}
JsonObject::operator string()const
{
    if(m_type==0) return "";
	JsonTypeAssert(5,string);
    return *static_cast<string *>(m_pvalue);
}
JsonObject::operator bool()const
{
	JsonTypeAssert(2,bool);
    return (int)(*static_cast<double *>(m_pvalue)) != 0;
}
JsonObject &JsonObject::AddMember(const string &key, const JsonObject &object)
{
    ChangeType(3);//转为object
    vector<JsonObject> *list = static_cast<vector<JsonObject>*>(m_pvalue);
    list->push_back(object);
    list->back().m_key = key;
    return *this;
}
JsonObject &JsonObject::Push(const JsonObject &object)
{
    ChangeType(4);//转为array
    vector<JsonObject> *list = static_cast<vector<JsonObject>*>(m_pvalue);
    list->push_back(object);
    return *this;
}
JsonObject& JsonObject::SetArray()
{
	ChangeType(4);//转为array
	return *this;
}
void JsonObject::RemoveMember(const string& key)
{
    if(m_type == 3)
    {
        vector<JsonObject> *list = static_cast<vector<JsonObject>*>(m_pvalue);
        for (vector<JsonObject>::iterator i = list->begin(); i != list->end(); ++i)
        {
            if((*i).m_key == key)
            {
                list->erase(i);
                return;
            }
        }
    }
}
void JsonObject::RemoveAt(int index)
{
    if(m_type == 3||m_type == 4)
    {
        vector<JsonObject> *list = static_cast<vector<JsonObject>*>(m_pvalue);
        list->erase(list->begin()+index);
    }
}

bool JsonObject::HasMember(const string& key) const
{
    if(m_type == 3)
    {
        vector<JsonObject> *list = static_cast<vector<JsonObject>*>(m_pvalue);
        for (vector<JsonObject>::iterator i = list->begin(); i != list->end(); ++i)
        {
            if((*i).m_key == key)
            {
                return true;
            }
        }
    }
    return false;
}
void FRD_SetType(JsonObject& jo, int t)
{
    jo.ChangeType(t);
}
int FRD_GetType(const JsonObject& jo)
{
    return jo.m_type;
}
void* FRD_GetValue(const JsonObject& jo)
{
    return jo.m_pvalue;
}
static void JsonObject_RapidToJson(const rapidjson::Value &v, JsonObject &jo)
{
    if(v.IsObject())
    {
        FRD_SetType(jo, 3);//转为object
        vector<JsonObject> *list = static_cast<vector<JsonObject>*>(FRD_GetValue(jo));
        list->reserve(v.MemberCount());
        for (rapidjson::Value::ConstMemberIterator itr = v.MemberBegin(); itr != v.MemberEnd(); ++itr)
        {
            list->push_back(std::move(JsonObject()));
            JsonObject& tempObj = list->back();
            JsonObject_RapidToJson(itr->value, tempObj);
            tempObj.SetName(itr->name.GetString());
            
        }
    }
    else if(v.IsArray())
    {
        jo.SetArray();
        vector<JsonObject> *list = static_cast<vector<JsonObject>*>(FRD_GetValue(jo));
        list->reserve(v.Size());
        for (const auto &iv : v.GetArray())
        {
            list->push_back(std::move(JsonObject()));
            JsonObject& tempObj = list->back();
            JsonObject_RapidToJson(iv, tempObj);
        }
    }
    else if(v.IsInt())
    {
        jo = (int)v.GetInt();
    }
    else if(v.IsFloat())
    {
        jo = (float)v.GetFloat();
    }
    else if(v.IsDouble())
    {
        jo = (double)v.GetDouble();
    }
    else if(v.IsString())
    {
        jo = (string)v.GetString();
    }
    else if(v.IsBool())
    {
        jo = (bool)v.GetBool();
    }
}
void JsonObject::Parse(const string& jsonStr, JsonObject &jo)
{
    rapidjson::Document doc;
    size_t l=jsonStr.size();
    char ss[l+1]={'\0'};
    for(size_t i=0;i<l;i++)
    {
    		if(jsonStr.at(i)=='\n'||jsonStr.at(i)=='\r')
    	{
    			ss[i]=' ';
    	}
    		else
    	{
    			ss[i]=jsonStr.at(i);
    	}
    }
    doc.Parse(ss);
    if (doc.HasParseError())
    {
        throw runtime_error(string("json parse error:")+GetParseError_En(doc.GetParseError()));
    }
    jo.ChangeType(0);
    JsonObject_RapidToJson(doc, jo);
}

JsonObject& JsonObject::Parse(const string& jsonStr)
{
    Parse(jsonStr,*this);
    return *this;
}
static void JsonObject_SetRapidjson(rapidjson::Value &v, const JsonObject &j, rapidjson::Document::AllocatorType &allocator)
{
    switch(FRD_GetType(j))
    {
    case 0://null
        v.SetNull();
        break;
    case 1://int
        v.SetInt((int)(*static_cast<double *>(FRD_GetValue(j))));
        break;
    case 2://bool
        v.SetBool((int)(*static_cast<double *>(FRD_GetValue(j))) != 0);
        break;
    case 3://object
    {
        vector<JsonObject> *list3 = static_cast<vector<JsonObject>*>(FRD_GetValue(j));
        v.SetObject();
        for(const auto &json : *list3)
        {
            rapidjson::Value mv;
            JsonObject_SetRapidjson(mv, json, allocator);
            v.AddMember(rapidjson::StringRef(json.GetName().c_str()), mv, allocator);
        }
        break;
    }
    case 4://array
    {
        vector<JsonObject> *list4 = static_cast<vector<JsonObject>*>(FRD_GetValue(j));
        v.SetArray();
        for(const auto &json : *list4)
        {
            rapidjson::Value mv;
            JsonObject_SetRapidjson(mv, json, allocator);
            v.PushBack(mv, allocator);
        }
        break;
    }
    case 5://string
        v.SetString(rapidjson::StringRef((*static_cast<string*>(FRD_GetValue(j))).c_str()));
        break;
    case 6://float
        v.SetFloat((float)(*static_cast<double *>(FRD_GetValue(j))));
        break;
    case 7://double
        v.SetDouble(*static_cast<double *>(FRD_GetValue(j)));
        break;
    }
}
string JsonObject::ToString() const
{
    if(m_type == 5)
    {
        return *static_cast<string *>(m_pvalue);
    }
    else if(m_type == 3 || m_type == 4)
    {
        rapidjson::Document doc;
        rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
        JsonObject_SetRapidjson(doc, *this, allocator);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        if(m_precision>=0)
            writer.SetMaxDecimalPlaces(m_precision);
        doc.Accept(writer);
        return buffer.GetString();
    }
    else if(m_type == 1)
    {
        return std::to_string((int)(*static_cast<double *>(m_pvalue)));
    }
    else if(m_type == 2)
    {
        return (int)(*static_cast<double *>(m_pvalue)) != 0 ? "true":"false";
    }
    else if(m_type == 6 || m_type == 7)
    {
        std::stringstream ss;
        if(m_precision>=0)
        {
            ss << std::setprecision(m_precision+1) << *static_cast<double *>(m_pvalue);
        }
        else
        {
            ss << *static_cast<double *>(m_pvalue);
        }
        return ss.str();
    }

    return "";
}

int JsonObject::Size() const
{
    if(m_type == 3 || m_type == 4) //object||array
    {
        vector<JsonObject> *list = static_cast<vector<JsonObject>*>(m_pvalue);
        return list->size();
    }
    return 0;
}
int JsonObject::GetPrecision() const
{
    return m_precision;
}
void JsonObject::SetPrecision(int p)
{
    m_precision = p;
}
const string& JsonObject::GetName() const
{
    return m_key;
}
void JsonObject::SetName(const string& s)
{
    m_key = s;
}
vector<JsonObject>::iterator JsonObject::begin()
{
    if(m_type == 3 || m_type == 4) //object||array
    {
        vector<JsonObject> *list = static_cast<vector<JsonObject>*>(m_pvalue);
        return list->begin();
    }
    throw runtime_error(string("json is not array or object. the type is:")+to_string(m_type));
}
vector<JsonObject>::iterator JsonObject::end()
{
    if(m_type == 3 || m_type == 4) //object||array
    {
        vector<JsonObject> *list = static_cast<vector<JsonObject>*>(m_pvalue);
        return list->end();
    }
    throw runtime_error(string("json is not array or object. the type is:")+to_string(m_type));
}
void JsonObject::Sort(function<bool(JsonObject,JsonObject)> func)
{
    if(m_type == 3 || m_type == 4) //object||array
    {
        vector<JsonObject> *list = static_cast<vector<JsonObject>*>(m_pvalue);
        sort(list->begin(),list->end(),func);
        return;
    }
    throw runtime_error(string("json is not array or object. the type is:")+to_string(m_type));
}
void JsonObject::ChangeType(int t)
{
    if(t != m_type)
    {
        ResetPointer();
        m_type = t;

        if(m_type == 1 || m_type == 2 || m_type == 6 || m_type == 7) //number||bool||float||double
        {
            m_pvalue = new double;
        }
        else if(m_type == 3 || m_type == 4) //object||array
        {
            vector<JsonObject>* pV = new vector<JsonObject>();
            pV->reserve(8);
            m_pvalue = pV;
        }
        else if(m_type == 5)//string
        {
            m_pvalue = new string;
        }
    }
}
void JsonObject::ResetPointer()
{
    if(m_pvalue == nullptr)return;
    if(m_type == 1 || m_type == 2 || m_type == 6 || m_type == 7) //number||bool||float||double
    {
        double *temp = static_cast<double *>(m_pvalue);
        delete temp;
    }
    else if(m_type == 3 || m_type == 4) //object||array
    {
        vector<JsonObject> *temp = static_cast<vector<JsonObject>*>(m_pvalue);
        temp->clear();
        delete temp;
    }
    else if(m_type == 5)//string
    {
        string *temp = static_cast<string *>(m_pvalue);
        delete temp;
    }
    m_pvalue = nullptr;
}

void JsonObject::FromFile(const string& path)
{
	std::ifstream ifs(path.c_str());
	if (!ifs)
	{
		throw runtime_error(string("can't open file:")+path);
	}
	std::ostringstream tmp;
	tmp << ifs.rdbuf();
	Parse(tmp.str());
	ifs.close();
}
