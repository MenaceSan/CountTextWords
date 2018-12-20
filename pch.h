// add headers that you want to pre-compile here

#ifndef PCH_H
#define PCH_H
#pragma once

#include <cassert>		// assert
#include <iostream>		// std::cout
#include <algorithm>    // std::transform
#include <cctype>		// std::toupper
#include <string>		// std::string
#include <cstring>		// std::strcmp
#include <memory>		// std::unique_ptr
#include <vector>		// std::vector
#include <thread>		// std::thread
#include <condition_variable>	// std::condition_variable
#include <mutex>		// std::mutex
#include <map>			// std::map
#include <exception>	// std::exception
#include <atomic>		// std::atomic
#include <algorithm>	// std::binary_search
#include <fstream>		// std::ofstream, std::ifstream
#include <queue>		// std::queue
#include <iomanip>		// std::quoted

#ifndef USE_FILESYSTEM	// indicate we want to use <filesystem> ? (defined(_MSC_VER) && _HAS_CXX17)
#define USE_FILESYSTEME	// no <filesystem> so need to emulate.
#endif

#ifdef USE_FILESYSTEME
#include "FileSystem.h"		// emulate my own version of std::filesystem
#ifdef USE_FILESYSTEMX		// use std::experimental::filesystem for path.
namespace fsx = std::experimental::filesystem;
namespace fse = FileSystem;
#else
namespace fsx = FileSystem;
namespace fse = FileSystem;
#endif
#else
// <filesystem> is not properly supported in some Linux distros. Weak support for Ubuntu 2018f. Also NEEDs C++17
#include <filesystem>	// std::filesystem	// NEEDs C++17 ! Only works for _MSC_VER?
namespace fsx = std::filesystem;
namespace fse = std::filesystem;
#endif
 
#endif //PCH_H
