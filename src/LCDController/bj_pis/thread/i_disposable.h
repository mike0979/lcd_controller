#ifndef I_DISPOSABLE_H_
#define I_DISPOSABLE_H_

class IDisposable
{
public:
	virtual ~IDisposable() = default;
	virtual void Dispose() = 0;
};

#endif
