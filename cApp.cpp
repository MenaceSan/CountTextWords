// SSFI By Dennis Robinson (dennis@menasoft.com)

#include "pch.h"
#include "cApp.h"
#include "cThreadDirReader.h"
#include "cThreadFileReader.h"

#ifdef _WIN32
#include <conio.h>	// _kbhit
#endif

namespace SSFI
{
	void cAppFilesToRead::AddFile(const fse::directory_entry& dirEnt, cApp& app)
	{
		// Add a file to be processed.
		std::unique_lock<std::mutex> lock(this->Mutex);

		app.FilesFound++;
		this->Files.push(cAppFileToRead(dirEnt));	// push back

		if (app.ThreadFileReaders.size() < app.Params.MaxThreads
			&& app.PleaseExit == WaitForMore	// not exiting.
			&& (!this->Files.empty() || app.ThreadFileReaders.empty()))	// only if we actually need a new thread.
		{
			// Create more worker threads as needed.
			auto thread = std::make_shared<cThreadFileReader>(app);
			app.ThreadFileReaders.push_back(thread);		// ASSUME this->Mutex is locked.
			lock.unlock();
			thread->Start();
		}
		else
		{
			lock.unlock();
		}

		if (this->Files.size() >= app.Params.DirHeadStart || app.PleaseExit != WaitForMore)
		{
			HasFiles.notify_one();	// un-block one worker to process this.
		}
	}

	cAppFileToRead cAppFilesToRead::WaitForFile(PleaseExitType& pleaseExit)
	{
		// Wait for a file.
		// RETURN: empty file means nothing to do. go back to sleep.

		std::unique_lock<std::mutex> lock(Mutex);
		HasFiles.wait(lock, [&] { return !Files.empty() || pleaseExit != WaitForMore; });

		if (this->Files.empty())	// We got pleaseExit i assume.
		{
			return cAppFileToRead();	// IsEmpty().
		}

		cAppFileToRead file = this->Files.front();
		this->Files.pop();
		return file;
	}

	void cAppDict::AddWord(std::string word)
	{
		// Add a word i have found (in a file) to the dictionary. Inc the count.

		std::transform(word.begin(), word.end(), word.begin(), ::tolower);		// make lower case.
		std::unique_lock<std::mutex> lock(Mutex);
		TotalWords++;
		this->_Map[word] ++;	// thread safe add.
	}

	inline bool ComparerTopTen(const std::pair<int, std::string>& a, int id)
	{
		// Flip logic to sort from greatest to least.
		return a.first >= id;
	}

	void cAppDict::BuildTopTen(std::vector<std::pair<int, std::string>>& TopTen)
	{
		// Build the TopTen list from the dictionary. Allow ties.

		for (const auto& word : this->_Map)
		{
			// find first element in the range that is not less than (i.e. greater or equal to) value (word.second), or last if no such element is found
			auto endi = TopTen.end();
			auto j = std::lower_bound(TopTen.begin(), endi, word.second, ComparerTopTen);
			if (TopTen.size() >= 10 && j == endi)
			{
				// toss it. i already have 10.
				continue;
			}
			TopTen.insert(j, std::pair<int, std::string>(word.second, word.first));
			if (TopTen.size() > 10)
			{
				TopTen.resize(10);	// keep first 10.
			}
		}
	}

	void cApp::CreateAllThreads()
	{
		// Front load creation of threads.
		for (unsigned int i = 0; i < this->Params.MaxThreads; i++)
		{
			auto thread = std::make_shared<cThreadFileReader>(*this);
			this->ThreadFileReaders.push_back(thread);		//  
		}
	}
 
	cApp::cApp(int argc, const char *argv[])
		: FilesFound(0)
		, FilesRead(0)
	{
		// "~/tmp"
		// "c:\tmp" -createsimpletest 1000 -quit
		// "c:\tmp" -t 4

		std::cout << "Start ssfi v" << std::to_string(cAppParams::kVersion) << "." << std::endl;

		// Parse the command line.
		if (!Params.Parse(argc, argv))
		{
			// Assume a cout message was printed.
			return;
		}

		// Start looking at my files.
		std::cout << "Inspecting directory " << this->Params.Dir << "." << std::endl;

		CreateAllThreads();
		for (auto thread : this->ThreadFileReaders)
		{
			thread->Start();
		}

		auto timeStart = std::chrono::high_resolution_clock::now();
		cThreadDirReader threadDirReader(*this);
		threadDirReader.Start();

#if false // def _WIN32 // true // false
		// https://docs.microsoft.com/en-us/previous-versions/58w7c94c(v%3Dvs.140)
		std::cout << "Press ESC to terminate." << std::endl;
		while (this->PleaseExit == WaitForMore || (this->PleaseExit == ExitWhenDone && this->FilesFound != this->FilesRead))	// just wait for all threads to do their thing.
		{
			if (_kbhit())	// no good way to block for thread and key at the same time.
			{
				char keyPress = _getch();
				if (keyPress == 27)		// ESCAPE key
				{
					this->PleaseExit = ExitNow;
					this->FilesToRead.PleaseExitChanged();
					break;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(200));	// reasonable human response time.
		}
#else
		// Press CTRL-C to send term signal to app.
		std::cout << "Press CTRL-C to terminate." << std::endl;
#endif

		// Wait for threads to complete/close properly.
		threadDirReader.Join();		// Wait for dir read to complete.
		assert(this->PleaseExit != WaitForMore);
		if (Params.DirHeadStart > 0)	// make sure all workers are started.
		{
			FilesToRead.PleaseExitChanged();
		}
		auto timeReadComplete = std::chrono::high_resolution_clock::now();

		// Next, wait for all worker cThreadFileReader threads to complete.
		for (auto thread : this->ThreadFileReaders)
		{
			thread->Join();
		}

		// we are done. 
		auto timeEnd = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> timeElapsed = timeEnd - timeStart;
		std::chrono::duration<double, std::milli> timeToRead = timeReadComplete - timeStart;

		// Tabulate the results.
		std::cout << "Total Time: " << timeElapsed.count() << " ms" << std::endl;
		std::cout << "Time to Read: " << timeToRead.count() << " ms" << std::endl;

		std::cout << std::to_string(this->ThreadFileReaders.size()) + " thread(s) created." << std::endl;
		std::cout << std::to_string(this->FilesRead) + " file(s) read." << std::endl;

		std::cout << std::to_string(this->Dict.TotalWords) << " total words." << std::endl;
		std::cout << std::to_string(this->Dict.getUniqueWords()) << " unique words." << std::endl;
		
		std::cout << "Top ten results are : " << std::endl;

		std::vector<std::pair<int, std::string>> TopTen;
		Dict.BuildTopTen(TopTen);

		// Print results.
		for (auto item : TopTen)
		{
			std::cout << item.second << "\t" << std::to_string(item.first) << std::endl;
		}

		std::cout << std::endl;
	}
}
