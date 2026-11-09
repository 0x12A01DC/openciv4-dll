// openciv4: TCHAR is char off Windows, as in a non-Unicode Windows build.
#pragma once
#include <cstring>
#include <strings.h>
#define _T(s) s
#define _TEXT(s) s
#define _tcscmp strcmp
#define _tcsicmp strcasecmp
#define _tcslen strlen
#define _tcscpy strcpy
#define _tcsncpy strncpy
