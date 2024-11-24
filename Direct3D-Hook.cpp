// Direct3D-Hook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "apihijack.h"
#include "Direct3D-Hook.h"
#include <stdio.h>
#include <string.h>
#include <wchar.h>

// Function prototypes.
HANDLE WINAPI MyCreateFileA(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
DWORD WINAPI MyGetFileAttributesA(LPCTSTR lpFileName);

SDLLHook D3DHook = 
{
	/*"KERNEL32.DLL",*/
	false, NULL,		// Default hook disabled, NULL function pointer.
	{
		{"Kernel32.DLL", "CreateFileA",MyCreateFileA},
		{"Kernel32.DLL", "GetFileAttributesA",MyGetFileAttributesA},
		{ NULL, NULL,NULL }
	}
};

bool FindUnicodeFile(LPCSTR lpFileName, LPWSTR lpwFileName) {
	MultiByteToWideChar(CP_ACP, 0, lpFileName, -1, lpwFileName, MAX_PATH);

	WIN32_FIND_DATAW findData;
	HANDLE hFind = FindFirstFileW(lpwFileName, &findData);

	if (hFind == INVALID_HANDLE_VALUE) return false;

	wchar_t wszMsg[MAX_PATH+50];
	swprintf(wszMsg, L"FindFirstFileW: %s\n", findData.cFileName);
	OutputDebugStringW(wszMsg);

	LPWSTR pwszFileName=wcsrchr(lpwFileName,'\\');
	if (pwszFileName) pwszFileName++; else pwszFileName = lpwFileName;

	wcscpy(pwszFileName, findData.cFileName);
	FindClose(hFind);

	return true;
}

HANDLE __stdcall MyCreateFileA(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
	char szMsg[MAX_PATH+50];
	sprintf(szMsg, "MyCreateFileA: %s\n", lpFileName);
	OutputDebugString(szMsg);

	if (strchr(lpFileName,'?')) {
		wchar_t wszFileName[MAX_PATH];
		FindUnicodeFile(lpFileName, wszFileName);

		return CreateFileW(wszFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);;
	} else {
		return CreateFile(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	}
}

DWORD __stdcall MyGetFileAttributesA(LPCTSTR lpFileName) {
	char szMsg[MAX_PATH+50];
	sprintf(szMsg, "GetFileAttributesA: %s\n", lpFileName);
	OutputDebugString(szMsg);

	if (strchr(lpFileName,'?')) {
		wchar_t wszFileName[MAX_PATH];
		FindUnicodeFile(lpFileName, wszFileName);

		return GetFileAttributesW(wszFileName);
	} else {
		return GetFileAttributes(lpFileName);
	}
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if(ul_reason_for_call == DLL_PROCESS_ATTACH)  // When initializing....
	{
		// We don't need thread notifications for what we're doing.  Thus, get
		// rid of them, thereby eliminating some of the overhead of this DLL
		DisableThreadLibraryCalls(hModule);
		
		OutputDebugString("SPI DllMain DLL_PROCESS_ATTACH\n");

		HookAPICalls(&D3DHook);
	} else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		OutputDebugString("SPI DllMain DLL_PROCESS_DETACH\n");
		UnhookAPICalls(&D3DHook);
	}

	return TRUE;
}

// Susie parts
const char *plugin_info[2] = {
    "00EN", // Effect plugin
    "Unicode File Plugin for Susie Image Viewer"/*,
    "*.xxx", // File dialog extension
    "sample (*.xxx)", // File dialog extension information
	*/
};

int __stdcall GetPluginInfo(int infono, LPSTR buf, int buflen)
{
	char szDebug[MAX_PATH];
	sprintf(szDebug, "SPI GetPluginInfo info=%d buffer_length=%d\n", infono, buflen);
	OutputDebugString(szDebug);
	if (infono < 0 || infono >= (sizeof(plugin_info) / sizeof(const char *)))
	{
		*buf = 0;
		return 0;
	}

	int ret = _snprintf(buf, buflen, "%s", plugin_info[infono]);
	if (ret > buflen)
	{
		ret = buflen;
	}

	return ret;
}


int __stdcall IsSupported(LPSTR filename, DWORD dw)
{
	OutputDebugString("SPI IsSupported\n");
	return 0;
}

int __stdcall GetPictureInfo
    (LPSTR buf, long len, unsigned int flag, struct PictureInfo *lpInfo)
{
	OutputDebugString("SPI GetPictureInfo\n");
	return SPI_NOT_SUPPORT;
}

int __stdcall GetPicture(
    LPSTR buf, long len, unsigned int flag, 
    HANDLE *pHBInfo, HANDLE *pHBm,
    SPI_PROGRESS lpPrgressCallback, long lData)
{
	OutputDebugString("SPI GetPicture\n");
	return SPI_NOT_SUPPORT;
}

int __stdcall GetPreview(
  LPSTR buf, long len, unsigned int flag,
  HANDLE *pHBInfo, HANDLE *pHBm,
  SPI_PROGRESS lpPrgressCallback, long lData)
{
	OutputDebugString("SPI GetPreview\n");
	return SPI_NOT_SUPPORT;
}
