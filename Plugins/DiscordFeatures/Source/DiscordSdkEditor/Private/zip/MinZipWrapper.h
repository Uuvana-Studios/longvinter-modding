// Copyright Pandores Marketplace 2021. All Rights Reserved

// Used to unzip the Discord Game SDK downloaded from Discord's servers.

#if !PLATFORM_MAC && !PLATFORM_IOS && !PLATFORM_ANDROID

#include "CoreMinimal.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "minizip/unzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME     (256)

enum class EZipResult : uint8
{
	Ok						= 0,
	GetInfoFailed			= 1,
	GetFileInfoFailed		= 2,
	GoToNextFileFailed		= 3,
	MemoryError				= 4,
	NewDirFailed			= 5,
	TargetExists			= 6,
	PasswordError			= 7,
	ReadCurrentFileFailed	= 8,
	WriteToFileFailed		= 9,
	CloseFileFailed			= 10
};

namespace
{
	bool mymkdir(const FString& DirName)
	{
		return FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*DirName);
	}

	bool makedir(const FString& Path)
	{
		const FString Directory = FPaths::GetPath(Path);
		return mymkdir(Directory);
	}
}

class FMinZip
{
public:
	FMinZip(const FString& Path)
	{
		size_buf = WRITEBUFFERSIZE;
		buf = (void*)FMemory::Malloc(size_buf);
		uf = unzOpen64(TCHAR_TO_UTF8(*Path));
		From = Path;
	}

	~FMinZip()
	{
		if (buf)
		{
			FMemory::Free(buf);
		}
		unzClose(uf);
	}
private:
	void* buf;
	uint32 size_buf;
	unzFile uf;
	FString From;
	char filename_inzip[256];

	EZipResult do_extract_currentfile(const FString& Target, const FString& Password)
	{
		char* filename_withoutpath;	
		char* p;

		unz_file_info64 file_info;
		uLong ratio = 0;

		int32 err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
		if (err != UNZ_OK)
		{
			return EZipResult::GetFileInfoFailed;
		}

		if (buf == NULL)
		{
			return EZipResult::MemoryError;
		}

		p = filename_withoutpath = filename_inzip;
		while ((*p) != '\0')
		{
			if (((*p) == '/') || ((*p) == '\\'))
			{
				filename_withoutpath = p + 1;
			}
			p++;
		}

		if ((*filename_withoutpath) == '\0')
		{
			//creating directory: %s\n", filename_inzip
			if (!mymkdir(filename_inzip))
			{
				return EZipResult::NewDirFailed;
			}
		}
		else
		{
			const char* write_filename;
			int32 skip = 0;

			write_filename = filename_inzip;

			if (Password.Len() <= 0)
			{
				err = unzOpenCurrentFile(uf);
			}
			else
			{
				err = unzOpenCurrentFilePassword(uf, TCHAR_TO_UTF8(*Password));
			}

			if (err != UNZ_OK)
			{
				return EZipResult::PasswordError;
			}

			const bool bForce = true;
			if (FPaths::FileExists(UTF8_TO_TCHAR(write_filename)))
			{
				if (!bForce)
				{
					return EZipResult::TargetExists;
				}
			}

			/* some zipfile don't contain directory alone before file */
			if (filename_withoutpath != (char*)filename_inzip)
			{
				char c = *(filename_withoutpath - 1);
				*(filename_withoutpath - 1) = '\0';
				makedir(UTF8_TO_TCHAR(write_filename));
				*(filename_withoutpath - 1) = c;
			}
			
			const FString TargetPath = Target / UTF8_TO_TCHAR(write_filename);

			TUniquePtr<FArchive> Ar = TUniquePtr<FArchive>(IFileManager::Get().CreateFileWriter(*TargetPath, 0));
			if (!Ar)
			{
				return EZipResult::WriteToFileFailed;
			}

			do
			{
				err = unzReadCurrentFile(uf, buf, size_buf);
				if (err < 0)
				{
					return EZipResult::ReadCurrentFileFailed;
				}
				
				if (err > 0)
				{
					Ar->Serialize((uint8*)buf, err);
				}
			} 
			while (err > 0);
			
			Ar->Close();
		}

		return EZipResult::Ok;
	}

public:
	EZipResult Extract(const FString& To, const FString& Password)
	{
		unz_global_info64 gi;

		int32 err = unzGetGlobalInfo64(uf, &gi);
		if (err != UNZ_OK)
		{
			return EZipResult::GetInfoFailed;
		}

		for (int32 i = 0; i < gi.number_entry; i++)
		{
			const EZipResult Result = FMinZip::do_extract_currentfile(To, Password);

			unzCloseCurrentFile(uf);

			if (Result != EZipResult::Ok)
			{
				return Result;
			}

			if ((i + 1) < gi.number_entry)
			{
				err = unzGoToNextFile(uf);
				if (err != UNZ_OK)
				{
					return EZipResult::GoToNextFileFailed;
				}
			}
		}

		return EZipResult::Ok;
	}
};


#undef CASESENSITIVITY 
#undef WRITEBUFFERSIZE 
#undef MAXFILENAME  

#endif // !PLATFORM_MAC && !PLATFORM_IOS && !PLATFORM_ANDROID
