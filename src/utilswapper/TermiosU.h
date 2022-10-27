#ifndef SRC_TERMIOSU_H_
#define SRC_TERMIOSU_H_

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

class Termios
{
public:
	enum OpenMode {
		R_ONLY = O_RDONLY,
		W_ONLY = O_WRONLY,
		RW = O_RDWR,
	};

	enum ParityCheckMode {
		PC_None = 0,
		PC_Odd,
		PC_Even,
		PC_Space,
	};

	enum FlowControlMode {
		FC_None = 0,
		FC_Hardware,
		FC_Software,
	};

public:
	Termios();
	~Termios();

	virtual int open(const std::string &path, OpenMode mode);
	virtual void close() const;

	int setBaudRate(unsigned speed);
	int setDataBits(int bits);
	int setStopBits(int bits);
	int setParityCheck(ParityCheckMode mode);
	int setFlowControl(FlowControlMode mode);

	int getFD();

private:
	static const char *TAG;

	mutable int mFD;
};

#endif /* SRC_TERMIOSU_H_ */
