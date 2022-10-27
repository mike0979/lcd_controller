#include "Log.h"

#include <string.h>
#include <unistd.h>

#include <termios.h>
#include "TermiosU.h"

// http://blog.csdn.net/specialshoot/article/details/50707965

Termios::Termios()
{
	mFD = -1;
}

Termios::~Termios()
{
	close();
}

int Termios::open(const std::string &path, OpenMode mode)
{
	mFD = ::open(path.c_str(), mode | O_NOCTTY | O_NDELAY);
	if (mFD < 0) {
		LogE("Failed to open Term I/O : %s.\n", path.c_str());
	}
	else {
		struct termios opt;
//		tcgetattr(mFD, &opt);
//		bzero(&opt, sizeof(struct termios));
		cfmakeraw(&opt);

		opt.c_cflag  |=  CLOCAL | CREAD;
		opt.c_cc[VTIME]  = 0;
		opt.c_cc[VMIN] = 0;

		tcsetattr(mFD, TCSANOW, &opt);
	}

	return mFD;
}

void Termios::close() const
{
	if (mFD >= 0) {
		::close(mFD);

		mFD = -1;
	}
}

int Termios::setBaudRate(unsigned speed)
{
	speed_t br = 0;
	int ret = 0;

	if (mFD < 0) {
		LogD("Term I/O have NOT opened.\n");

		return ret;
	}

	switch (speed) {
	case 0:
		close();
		break;

	case 50:
		br = B50;
		break;

	case 75:
		br = B75;
		break;

	case 110:
		br = B110;
		break;

	case 134:
		br = B134;
		break;

	case 150:
		br = B150;
		break;

	case 200:
		br = B200;
		break;

	case 300:
		br = B300;
		break;

	case 600:
		br = B600;
		break;

	case 1200:
		br = B1200;
		break;

	case 1800:
		br = B1800;
		break;

	case 2400:
		br = B2400;
		break;

	case 4800:
		br = B4800;
		break;

	case 9600:
		br = B9600;
		break;

	case 19200:
		br = B19200;
		break;

	case 38400:
		br = B38400;
		break;

	case 57600:
		br = B57600;
		break;

	case 115200:
		br = B115200;
		break;

	case 230400:
		br = B230400;
		break;

	case 460800:
		br = B460800;
		break;

	case 500000:
		br = B500000;
		break;

	case 576000:
		br = B576000;
		break;

	case 921600:
		br = B921600;
		break;

	case 1000000:
		br = B1000000;
		break;

	case 1152000:
		br = B1152000;
		break;

	case 1500000:
		br = B1500000;
		break;

	case 2000000:
		br = B2000000;
		break;

	case 2500000:
		br = B2500000;
		break;

	case 3000000:
		br = B3000000;
		break;

	case 3500000:
		br = B3500000;
		break;

	case 4000000:
		br = B4000000;
		break;

	default:
		LogE("Baud Rate %u is NOT supported now.\n", speed);
		break;
	}

	if (br > 0) {
		struct termios opt;
		tcgetattr(mFD, &opt);

		cfsetispeed(&opt, br);
		cfsetospeed(&opt, br);

		ret = tcsetattr(mFD, TCSANOW, &opt);
	}

	return ret;
}

int Termios::setDataBits(int bits)
{
	int ret = 0;

	if (mFD < 0) {
		LogD("Term I/O have NOT opened.\n");

		return ret;
	}

	struct termios opt;
	tcgetattr(mFD, &opt);

	opt.c_cflag &= ~CSIZE;
	switch (bits) {
	case 5:
		opt.c_cflag |= CS5;
		break;

	case 6:
		opt.c_cflag |= CS6;
		break;

	case 7:
		opt.c_cflag |= CS7;
		break;

	case 8:
		opt.c_cflag |= CS8;
		break;

	default:
		ret = -1;
		LogE("Stop Bits %d is NOT supported now.\n", bits);
		break;
	}

	if (ret == 0) {
		ret = tcsetattr(mFD, TCSANOW, &opt);
	}

	return ret;
}

int Termios::setStopBits(int bits)
{
	int ret = 0;

	if (mFD < 0) {
		LogD("Term I/O have NOT opened.\n");

		return ret;
	}

	struct termios opt;
	tcgetattr(mFD, &opt);

	switch (bits) {
	case 1:
		opt.c_cflag &= ~CSTOPB;
		break;

	case 2:
		opt.c_cflag |= CSTOPB;
		break;

	default:
		ret = -1;
		LogE("Stop Bits %d is NOT supported now.\n", bits);
		break;
	}

	if (ret == 0) {
		ret = tcsetattr(mFD, TCSANOW, &opt);
	}

	return ret;
}

int Termios::setParityCheck(ParityCheckMode mode)
{
	int ret = 0;

	if (mFD < 0) {
		LogD("Term I/O have NOT opened.\n");

		return ret;
	}

	struct termios opt;
	tcgetattr(mFD, &opt);

	switch (mode) {
	case PC_None:
		opt.c_cflag &= ~PARENB;
		opt.c_iflag &= ~INPCK;
		break;

	case PC_Odd:
		opt.c_cflag |= PARENB;
		opt.c_cflag |= PARODD;
		opt.c_iflag |= INPCK;
		break;

	case PC_Even:
		opt.c_cflag |= PARENB;
		opt.c_cflag &= ~PARODD;
		opt.c_iflag |= INPCK;
		break;

	case PC_Space:
		opt.c_cflag &= ~PARENB;
		opt.c_cflag &= ~CSTOPB;
		break;

	default:
		ret = -1;
		LogE("Parity Check Mode %d is NOT supported now.\n", mode);
		break;
	}

	if (ret == 0) {
		ret = tcsetattr(mFD, TCSANOW, &opt);
	}

	return ret;
}

int Termios::setFlowControl(FlowControlMode mode)
{
	int ret = 0;

	if (mFD < 0) {
		LogD("Term I/O have NOT opened.\n");

		return ret;
	}

	struct termios opt;
	tcgetattr(mFD, &opt);

	switch (mode) {
	case FC_None:
		opt.c_cflag &= ~CRTSCTS;
		break;

	case FC_Hardware:
		opt.c_cflag |= CRTSCTS;
		break;

	case FC_Software:
		opt.c_cflag |= IXON;
		opt.c_cflag |= IXOFF;
		opt.c_cflag |= IXANY;
		break;

	default:
		ret = -1;
		LogE("Flow Control Mode %d is NOT supported now.\n", mode);
		break;
	}

	if (ret == 0) {
		ret = tcsetattr(mFD, TCSANOW, &opt);
	}

	return ret;
}

int Termios::getFD()
{
	return mFD;
}

const char *Termios::TAG = "Termios";
