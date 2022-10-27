#ifndef _FILE_SYS_UTILS_H_
#define _FILE_SYS_UTILS_H_

#include <string>
#include <list>

class FileSysUtils {
public:
	class DiskSpaceReportInner {
	public:
		std::string mMounted;
		std::string mFilesystem;

		unsigned mTotal;
		unsigned mUsed;
		unsigned mAvailable;
		unsigned mUsedPcg;
	};

	class DiskSpaceReport {
	public:
		std::list<DiskSpaceReportInner *> mReport;

		unsigned mTotal;
		unsigned mUsed;
		unsigned mAvailable;
		unsigned mUsedPcg;

	public:
		~DiskSpaceReport();

		void print();
	};

	static const int FR_OK;
	static const int FW_OK;
	static const int FX_OK;
	static const int FE_OK;

public:
	// test file's accessibility
	static bool Accessible(const std::string &file, int mode);

	// make a directory
	static int MakeDir(const std::string &dir);

	// new a empty file
	static int NewFile(const std::string &file);

	//get current path
	static std::string GetCurrPath();

	// scans the files in the directory
	static int ScanDirFiles(const std::string &dir, std::list<std::string> &list, bool recursive = false);

	// do house keeping in the directory, delete files which created outdated(days) ago
	static int HouseKeeping(const std::string &dir, unsigned outdated);

	// do house keeping in the directory, delete files which created outdated(days) ago
	static int HouseKeepingByCount(const std::string &dir, unsigned count);

	// get disk space report.
	static int DiskSpaceReporter(const std::string &path, DiskSpaceReport &report);

	// get disk space report.
	static std::string CpuUsedReporter();

	static std::string MemUsedReporter();

	// get total memory.(KB)
	static long MemTotal();

	// get directory or file total space size.(KB)
	static long GetSpaceSize(const std::string &path);

	// get dir name from path string
	static std::string Path2Dir(const std::string &path);

	// get file name from path string
	static std::string Path2File(const std::string &path);

	//check process had opened
	static bool checkProcessOpened(const std::string name);
private:
	static const char *TAG;
};

#endif /* _FILE_SYS_UTILS_H_ */
