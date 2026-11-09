// openciv4: COM does not exist here; nothing the engine does off Windows needs it.
#pragma once
#include <windows.h>
#define COINIT_MULTITHREADED 0
#define COINIT_APARTMENTTHREADED 2
inline int CoInitializeEx(void*, int) { return 0; }
