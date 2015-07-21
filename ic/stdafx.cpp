#include "stdafx.h"

void print(_In_ const char* fmt, _In_ ...)
{
    char log_buffer[2048];
    va_list args;

    va_start(args, fmt);
    HRESULT hRes = StringCbVPrintfA(log_buffer, sizeof(log_buffer), fmt, args);
    if (S_OK != hRes)
    {
        fprintf(
            stderr,
            "%s, StringCbVPrintfA() failed. res = 0x%08x",
            __FUNCTION__,
            hRes
            );
        return;
    }

    OutputDebugStringA(log_buffer);
    fprintf(stdout, "%s \n", log_buffer);
}
bool is_file_existsW(_In_ const wchar_t* file_path)
{
    _ASSERTE(NULL != file_path);
    _ASSERTE(TRUE != IsBadStringPtrW(file_path, MAX_PATH));
    if ((NULL == file_path) || (TRUE == IsBadStringPtrW(file_path, MAX_PATH))) return false;

    WIN32_FILE_ATTRIBUTE_DATA info = { 0 };
    if (GetFileAttributesExW(file_path, GetFileExInfoStandard, &info) == 0)
        return false;
    else
        return true;
}
wchar_t* get_current_directory(void)
{
    wchar_t *buf = NULL;
    uint32_t buflen = 0;
    buflen = GetCurrentDirectoryW(buflen, buf);
    if (0 == buflen)
    {
        print("err ] GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
        return NULL;
    }

    buf = (PWSTR)malloc(sizeof(WCHAR) * buflen);
    if (0 == GetCurrentDirectoryW(buflen, buf))
    {
        print("err ] GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
        free(buf);
        return NULL;
    }

    return buf;

}