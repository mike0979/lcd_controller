/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef SRC_XMLPARSER_H_
#define SRC_XMLPARSER_H_


#include <string>
#include <stack>

#include "FileMap.h"
#include "KeyValuePair.h"

#include <expat.h>

typedef void *(*XMLParser_StartElementHandler)(const char *name, const char **atts, void *data);
typedef void *(*XMLParser_EndElementHandler)(const char *name, void *data);
typedef void (*XMLParser_ElementTextHandler)(const std::string &text, void *data);

typedef struct {
	const char *mName;
	const char *mPName;

	XMLParser_StartElementHandler mStartElementHandler;
	XMLParser_EndElementHandler mEndElementHandler;
	XMLParser_ElementTextHandler mElementTextHandler;
} XMLParserHandler;

class XMLParser {
public:
	XMLParser(const std::string &file);
	XMLParser(const void *data, size_t len);
	~XMLParser();

	void* parser(XMLParserHandler *handler, void *data);

	bool isParsered();
public:
	static int FastAtts(const char **atts, KeyValuePair *kvset);

private:
	static void startElementHandler(void *data, const char *name, const char **atts);
	static void endElementHandler(void *data, const char *name);
	static void characterDataHandler(void *data, const char *s, int len);

private:
	static const char *TAG;

	XMLParserHandler *mHandler;
	void *mUsrData;

	std::string mName;
	std::stack<void *> mData;
	std::stack<XMLParser_ElementTextHandler> mTextHandler;

	FileMap *mXMLFile;
	XML_Parser mParser;

	const void *mDataBuf;
	size_t mDataBufLen;

	bool mParsered;
};

#endif /* SRC_XMLPARSER_H_ */
