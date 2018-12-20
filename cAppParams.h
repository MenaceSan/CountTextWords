#pragma once
#include "pch.h"

namespace SSFI
{
	struct cAppParams
	{
		// Params from the command line to control what this app does.

		static const int kVersion = 3;	// every app should have a version
		static const char* kExt;		// Only read files with this extension.

		fsx::path Dir;		// default param. e.g. : ./ssfi /usr/share/doc

		unsigned int MaxThreads = 0;		// fixed number (N) of worker threads. "-t #"

		// Experiments in timing.
		size_t DirHeadStart = 0;		// Give the dir reader thread a head start.
		bool SerialFileReads = false;	// because the disk may be serialized the thread contention here might be bad.
		std::mutex SerialFileReadMutex;	// Exclusive control for the disk.

	public:
		static int GetRand1(int qty);
		static int GetRand2(int lo, int hi);

		void CreateTestData(bool isSimpleTestData, int seed = 1234);

		bool Parse(int argc, const char *argv[]);
	};
}
