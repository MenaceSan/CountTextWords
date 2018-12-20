#include "pch.h"
#include "cThreadDirReader.h"
#include "cApp.h"

namespace SSFI
{
	void cThreadDirReader::ReadDir(fsx::path dirPath)
	{
		// recursive descent.

		for (auto& dirEnt : fse::directory_iterator(dirPath))
		{
			if (this->App.PleaseExit != WaitForMore)
				return;

			const fsx::path& pathEnt = dirEnt;
			if (dirEnt.is_directory())
			{
				ReadDir(pathEnt);	// recurse.
				continue;
			}

			if (dirEnt.is_regular_file() && pathEnt.extension().compare(cAppParams::kExt) == 0)		// case sensitive. Last dot.
			{
				App.FilesToRead.AddFile(dirEnt, App);
			}
		}
	}

	void cThreadDirReader::Run()	// virtual
	{
		// Read Dir until we are done or PleaseExit.
		std::cout << "Start cThreadDirReader" << std::endl ;

		ReadDir(App.Params.Dir);
		App.PleaseExit = ExitWhenDone;	// Tell the cThreadFileReader(s) to exit when they are done.
		App.FilesToRead.PleaseExitChanged();	// poke other threads to cause them to re-evaluate.
	}
}
