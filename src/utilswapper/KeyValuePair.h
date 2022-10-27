#ifndef KEYVALUEPAIR_H_
#define KEYVALUEPAIR_H_

#include <string>
#include <map>
class KeyValuePair {
public:
	const char *mKey;

	union {
		void *V;	// NOT used

		int *I;
		unsigned *H;
		bool *B;
		float *F;
		std::string *S;
		unsigned *C;
		std::map<std::string,std::string> *L;
		std::map<std::string,int> *M;

	} mValue;

	const char mValType;		// I(nt)/H(ex)/B(ool)/F(loat)/S(tring)/C(olor)
	const char *mDefVal;

public:
	KeyValuePair(const char *key, void *value, const char valType, const char *defVal);

	void setValue(const char *value,const char *key);
	std::string getValue();


private:
	static const char *TAG;
};

#endif /* KEYVALUEPAIR_H_ */
