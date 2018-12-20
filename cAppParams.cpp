#include "pch.h"
#include "cAppParams.h"
#include "Util.h"

#define USE_RAND_LOCAL

namespace SSFI
{
	const char* cAppParams::kExt = ".txt";	// ONLY files that end in this.

#ifdef USE_RAND_LOCAL
	static unsigned int g_nRandSeed = 0;	// RAND_MAX
#endif

	int cAppParams::GetRand1(int qty)	// static
	{
		if (qty <= 0)
			return 0;
		assert(qty <= RAND_MAX);	// ASSUME qty <= RAND_MAX

#ifdef USE_RAND_LOCAL
		g_nRandSeed = g_nRandSeed * 214013L + 2531011L;	// This will match both linux and windows versions.
		return (g_nRandSeed >> 16) % qty;
#else
		return (std::rand() % qty);
#endif
	}
	int cAppParams::GetRand2(int lo, int hi)	// static
	{
		// less than or equal to hi.
		return lo + GetRand1(hi - lo);
	}

	void cAppParams::CreateTestData(bool isSimpleTestData, int seed )
	{
		// Create a bunch of test files. latin1 encoded.
		// "-createtest -quit" 
		// "-createsimpletest 1000 -quit"

		const int simpleScale = seed;
		// For debugging, set seed for predictable results. 
		if (isSimpleTestData)
		{
#ifdef USE_RAND_LOCAL
			g_nRandSeed = seed;
#else
			std::srand(seed);	// a predictable set.
#endif
		}

		// Make up a bunch of words to put in files.
		int iWordsTotal = isSimpleTestData ? simpleScale : GetRand2(100, 4000);
		std::vector<std::string> words;
		for (int iWord = 0; iWord < iWordsTotal; iWord++)
		{
			size_t iWordLen = GetRand2(1, 30);
			std::string word;
			for (size_t i = 0; i < iWordLen; i++)
			{
				char ch = (char) GetRand2('a', 'z');
				if (i == 0 && GetRand1(100) == 0)	// make it a proper noun.
				{
					ch = (char)std::toupper(ch);
				}
				word += ch;
			}
			words.push_back(word);
		}

		int iFiles = isSimpleTestData ? 10 : GetRand2(10, 100);

		std::cout << "Creating " << std::to_string(iFiles) << " test files from " << std::to_string(iWordsTotal) << " words." << std::endl;

		for (int iFile = 0; iFile < iFiles; iFile++)
		{
			// Make a file.
			fsx::path filePath = this->Dir;
			filePath.append("TestFile");	// add dir sep
			filePath += std::to_string(iFile+1);
			filePath += cAppParams::kExt;

			std::ofstream file;
			file.open(Util::CvtA(filePath));	// throw on fail. is RAII.

			// Write file with a bunch of random words.
			int iWordsInFile = isSimpleTestData ? simpleScale : GetRand2(10, 6000);
			int iWordsInSentence = 0;
			size_t nCharsOnLine = 0;

			for (int i = 0; i < iWordsInFile; i++)
			{
				if (nCharsOnLine > 64)
				{
					file << std::endl;
					nCharsOnLine = 0;
				}
				else if (nCharsOnLine != 0)
				{
					file << " ";
					nCharsOnLine++;
				}

				int iNextWord = GetRand1(iWordsTotal);
				const std::string& nextWord = words[iNextWord];

				if (iWordsInSentence <= 0)
				{
					// Start new sentence.
					iWordsInSentence = GetRand2(3, 50);
					file << (char)(std::toupper(nextWord[0]));	// capitalize.
					file << (nextWord.c_str()+1);
				}
				else
				{
					file << nextWord;
					if (--iWordsInSentence <= 0)
					{
						file << ".";	// end this sentence.
						nCharsOnLine++;
					}
				}

				nCharsOnLine += nextWord.length();
			}

			if (iWordsInSentence > 0)
			{
				file << "." << std::endl;	// hard terminate sentence.
			}
		}
	}

	bool cAppParams::Parse(int argc, const char *argv[])
	{
		// Read/Parse my command line args.
		// its up to me to convert utf8 argv to Unicode.
		// TEST: "c:\tmp" -createsimpletest 1000 -quit
		// TEST: "c:\tmp" -createtest -quit
		// RETURN: false = halt further work.
	 
		if (argc <= 1)
		{
			std::cout << "No params! Use -help." << std::endl;
			return false;
		}

		for (int i = 1; i < argc; i++)
		{
			const char* argi = argv[i];

			if (!Util::StrCmpI(argi, "-help") || !Util::StrCmpI(argi, "-?"))
			{
				std::cout << "SSFI v" << std::to_string(kVersion) << " params:" << std::endl
					<< "-createsimpletest scale" << std::endl
					<< "-createtest" << std::endl
					<< "-DirHeadStart #" << std::endl
					<< "-t # // Set number of threads" << std::endl
					<< "-quit" << std::endl;
				return false;
			}

			if (!Util::StrCmpI(argi, "-createsimpletest"))
			{
				if (++i >= argc)
				{
					std::cout << "Invalid -t param! Use -help." << std::endl;
					return false;
				}

				CreateTestData(true, std::atoi(argv[i]));
				continue;
			}
			if (!Util::StrCmpI(argi, "-createtest"))
			{
				CreateTestData(false, 1234);
				continue;
			}
			if (!Util::StrCmpI(argi, "-quit"))	// don't do normal processing
			{
				return false;
			}

			// experiments.

			if (!Util::StrCmpI(argi, "-DirHeadStart"))	// don't do normal processing
			{
				if (++i >= argc)
				{
					std::cout << "Invalid -t param! Use -help." << std::endl;
					return false;
				}
				this->DirHeadStart = std::atoi(argv[i]);
				continue;
			}

			if (!Util::StrCmpI(argi, "-SerialFileReads") || !Util::StrCmpI(argi, "-Ser"))	//  
			{
				this->SerialFileReads = true;
				continue;
			}
			
			if (!std::strcmp(argi, "-t"))
			{
				if (++i >= argc)
				{
					std::cout << "Invalid -t param! Use -help." << std::endl;
					return false;
				}
				this->MaxThreads = std::atoi(argv[i]);
				continue;
			}

			if (argi[0] == '-')
			{
				// BAD!
				std::cout << "Invalid param " << std::quoted(argi) << "! Use -help." << std::endl;
				return false;
			}

			this->Dir = argi;	// might be quoted to contain spaces! assume quotes are stripped automatically.  https://www.geeksforgeeks.org/command-line-arguments-in-c-cpp/
		}

		if (this->MaxThreads <= 0)
		{
			this->MaxThreads = std::thread::hardware_concurrency();	// default = match the number of cores.
		}
		if (this->Dir.empty())
		{
			// Just get current file dir??
			std::cout << "No directory specified! Use -help." << std::endl;
			return false;
		}

		assert(this->MaxThreads>0);	
		return true;
	}
}
