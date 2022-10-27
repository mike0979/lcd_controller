#include <bj_pis/thread/thread_zcg.h>

using std::thread;

void ThreadZCG::Start()
{
	is_run_ = true;
	thread_ = thread([this]()
		{
			while (is_run_)
			{
				Update();
			}
		});
}

void ThreadZCG::Stop()
{
	if (is_run_)
	{
		is_run_ = false;
	}
}

void ThreadZCG::Dispose()
{
	Stop();
	if (thread_.joinable())
	{
		thread_.join();
	}
}

bool ThreadZCG::IsRun()
{
	return is_run_;
}

