/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef XML_STATIONEQUIPDEF_H_
#define XML_STATIONEQUIPDEF_H_

#include <string>
#include <list>

namespace xml {

	class StationEquip {
	public:
		std::string mEquipID;
		std::string mEquipIP;
		std::string mText;

	public:
		void print();

	private:
			static void StationEquipTextReader(const std::string &text, void *data);

	private:
		static const char *TAG;

		friend class StationsDef;
		friend class StationDef;
	};

	class StationDef {
	public:
		std::string mCName;
		std::string mEName;
		std::string mEquipType;

		std::list<StationEquip*> mEquipList;

	public:
		~StationDef();
		void print();
	private:
		static void *StationDefReader(const char *name, const char **atts, void *data);

	private:
		static const char *TAG;
		friend class StationsDef;
	};

	class StationsDef {
	public:
		std::string mTimestamp;
		std::list<StationDef*> mStationDefList;

	public:

		~StationsDef();
		void print();
		static bool StationsDefXMLParser(const std::string &file,
				StationsDef *arrivalmsg);

	private:
		static void *SelfReader(const char *name, const char **atts, void *data);
		static void *StationDefReader(const char *name, const char **atts, void *data);

	private:
		static const char *TAG;
	};

}




#endif /* XML_STATIONEQUIPDEF_H_ */
