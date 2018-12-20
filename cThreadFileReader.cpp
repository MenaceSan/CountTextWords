#include "pch.h"
#include "cThreadFileReader.h"
#include "cApp.h"
#include "Util.h"

namespace SSFI
{
	void cThreadFileReader::FlushWord()
	{
		if (_word.length() > 0)	// last word to process.
		{
			App.Dict.AddWord(_word);
			_word.clear();
		}
	}

	void cThreadFileReader::ReadFile(const fsx::path& filePath)
	{
		std::ifstream file(filePath.string());		// is RAII.
		_word.clear();
		int nChars = 0;
		const int blockSizeMax = 16 * 1024;
		std::vector<char> block(blockSizeMax);

		while (this->App.PleaseExit != ExitNow)
		{
			if (file.eof())
			{
				break;
			}
			if (file.fail())
			{
				// Bad !
				std::cout << "Failed to read file " << Util::CvtA(filePath) << std::endl;
				return;
			}

			// read a block.
			if (App.Params.SerialFileReads)
			{
				App.Params.SerialFileReadMutex.lock();
			}
			file.read(&block[0], blockSizeMax);
 			if (App.Params.SerialFileReads)
			{
				App.Params.SerialFileReadMutex.unlock();
			}

			std::streamsize sizeBlock = file.gcount();
			for (int i = 0; i < sizeBlock; i++)
			{
				char ch = block[i];
				nChars++;

				// Words are delimited by any character other than A- Z, a-z, or 0-9. Words should be matched case insensitive.
				if (std::isalnum(ch))
				{
					_word += ch;	// append word.
				}
				else
				{
					FlushWord();	// end of a word?
				}
			}
		}

		FlushWord();	// partial word at end?
		App.FilesRead++;
	}

	void cThreadFileReader::Run()  // virtual
	{
		// Wait for PleaseExit or a file to process . If i find it, read it 
		while (this->App.PleaseExit != ExitNow)
		{
			cAppFileToRead file = App.FilesToRead.WaitForFile(this->App.PleaseExit);
			if (file.IsEmpty())
			{
				if (this->App.PleaseExit != WaitForMore)	// nothing left to do i guess. exit.
					break;
				continue;
			}
			ReadFile(file.Path);
		}
	}
}
