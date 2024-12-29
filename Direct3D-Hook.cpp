// Direct3D-Hook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "apihijack.h"
#include "Direct3D-Hook.h"
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <malloc.h>

typedef struct RESOLVED_T {
	LPSTR pszANSI;
	LPWSTR pwszUnicode;
	struct RESOLVED_T* nextNode;
} RESOLVED_T, *LPRESOLVED_T, * const LPCRESOLVED_T;

typedef struct FAKEFINDDATA_T {
	HANDLE hFind;
	LPWIN32_FIND_DATAA lpFindData;
	struct FAKEFINDDATA_T* nextNode;
} FAKEFINDDATA_T, *LPFAKEFINDDATA_T, * const LPCFAKEFINDDATA_T;

LPRESOLVED_T lpResolved = NULL;
LPFAKEFINDDATA_T lpFakeFindData = NULL;

void AddResolved(LPCSTR pszANSI, LPCWSTR pwszUnicode);
void CleanResolved();
LPCRESOLVED_T findResolved(LPCSTR pcszANSI, BOOL exact);


// Function prototypes.
HANDLE WINAPI MyCreateFileA(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
DWORD WINAPI MyGetFileAttributesA(LPCTSTR lpFileName);
HANDLE WINAPI MyFindFirstFileA(LPCSTR lpFileName, __out LPWIN32_FIND_DATAA* lpFindFileData);
BOOL WINAPI MyFindClose(HANDLE hFindFile);

SDLLHook D3DHook = 
{
	/*"KERNEL32.DLL",*/
	false, NULL,		// Default hook disabled, NULL function pointer.
	{
		{"Kernel32.DLL", "CreateFileA",MyCreateFileA},
		{"Kernel32.DLL", "GetFileAttributesA",MyGetFileAttributesA},
		{"Kernel32.DLL", "FindFirstFileA",MyFindFirstFileA},
		{"Kernel32.DLL", "FindClose",MyFindClose},
		{ NULL, NULL,NULL }
	}
};

bool FindUnicodeFile(LPCSTR lpFileName, LPWSTR lpwFileName) {
	MultiByteToWideChar(CP_ACP, 0, lpFileName, -1, lpwFileName, MAX_PATH);

	LPCRESOLVED_T lpResolved = findResolved(lpFileName, FALSE);
	if (lpResolved) {
		OutputDebugStringA("FindUnicodeFile: Resolved path found\n");
		CopyMemory(lpwFileName, lpResolved->pwszUnicode, wcslen(lpResolved->pwszUnicode) * sizeof(wchar_t));
	}

	WIN32_FIND_DATAW findData;
	HANDLE hFind = FindFirstFileW(lpwFileName, &findData);

	if (hFind == INVALID_HANDLE_VALUE) return false;

	wchar_t wszMsg[MAX_PATH+50];
	swprintf(wszMsg, L"FindFirstFileW: %s\n", findData.cFileName);
	OutputDebugStringW(wszMsg);

	LPWSTR pwszFileName=wcsrchr(lpwFileName,'\\');
	if (pwszFileName) pwszFileName++; else pwszFileName = lpwFileName;

	wcscpy(pwszFileName, findData.cFileName);
	if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0) AddResolved(lpFileName, lpwFileName);
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

HANDLE WINAPI MyFindFirstFileA(LPCSTR lpFileName, __out LPWIN32_FIND_DATAA* lpFindFileData) {
	char szDebug[MAX_PATH + 100];
	sprintf(szDebug, "MyFindFirstFileA: %s\n", lpFileName);
	OutputDebugStringA(szDebug);

	wchar_t lpwFileName[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, lpFileName, -1, lpwFileName, MAX_PATH);

	if (LPCRESOLVED_T lpResolved = findResolved(lpFileName, FALSE)) {
		OutputDebugStringA("MyFindFirstFileA: Resolved path found\n");
		CopyMemory(lpwFileName, lpResolved->pwszUnicode, wcslen(lpResolved->pwszUnicode) * sizeof(wchar_t));
	}

	WIN32_FIND_DATAW findData;
	HANDLE hFind = FindFirstFileW(lpwFileName, &findData);

	if (hFind == INVALID_HANDLE_VALUE) return hFind;

	LPWIN32_FIND_DATAA lpFindA = (LPWIN32_FIND_DATAA) LocalAlloc(LPTR, sizeof(WIN32_FIND_DATAA));
	lpFindA->dwFileAttributes = findData.dwFileAttributes;
	lpFindA->ftCreationTime = findData.ftCreationTime;
	lpFindA->ftLastAccessTime = findData.ftLastAccessTime;
	lpFindA->ftLastWriteTime = findData.ftLastWriteTime;
	lpFindA->nFileSizeHigh = findData.nFileSizeHigh;
	lpFindA->nFileSizeLow = findData.nFileSizeLow;
	lpFindA->dwReserved0 = findData.dwReserved0;
	lpFindA->dwReserved1 = findData.dwReserved1;
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) findData.cFileName, -1, lpFindA->cFileName, MAX_PATH, "?", NULL);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) findData.cAlternateFileName, -1, lpFindA->cAlternateFileName, MAX_PATH, "?", NULL);

	lpFindFileData = &lpFindA;

	LPFAKEFINDDATA_T lpNewFakeFindData = (LPFAKEFINDDATA_T) LocalAlloc(LPTR, sizeof(FAKEFINDDATA_T));
	LPFAKEFINDDATA_T lpLastFakeFindData = lpFakeFindData;
	while (lpLastFakeFindData && lpLastFakeFindData->nextNode) {
		lpLastFakeFindData = lpLastFakeFindData->nextNode;
	}

	lpNewFakeFindData->hFind = hFind;
	lpNewFakeFindData->lpFindData = lpFindA;

	if (lpLastFakeFindData) {
		lpLastFakeFindData->nextNode = lpNewFakeFindData;
	} else {
		lpFakeFindData = lpNewFakeFindData;
	}

	return hFind;
}

BOOL WINAPI MyFindClose(HANDLE hFindFile) {
	LPFAKEFINDDATA_T lpLastFakeFindData = NULL;

	for (LPFAKEFINDDATA_T lpThisFakeFindData = lpFakeFindData; lpThisFakeFindData; lpLastFakeFindData = lpThisFakeFindData, lpThisFakeFindData = lpThisFakeFindData->nextNode) {
		if (lpThisFakeFindData->hFind == hFindFile) {
			OutputDebugStringA("MyFindClose: Freed fake find result\n");
			if (lpLastFakeFindData) {
				lpLastFakeFindData->nextNode = lpThisFakeFindData->nextNode;
			} else {
				lpFakeFindData = lpThisFakeFindData->nextNode;
			}
			LocalFree(lpThisFakeFindData->lpFindData);
			LocalFree(lpThisFakeFindData);
			break;
		}
	}

	return FindClose(hFindFile);
}

void AddResolved(LPCSTR pszANSI, LPCWSTR pwszUnicode) {
	LPRESOLVED_T newNode = (LPRESOLVED_T) LocalAlloc(LPTR, sizeof(RESOLVED_T));
	LPRESOLVED_T lastNode = lpResolved;
	int count = 1;

	while (lastNode) {
		count++;
		if (!strcmp(pszANSI, lastNode->pszANSI)) return;
		if (!lastNode->nextNode) break;
		lastNode = lastNode->nextNode;
	}

	size_t cbANSI = strlen(pszANSI) + 1;
	size_t cbUnicode = (wcslen(pwszUnicode) + 1) * sizeof(wchar_t);
	newNode->pszANSI = (LPSTR) LocalAlloc(LPTR, cbANSI);
	newNode->pwszUnicode = (LPWSTR) LocalAlloc(LPTR, cbUnicode);
	CopyMemory(newNode->pszANSI, pszANSI, cbANSI);
	CopyMemory(newNode->pwszUnicode, pwszUnicode, cbUnicode);
	if (!lastNode) {
		lpResolved = newNode;
	} else {
		lastNode->nextNode = newNode;
	}

	char szDebug[MAX_PATH];
	sprintf(szDebug, "AddResolved: Added resolve node #%d of %s\n", count, pszANSI);
	OutputDebugString(szDebug);
}

void CleanResolved() {
	int count = 0;

	while (lpResolved) {
		LPRESOLVED_T thisNode = lpResolved;
		LPRESOLVED_T nextNode = lpResolved->nextNode;
		LocalFree(thisNode->pszANSI);
		LocalFree(thisNode->pwszUnicode);
		LocalFree(thisNode);
		lpResolved = nextNode;
		count++;
	}

	char szDebug[MAX_PATH];
	sprintf(szDebug, "CleanResolved: Removed %d resolve nodes\n", count);
	OutputDebugString(szDebug);
}

LPCRESOLVED_T findResolved(LPCSTR pcszANSI, BOOL exact) {
	LPRESOLVED_T lastResolved = NULL;
	int lastScore = 0;

	for (LPRESOLVED_T node = lpResolved; node != NULL; node = node->nextNode) {
		if (!memcmp(pcszANSI, node->pszANSI, strlen(node->pszANSI) + (exact ? 1 : 0))) {
			int score = strlen(node->pszANSI);
			if (score > lastScore) {
				lastScore = score;
				lastResolved = node;
			}
		}
	}

	return lastResolved;
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
		CleanResolved();

		LPFAKEFINDDATA_T lpNextFakeFindData;
		while (lpFakeFindData) {
			OutputDebugStringA("DLL_PROCESS_DETACH: Freed one leaked fake find result\n");
			LocalFree(lpFakeFindData->lpFindData);
			lpNextFakeFindData = lpFakeFindData->nextNode;
			LocalFree(lpFakeFindData);
			lpFakeFindData = lpNextFakeFindData;
		}
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
