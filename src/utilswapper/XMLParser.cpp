/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/


#include "XMLParser.h"
#include "Log.h"

#include <stdlib.h>
#include <string.h>

#include <list>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

XMLParser::XMLParser(const std::string &file)
{
	mHandler = NULL;
	mUsrData = NULL;

	mXMLFile = NULL;

	mParser = XML_ParserCreate(NULL);
	XML_SetUserData(mParser, this);
	XML_SetElementHandler(mParser, startElementHandler, endElementHandler);
	XML_SetCharacterDataHandler(mParser, characterDataHandler);

	int fd = open(file.c_str(), O_RDONLY);
	if (fd < 0) {
		LogE("Failed to open file %s.\n", file.c_str());
	}
	else {
		mXMLFile = new FileMap();

		bool ret = mXMLFile->create(file.c_str(), fd);
		if (ret == false) {
			LogE("Failed to mmap file : %s\n", file.c_str());

			mXMLFile->release();
			mXMLFile = NULL;
		}

		close(fd);
	}

	mDataBuf = NULL;
	mDataBufLen = 0;

	mParsered = false;
}

XMLParser::XMLParser(const void *data, size_t len) : mDataBuf(data), mDataBufLen(len)
{
	mHandler = NULL;
	mUsrData = NULL;

	mXMLFile = NULL;

	mParser = XML_ParserCreate(NULL);
	XML_SetUserData(mParser, this);
	XML_SetElementHandler(mParser, startElementHandler, endElementHandler);
	XML_SetCharacterDataHandler(mParser, characterDataHandler);

	mParsered = false;
}

XMLParser::~XMLParser()
{
	if (mXMLFile != NULL) {
		mXMLFile->release();
	}

	XML_ParserFree(mParser);
}

void* XMLParser::parser(XMLParserHandler *handler, void *data)
{
	if (mParser == NULL) {
		return NULL;
	}

	if (handler == NULL) {
		return NULL;
	}

	const char *buf = NULL;
	int remains = 0;
	if (mXMLFile != NULL) {
		buf = (const char *)mXMLFile->getDataPtr();
		remains = mXMLFile->getDataLength();
	}
	else if (mDataBuf != NULL) {
		buf = (const char *)mDataBuf;
		remains = mDataBufLen;
	}
	else {
		return NULL;
	}

	mHandler = handler;
	mUsrData = data;
	mData.push(data);

	int offset = 0, len, final;
	while (remains > 0) {
		if (remains > BUFSIZ) {
			len = BUFSIZ;
			final = 0;
		}
		else {
			len = remains;
			final = 1;
		}

		if (XML_Parse(mParser, buf + offset, len, final) == XML_STATUS_ERROR) {
			LogE("Parser XML(%s) failed at line %\n", XML_ErrorString(XML_GetErrorCode(mParser)), XML_GetCurrentLineNumber(mParser));
		}

		offset += len;
		remains -= len;
	}

	data = mData.top();

	mData.pop();

	mParsered = true;

	return data;
}

bool XMLParser::isParsered()
{
	return mParsered;
}

int XMLParser::FastAtts(const char **atts, KeyValuePair *kvset)
{
	if (kvset == NULL) {
		return 0;
	}

	int matched = 0;

	std::list<KeyValuePair *> kvlist;
	for (KeyValuePair *kv = kvset; kv->mKey != NULL; kv++) {
		kvlist.push_back(kv);
	}

	for (const char **aptr = atts; *aptr != NULL; aptr += 2) {
		for (std::list<KeyValuePair *>::iterator kvi = kvlist.begin(); kvi != kvlist.end(); kvi++) {
			if (strcmp(*aptr, (*kvi)->mKey) == 0) {
				(*kvi)->setValue(*(aptr + 1),NULL);

				kvlist.erase(kvi);
				matched++;
				break;
			}
		}
	}

	for (std::list<KeyValuePair *>::iterator kvi = kvlist.begin(); kvi != kvlist.end(); kvi++) {
		(*kvi)->setValue(NULL,NULL);
	}

	return matched;
}

void XMLParser::startElementHandler(void *data, const char *name, const char **atts)
{
	XMLParser *parser = (XMLParser *)data;

	void *value = NULL;
	XMLParserHandler *handler = NULL;

	for (handler = parser->mHandler; handler->mName != NULL; handler++) {
		if (strcmp(handler->mName, name) == 0) {
			bool call = false;

			if (handler->mPName == NULL) {
				if (parser->mName.empty() == true) {
					call = true;
				}
			}
			else {
				size_t nlen = parser->mName.length();
				size_t pnlen = strlen(handler->mPName);
				if ((nlen >= pnlen) && parser->mName.compare(nlen - pnlen, pnlen, handler->mPName) == 0) {
					call = true;
				}
			}

			if (call == true) {
				if (handler->mStartElementHandler != NULL) {
					value = handler->mStartElementHandler(name, atts, parser->mData.top());
				}
				break;
			}
		}
	}

	if (parser->mData.size() == 1 && parser->mData.top() == NULL) {
		parser->mData.top() = value;
	}

	parser->mName.append("/").append(name);
	parser->mData.push(value);
	parser->mTextHandler.push(handler->mElementTextHandler);
}

void XMLParser::endElementHandler(void *data, const char *name)
{
	XMLParser *parser = (XMLParser *)data;

	parser->mName.erase(parser->mName.find_last_of('/'));

	for (XMLParserHandler *handler = parser->mHandler; handler->mName != NULL; handler++) {
		if (strcmp(handler->mName, name) == 0) {
			bool call = false;

			if (handler->mPName == NULL) {
				call = true;
			}
			else {
				size_t nlen = parser->mName.length();
				size_t pnlen = strlen(handler->mPName);
				if ((nlen >= pnlen) && parser->mName.compare(nlen - pnlen, pnlen, handler->mPName) == 0) {
					call = true;
				}
			}

			if (call == true) {
				if (handler->mEndElementHandler != NULL) {
					handler->mEndElementHandler(name, parser->mData.top());
				}
				break;
			}
		}
	}

	parser->mData.pop();
	parser->mTextHandler.pop();
}

void XMLParser::characterDataHandler(void *data, const char *s, int len)
{
	XMLParser *parser = (XMLParser *)data;

	XMLParser_ElementTextHandler textHandler = parser->mTextHandler.top();
	if (textHandler != NULL) {
		textHandler(std::string(s, len), parser->mData.top());
	}
}

const char *XMLParser::TAG = "XMLParser";
