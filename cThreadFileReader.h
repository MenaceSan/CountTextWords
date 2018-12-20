#pragma once
#include "cThreadBase.h"

namespace SSFI
{ 
	class cThreadFileReader : public cThreadBase
	{
		// Read a file on a separate thread.
		// Q: We don't bother reading single files on more than one thread at a time. Assume files are serial on a single device. SAN array would make this NOT true.

	protected:
		void FlushWord();
		void ReadFile(const fsx::path& filePath);
		virtual void Run();

	private:
		std::string _word;

	public:
		cThreadFileReader(cApp& app)
			: cThreadBase(app)
		{
		}
	};
}
