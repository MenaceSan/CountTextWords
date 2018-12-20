// ssfi.cpp : This file contains the 'main' function. Program execution begins and ends there.
// SSFI By Dennis Robinson (dennis@menasoft.com)

#include "pch.h"
#include "cApp.h"

int main(int argc, const char *argv[])
{
	// "c:\tmp" -t 1

	try
	{
		SSFI::cApp app(argc, argv);
	}
	catch (std::exception& ex)
	{
		std::cout << "Error:" << std::endl << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...)
	{
		std::cout << "Error: Failed?";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;	// success
}
