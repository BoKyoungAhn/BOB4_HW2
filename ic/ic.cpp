#include "stdafx.h"
#include <stdint.h>
#include <conio.h>
#include "StopWatch.h"
#include "mmio.h"
#include <crtdbg.h>
#include <Windows.h>
#include "FileIoHelper.h"

bool OpenFileContext(IN PCWSTR FilePath, IN bool ReadOnly, OUT PFILE_CTX& Ctx, IN PLARGE_INTEGER pOffset, DWORD dwoffsetSize)
{
	_ASSERTE(NULL != FilePath);
	if (NULL == FilePath) return false;
	if (!is_file_existsW(FilePath)) return false;

	_ASSERT(NULL == Ctx);
	if (NULL != Ctx) return false;

	Ctx = (PFILE_CTX)malloc(sizeof(FILE_CTX));
	if (NULL == Ctx) return false;

	RtlZeroMemory(Ctx, sizeof(FILE_CTX));
	bool ret = false;

#pragma warning (disable: 4127)
	do
	{
		Ctx->FileHandle = CreateFileW(
			(LPCWSTR)FilePath,
			(true == ReadOnly) ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE,
			NULL,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);
		if (INVALID_HANDLE_VALUE == Ctx->FileHandle)
		{
			log_err
				"CreateFile(%ws) failed, gle = %u",
				FilePath,
				GetLastError()
				log_end
				break;
		}
		LARGE_INTEGER fileSize;
		if (TRUE != GetFileSizeEx(Ctx->FileHandle, &fileSize))
		{
			log_err
				"%ws, can not get file size, gle = %u",
				FilePath,
				GetLastError()
				log_end
				break;
		}
		_ASSERTE(fileSize.HighPart == 0);
		if (fileSize.HighPart > 0)
		{
			log_err
				"file size = %I64d (over 4GB) can not handle. use FileIoHelperClass",
				fileSize.QuadPart
				log_end
				break;
		}

		Ctx->FileSize = (DWORD)fileSize.QuadPart;
		Ctx->FileMap = CreateFileMapping(
			Ctx->FileHandle,
			NULL,
			(true == ReadOnly) ? PAGE_READONLY : PAGE_READWRITE,
			0,
			0,
			NULL
			);
		if (NULL == Ctx->FileMap)
		{
			log_err
				"CreateFileMapping(%ws) failed, gle = %u",
				FilePath,
				GetLastError()
				log_end
				break;
		}

		Ctx->FileView = (PCHAR)MapViewOfFile(
			Ctx->FileMap,
			(true == ReadOnly) ? FILE_MAP_READ : FILE_MAP_WRITE,
			(pOffset) ? pOffset->HighPart : 0,
			(pOffset) ? pOffset->LowPart : 0,
			dwoffsetSize
			);
		if (Ctx->FileView == NULL)
		{
			log_err
				"MapViewOfFile(%ws) failed, gle = %u",
				FilePath,
				GetLastError()
				log_end
				break;
		}
		ret = true;
	} while (FALSE);
#pragma warning (default: 4127)
	if (!ret)
	{
		if (INVALID_HANDLE_VALUE != Ctx->FileHandle) CloseHandle(Ctx->FileHandle);
		if (NULL != Ctx->FileMap) CloseHandle(Ctx->FileMap);
		if (NULL != Ctx->FileView) UnmapViewOfFile(Ctx->FileView);
		free(Ctx);
	}
	return ret;
}
void CloseFileContext(IN PFILE_CTX& Ctx)
{
//	_ASSERTE(NULL != Ctx);
	if (NULL == Ctx) return;

	if (INVALID_HANDLE_VALUE != Ctx->FileHandle) CloseHandle(Ctx->FileHandle);
	if (NULL != Ctx->FileMap) CloseHandle(Ctx->FileMap);
	if (NULL != Ctx->FileView) UnmapViewOfFile(Ctx->FileView);
	free(Ctx);
}

int _tmain(int argc, _TCHAR* argv[])
{
	LARGE_INTEGER large = { 0 };
	PFILE_CTX f = 0;
	
	StopWatch sw2;
	sw2.Start();
	create_very_big_file(L"Above4GB.txt", 5000);
	sw2.Stop();
	print("[create over 4Gb File ] time elapsed = %f\n", sw2.GetDurationSecond());
	CloseFileContext(f);

	//copy File
	system("copy Above4GB.txt CoPy_Above4GB.txt");
	system("exit(1)");

	return 0;
}

