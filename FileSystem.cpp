#include "pch.h"

#ifdef USE_FILESYSTEME
#include "FileSystem.h"

#ifdef __linux__
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <sys/statfs.h>
#endif

namespace FileSystem
{
	void directory_entry::InitFileStatus(const CFileStatusSys& fileStatus)
	{
		//! convert from OS native format.
#ifdef _WIN32
		m_Size = fileStatus.nFileSizeLow;
		if (fileStatus.nFileSizeHigh != 0)
		{
			m_Size |= ((FILE_SIZE_t)fileStatus.nFileSizeHigh) << 32;
		}
		m_Attributes = (FILEATTR_MASK_t)fileStatus.dwFileAttributes; // truncated!
#elif defined(__linux__)
		// http://linux.die.net/man/2/stat
		// hidden file start with .
		m_Size = fileStatus.st_size;
		m_Attributes = 0; // check the read,write,execute bits?
		if (S_ISREG(fileStatus.st_mode))
		{
			m_Attributes |= FILEATTR_Normal;
		}
		else if (S_ISDIR(fileStatus.st_mode))
		{
			m_Attributes |= FILEATTR_Directory;
		}
		else if (S_ISLNK(fileStatus.st_mode))
		{
			m_Attributes |= FILEATTR_Link;
		}
		else	// S_ISBLK, S_ISSOCK, S_ISCHR, S_ISFIFO
		{
			m_Attributes |= FILEATTR_Volume;	// device of some sort.
		}
#endif
	}

	directory_iterator::directory_iterator(const path& _Path)
		: m_sDirPath(_Path)
#ifdef _WIN32
		, m_hContext(INVALID_HANDLE_VALUE)
#elif defined(__linux__)
		, m_bReadStats(true)
		, m_hContext(nullptr)
#endif
	{
		FindFileOpen();
		FindFileNext(true);
	}

	bool directory_iterator::isContextOpen() const
	{
#ifdef _WIN32
		if (m_hContext == INVALID_HANDLE_VALUE)
			return false;
#elif defined(__linux__)
		if (m_hContext == nullptr)
			return false;
#endif
		return true;
	}

	void directory_iterator::CloseContext()
	{
		if (!isContextOpen())
			return;
#ifdef _WIN32
		::FindClose(m_hContext);
		m_hContext = INVALID_HANDLE_VALUE;
#elif defined(__linux__)
		::closedir(m_hContext);
		m_hContext = nullptr;
#endif
		m_FileEntry.m_sFileName = "";
		m_FileEntry.m_sFilePath = "";
	}

	bool directory_iterator::FindFileNext(bool bFirst)
	{
		//! Read the next file in the directory list.
		//! ASSUME cFileFind::FindFile() was called.
		//! @return
		//!  HRESULT_WIN32_C(ERROR_NO_MORE_ITEMS) = no more files
		//! @note UNICODE files are converted to '?' chars if calling the non UNICODE version.

		if (!isContextOpen())
		{
			return false; // HRESULT_WIN32_C(ERROR_NO_MORE_ITEMS);
		}

#ifdef _WIN32
		if (!bFirst)
		{
			if (!_FN(::FindNextFile)(m_hContext, &m_FindInfo))
			{
				CloseContext();
				return false; // HRESULT_WIN32_C(ERROR_NO_MORE_ITEMS);
			}
		}

		m_FileEntry.m_sFileName = m_FindInfo.cFileName;		// How should we deal with UNICODE names ?
		if (m_FileEntry.isDots())	// ignore the . and .. that old systems can give us.
		{
			return FindFileNext(false);
		}
 
		m_FileEntry.m_sFilePath = m_sDirPath;
		m_FileEntry.m_sFilePath.append(m_FileEntry.m_sFileName.c_str());
		m_FileEntry.InitFileStatus(m_FindInfo);
 
#elif defined(__linux__)

		(void)bFirst;   // silence UNREFERENCED_PARAMETER() warningssfi /tmp

		struct dirent* pFileInfo = ::readdir(m_hContext);
		if (pFileInfo == nullptr)
		{
			CloseContext();
			return false; // HRESULT_WIN32_C(ERROR_NO_MORE_ITEMS);
		}

		m_FileEntry.m_sFileName = pFileInfo->d_name;
		if (m_FileEntry.isDots())
		{
			return FindFileNext(false);
		}

		m_FileEntry.m_sFilePath = m_sDirPath;
		m_FileEntry.m_sFilePath.append(m_FileEntry.m_sFileName.c_str());

		if (m_bReadStats)	// some dirs don't have stat() ability. e.g. "/proc"
		{
			// dirent() doesn't have size stuff...it's in stat()
			// use ::lstat(m_FileEntry.m_sFilePath, &fileStatus) to follow links.
			CFileStatusSys fileStatus;
			int iRet = ::stat(m_FileEntry.m_sFilePath.c_str(), &fileStatus);
			if (iRet != 0)
			{
				return false; //  HResult::GetLastDef(HRESULT_WIN32_C(ERROR_FILE_NOT_FOUND));
			}
			m_FileEntry.InitFileStatus(fileStatus);
		}
#endif

		m_FileEntry.UpdateLinuxHidden(m_FileEntry.m_sFileName);
		return true;	// S_OK
	}

	bool directory_iterator::FindFileOpen()
	{
		//! start a sequential read of the files in a list of possible matches.
		//! @return
		//!  false = no files. HRESULT_WIN32_C(ERROR_NO_MORE_ITEMS) 

		CloseContext();
		assert(!isContextOpen());

#ifdef _WIN32
		// in _WIN32 wildcard filter is built in.
		std::memset(&m_FindInfo, 0, sizeof(m_FindInfo));
		m_FindInfo.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;	 // docs say this is not needed

		path sFilePath = m_sDirPath;
		sFilePath.append("*");
		m_hContext = _FN(::FindFirstFile)(sFilePath.c_str(), &m_FindInfo);

#elif defined(__linux__)
		// Need to strip out the *.EXT part. just need the dir name here.
		m_hContext = ::opendir(m_sDirPath.c_str());
#endif

		if (!isContextOpen())
		{
			return false; // HResult::GetLastDef(HRESULT_WIN32_C(ERROR_PATH_NOT_FOUND));
		}

		return true;	// S_OK
	}
}

#endif
