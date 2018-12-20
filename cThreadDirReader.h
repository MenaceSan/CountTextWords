#pragma once
#include "cThreadBase.h"

namespace SSFI
{
	class cThreadDirReader : public cThreadBase
	{
		// Read directory on a separate thread.

	protected:
		void ReadDir(fsx::path dirPath);
		virtual void Run();

	public:
		cThreadDirReader(cApp& app)
			: cThreadBase(app)
		{
		}
	};
}
