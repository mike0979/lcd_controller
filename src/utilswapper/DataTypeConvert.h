/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name :
* @author :
* @date : 2017/7/14 14:29
* @brief :
*/
#ifndef SRC_DATATYPECONVERT_H_
#define SRC_DATATYPECONVERT_H_
#include <string>
#include <sstream>


class DataTypeConvert {

public:
	template<typename T>
	static std::string dataToString(T t)
	{
		std::ostringstream oss;
		oss << t;
		return oss.str();
	}

	template<typename T>
	static T stringToNum(const std::string& str){
	    std::istringstream iss(str);
	    T num;
	    iss >> num;
	    return num;
	}
};


#endif /* SRC_DATATYPECONVERT_H_ */
