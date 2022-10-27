#ifndef PARCELABLE_H_
#define PARCELABLE_H_

#include "Parcel.h"

class Parcelable {
public:
	virtual ~Parcelable() {};

	virtual int writeToParcel(Parcel &out) const = 0;
	virtual int readFromParcel(Parcel &in) = 0;

#if 0
	 template <typename T>
	 class Creator {
	 public:
		 virtual ~Creator() {}

		 virtual T* createFromParcel(Parcel& in) = 0;
	 };
#endif
};

#endif /* PARCELABLE_H_ */
