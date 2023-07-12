#include "StringUtils.hpp"
#include <Windows.h>

void rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
        }).base(), s.end());
}

void CopyToClipboard(std::string& source)
{
    if (OpenClipboard(nullptr))
    {
        HGLOBAL clipbuffer;
        char* buffer;
        EmptyClipboard();
        clipbuffer = GlobalAlloc(GMEM_DDESHARE, source.length() + 1);
        buffer = (char*)GlobalLock(clipbuffer);
        strcpy_s(buffer, source.length() + 1,source.c_str());
        GlobalUnlock(clipbuffer);
        SetClipboardData(CF_TEXT, clipbuffer);
        CloseClipboard();
    }
}
