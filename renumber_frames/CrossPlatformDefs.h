#pragma once

#ifndef FOR_LINUX
#define CrossPlatformString std::wstring
#define CrossPlatformChar wchar_t
#define CrossPlatformPerror _wperror
#define CrossPlatformText(txt) L##txt
#define CrossPlatformCin std::wcin
#define CrossPlatformCout std::wcout
#define CrossPlatformCerr std::wcerr
#define CrossPlatformNumberToString std::to_wstring
#define CrossPlatformCaseInsensitiveTextCompare(a,b) _wcsicmp(a, b)
#else
#define CrossPlatformString std::string
#define CrossPlatformChar char
#define CrossPlatformPerror perror
#define CrossPlatformText(txt) txt
#define CrossPlatformCin std::cin
#define CrossPlatformCout std::cout
#define CrossPlatformCerr std::cerr
#define CrossPlatformNumberToString std::to_string
#define CrossPlatformCaseInsensitiveTextCompare(a,b) strcasecmp(a, b)
#endif
