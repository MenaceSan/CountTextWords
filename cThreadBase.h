#pragma once
#include "pch.h"

namespace SSFI
{
	class cApp;		// forward declare this.

	class cThreadBase
	{
		// Base class for some worker thread.
		// Base for cThreadDirReader or cThreadFileReader

	public:
		cApp& App;
		bool Running = false;

	protected:
		std::unique_ptr<std::thread> Thread;		// The worker thread. see Start().

	protected:
		cThreadBase(cApp& app)
			: App(app)
		{
		}

		virtual void Run() = 0;
		static void RunX(cThreadBase* pThis)
		{
			pThis->Running = true;
			try
			{
				pThis->Run();
				pThis->Running = false;
			}
			catch (std::exception& ex)
			{
				pThis->Running = false;
				std::cout << "Error: Thread Failed: " << ex.what() << std::endl;
			}
			catch (...)  
			{
				pThis->Running = false;
				std::cout << "Error: Thread Failed?";
			}
		}

	public:
		void Join()
		{
			if (!Thread)
				return;
			Thread->join();
		}

		void Start()
		{
			// Make Start() separate from construct to avoid construction race conditions.
			Thread = std::make_unique<std::thread>(RunX, this);
		}

		~cThreadBase()
		{
			assert(!Running);	// MUST stop before destruct.
		}
	};
}
