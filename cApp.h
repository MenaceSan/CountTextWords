// SSFI By Dennis Robinson (dennis@menasoft.com)
#pragma once
#include "cAppParams.h"
#include "cThreadFileReader.h"

namespace SSFI
{
	struct cAppLockable
	{
		// Make something thread safe.
	protected:
		std::mutex Mutex;	// Exclusive control for a thread.
	};

	class cAppDict : cAppLockable
	{
		// Thread safe Dictionary to hold words I have found.
		// How big could this get? ~100k max ? https://en.oxforddictionaries.com/explore/how-many-words-are-there-in-the-english-language/

		std::map<std::string, int> _Map;		// All the words i have found so far.
	public:
		int TotalWords = 0;

	public:
		size_t getUniqueWords() const
		{
			return _Map.size();
		}
		void AddWord(std::string word);
		void BuildTopTen(std::vector<std::pair<int, std::string>>& TopTen);
	};

	class cAppFileToRead
	{
		// A single file to be processed.

	public:
		fsx::path Path;
		uintmax_t Size = 0;	// File system size of the file.

	public:
		cAppFileToRead()
		{
			// Empty means nothing to do yet. IsEmpty()
			assert(IsEmpty());
		}
		cAppFileToRead(const fse::directory_entry& dirEnt)
			: Path(dirEnt)
			, Size(dirEnt.file_size())
		{
		}

		bool IsEmpty() const
		{
			return Path.empty() || Size <= 0;
		}
	};
	
	enum PleaseExitType
	{
		WaitForMore = 0,	// 0 = keep waiting for more work.
		ExitWhenDone,		// 1 = Ask the threads nicely to close when done working. 
		ExitNow,			// 2 = close immediately.
	};

	class cAppFilesToRead : cAppLockable
	{
		// A work queue of files to be read/processed.
		std::queue<cAppFileToRead> Files;		// Files to be processed. 
		std::condition_variable HasFiles;		// Wait on this until there is data or we get pleaseExit.

	public:
		void AddFile(const fse::directory_entry& dirEnt, cApp& app);
		cAppFileToRead WaitForFile(PleaseExitType& pleaseExit);

		void PleaseExitChanged()
		{
			// ASSUME PleaseExit is set.
			HasFiles.notify_all();	// break all waits. recheck condition_variable.
		}
	};
	
	class cApp
	{
		// The NetApp SSFI interview application.

	public:
		cAppParams Params;					// command line args.
		std::vector< std::shared_ptr<cThreadFileReader>> ThreadFileReaders;	// workers
		std::atomic<int> FilesFound;	// count total files found.
		std::atomic<int> FilesRead;		// count total files read.
		cAppFilesToRead FilesToRead;		// Queue of files left to be read/process.
		cAppDict Dict;						// Count occurrences of words.
		PleaseExitType PleaseExit = WaitForMore;  // Exit?

	private:
		void CreateAllThreads();

	public:
		cApp(int argc, const char *argv[]);
	};
}
