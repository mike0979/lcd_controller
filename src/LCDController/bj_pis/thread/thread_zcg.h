#ifndef THREAD_H_
#define THREAD_H_

#include <thread>
#include "i_disposable.h"

using namespace std;

class ThreadZCG : public IDisposable
{
public:
	virtual void Start();
	void Stop();
	void Dispose() override;
	bool IsRun();
protected:
	virtual void Update() {};
protected:
	std::thread thread_;
	bool is_run_{ false };
};

#endif
