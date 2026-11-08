// openciv4: the user's Documents folder (OC4_DOCUMENTS overrides it, for tests).
#pragma once
#include <windows.h>
#include <cstdlib>
#include <string>
#define FOLDERID_Documents 0
inline HRESULT SHGetKnownFolderPath(int, DWORD, void*, PWSTR* out)
{
	const char* documents = std::getenv("OC4_DOCUMENTS");
	std::string path = documents ? documents : std::string(std::getenv("HOME") ? std::getenv("HOME") : ".") + "/Documents";
	auto* wide = static_cast<wchar_t*>(std::malloc((path.size() + 1) * sizeof(wchar_t)));
	const int n = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), int(path.size()), wide, int(path.size() + 1));
	wide[n] = 0;
	*out = wide;
	return 0;
}
inline void CoTaskMemFree(void* p) { std::free(p); }
