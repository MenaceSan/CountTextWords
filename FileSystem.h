#pragma once
#include "Util.h"

#ifdef USE_FILESYSTEME
#ifdef _WIN32
#include <windows.h>		// WIN32_FIND_DATAW WIN32_FIND_DATAA
// #define USE_UNICODE
#elif defined(__linux__)
#include <dirent.h>		// DIR
struct stat;
#else
#error NOOS
#endif

#ifdef USE_FILESYSTEMX
#include <experimental/filesystem> // like std::filesystem	// NEEDs C++17 ! Weak support for Ubuntu 2018f. Only path seems to work.
#endif

namespace FileSystem
{
	// c++17 The STL <filesystem> isn't universally available. The <experimental/filesystem> lacks decent functionality. Only path?
	// Build my own minimal emulation of <filesystem> 

#ifdef USE_UNICODE
	typedef wchar_t FILECHAR_t;
	typedef std::wstring FILESTR_t;
#define TOFILESTR(s) Util::CvtW(s)
#define _FN(f)	f##W
#else
	typedef char FILECHAR_t;
	typedef std::string FILESTR_t;
#define TOFILESTR(s) Util::CvtA(s)
#define _FN(f)	f##A
#endif

#ifdef _WIN32 
	typedef ULONGLONG FILE_SIZE_t;	//!< similar to STREAM_POS_t size_t
	typedef _FN(WIN32_FIND_DATA) CFileStatusSys;
#elif defined(__linux__)
	typedef uintmax_t FILE_SIZE_t;	//!< similar to STREAM_POS_t size_t
	typedef struct stat CFileStatusSys;
#endif

	enum FILEATTR_TYPE_
	{
		//! @enum Gray::FILEATTR_TYPE_
		//! FAT, FAT32 and NTFS file attribute flags. translated to approximate __linux__ NFS
		FILEATTR_None,
		FILEATTR_ReadOnly = 0x0001,	//!< FILE_ATTRIBUTE_READONLY. __linux__ permissions for user ?
		FILEATTR_Hidden = 0x0002,	//!< FILE_ATTRIBUTE_HIDDEN. __linux__ starts with .
		FILEATTR_System = 0x0004,	//!< FILE_ATTRIBUTE_SYSTEM

		FILEATTR_NormalMask = 0x000F,	//!< (FILEATTR_ReadOnly|FILEATTR_Hidden|FILEATTR_System)

		FILEATTR_Directory = 0x0010,	//!< FILE_ATTRIBUTE_DIRECTORY
		FILEATTR_Archive = 0x0020,	//!< FILE_ATTRIBUTE_ARCHIVE = this has been changed. (needs to be archived) not yet backed up.
		FILEATTR_Volume = 0x0040,	//!< FILE_ATTRIBUTE_DEVICE = some sort of device. not a file or dir. e.g. COM1
		FILEATTR_Normal = 0x0080,	//!< FILE_ATTRIBUTE_NORMAL = just a file.

		// NTFS only flags. (maybe Linux)
		FILEATTR_Temporary = 0x0100,	//!< FILE_ATTRIBUTE_TEMPORARY
		FILEATTR_Link = 0x0400,			//!< FILE_ATTRIBUTE_REPARSE_POINT = a link. This file doesn't really exist locally but is listed in the directory anyhow.
		FILEATTR_Compress = 0x0800,		//!< FILE_ATTRIBUTE_COMPRESSED. this is a file that will act like a ATTR_directory. (sort of)
	};
	typedef unsigned int FILEATTR_MASK_t;	// FILEATTR_TYPE_ mask

#ifdef USE_FILESYSTEMX
	using namespace std::experimental::filesystem;	// path
#else

	class path : public FILESTR_t
	{
		// Represent a file name/path.

	public:
#ifdef _WIN32
		static constexpr FILECHAR_t preferred_separator = '\\';
#elif defined(__linux__)
		static constexpr FILECHAR_t preferred_separator = '/';
#endif

	public:

		std::string string() const
		{
			// convert the native path from this instance into a std::string
			return Util::CvtA(*this);
		}

		int compare(const char* p2) const
		{
			// 0 = is same. case sensitive.
			const FILECHAR_t* p1 = c_str();
			int i = 0;
			for (; p1[i] != '\0'; i++)
			{
				if (p1[i] != p2[i])
					break;
			}
			return p1[i] - p2[i];
		}

		int compare(const wchar_t* p2) const
		{
			// 0 = is same ? case sensitive.
			const FILECHAR_t* p1 = c_str();
			int i = 0;
			for (; p1[i] != '\0'; i++)
			{
				if (p1[i] != p2[i])
					break;
			}
			return p1[i] - p2[i];
		}

		path extension() const
		{
			// Find extension at last dot.
			auto n1 = find_last_of(preferred_separator);
			if (n1 == FILESTR_t::npos)
				n1 = 0;
			else
				n1++;
			auto n2 = find_last_of('.');
			if (n2 <= n1 || n2 == FILESTR_t::npos)
				return "";
			return path(substr(n2, size() - n2));
		}

		void appendSep()
		{
			FILECHAR_t sep[2];
			sep[0] = preferred_separator;
			sep[1] = '\0';
			*this += sep;
		}
		void append(const wchar_t* p)
		{
			// prefered_seperator
			if (!empty() && c_str()[length() - 1] != preferred_separator)
			{
				appendSep();
			}
			*this += path(p);
		}
		void append(const char* p)
		{
			// prefered_seperator
			if (!empty() && c_str()[length() - 1] != preferred_separator)
			{
				appendSep();
			}
			*this += path(p);
		}

		path()
		{}
		explicit path(FILESTR_t s)
			: FILESTR_t(s)
		{
		}

		path(const wchar_t* p)
			: FILESTR_t(TOFILESTR(p))
		{
		}
		path(const char* p)
			: FILESTR_t(TOFILESTR(p))
		{
		}
	};

#endif

	class directory_entry
	{
		// Represent a file or folder in a directory.
		friend class directory_iterator;

		path m_sFilePath;	// full path.
		path m_sFileName;	// name only.
		FILE_SIZE_t m_Size;
		FILEATTR_MASK_t m_Attributes = 0;

	protected:
		bool isDots() const
		{
			//! ignore the . and .. that old systems can give us.
			const FILECHAR_t* p = m_sFileName.c_str();
			if (p[0] != '.')
				return false;
			if (p[1] == '\0')
				return true;
			if (p[1] != '.')
				return false;
			if (p[2] == '\0')
				return true;
			return false;
		}
		void InitFileStatus(const CFileStatusSys& filestat);

		static bool IsLinuxHidden(const path& path)
		{
			//! Is this a hidden file on __linux__ (NFS) ?
			return path.empty() || path.c_str()[0] == '.';
		}
		bool UpdateLinuxHidden(const path& path)
		{
			//! Is this a __linux__ (NFS) hidden file name ? starts with dot.
			if (IsLinuxHidden(path))
			{
				m_Attributes |= FILEATTR_Hidden;
				return true;
			}
			return false;
		}

	public:
		operator const path&() const
		{
			return m_sFilePath;
		}
		uintmax_t file_size() const
		{
			return m_Size;
		}
		bool is_directory() const
		{
			return m_Attributes & FILEATTR_Directory;
		}
		bool is_regular_file() const
		{
			// FILEATTR_Normal
			return 0 == (m_Attributes & (FILEATTR_Directory | FILEATTR_Link | FILEATTR_Hidden | FILEATTR_Volume | FILEATTR_System));
		}
	};

	class directory_iterator
	{
		// Walk a directory listing.

		path m_sDirPath;				//!< The dir we are reading.
		directory_entry m_FileEntry;	//!< Current entry

#ifdef _WIN32
		_FN(WIN32_FIND_DATA) m_FindInfo;	//!< store current state of files search.
		HANDLE m_hContext;				//!< Handle for my search. NOT OSHandle, uses FindClose()
#elif defined(__linux__)
	public:
		bool m_bReadStats;				//!< e.g. "/proc" directory has no extra stats. don't read them.
	private:
		DIR* m_hContext;				//!< Handle for my search/enum.
#else
#error NOOS
#endif

	protected:
		bool isContextOpen() const;
		void CloseContext();
		bool FindFileNext(bool bFirst = false);
		bool FindFileOpen();

	public:
		directory_iterator()
		{}
		directory_iterator(const path& _Path);

		const directory_entry& operator*() const noexcept // strengthened
		{
			return m_FileEntry;
		}
		const directory_entry * operator->() const noexcept // strengthened
		{
			return &m_FileEntry;
		}
		bool operator!=(const directory_iterator& _Rhs) const noexcept // strengthened
		{
			return m_FileEntry.m_sFilePath.compare(_Rhs.m_FileEntry.m_sFilePath.c_str()) != 0;
		}
		directory_iterator& operator++()
		{
			// iterator to next
			FindFileNext(false);
			return *this;
		}
	};

	inline directory_iterator begin(directory_iterator _Iter) noexcept
	{
		return (_Iter);
	}

	inline directory_iterator end(directory_iterator) noexcept
	{
		return {};	// mark the end of the list. CloseContext();
	}
}

#endif

