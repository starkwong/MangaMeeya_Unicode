// NewHook.cpp : 定義應用程式的進入點。
//

#include "stdafx.h"
#include "NewHook.h"
#include "Direct3D-Hook.h"
#define MAX_LOADSTRING 100

// 全域變數:
/*HINSTANCE hInst;								// 目前執行個體
TCHAR szTitle[MAX_LOADSTRING];					// 標題列文字
TCHAR szWindowClass[MAX_LOADSTRING];			// 主視窗類別名稱*/

// 這個程式碼模組中所包含之函式的向前宣告:
/*ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);*/

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	char szPath[MAX_PATH];
	char szEXE[MAX_PATH]={0};
	GetCurrentDirectory(MAX_PATH,szPath);
	strcat(szPath,"\\Hook.ini");

	GetPrivateProfileString("General","ProcessName",NULL,szEXE,MAX_PATH,szPath);
	if (*szEXE) {
		PROCESS_INFORMATION pi;
		STARTUPINFO si={sizeof(si)};
		char szPath[MAX_PATH]={0};
		//char* pszFile=strrchr(szEXE,'\\');
		if (strchr(szEXE,'\\')) {
			strcpy(szPath,szEXE);
			*strrchr(szPath,'\\')=0;
		}

		/*if (pszFile) {
			*pszFile=0;
			pszFile++;
		}*/

		InstallHook();
		SetTarget();

		if (CreateProcess(szEXE,NULL,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,*szPath?szPath:NULL,&si,&pi)) {
			CloseHandle(pi.hThread);

			WaitForSingleObject(pi.hProcess,INFINITE);

			CloseHandle(pi.hProcess);
		} else {
			MessageBox(NULL,"Failed Launching Application",szEXE,MB_ICONERROR);
		}

		RemoveHook();
	} else
		MessageBox(NULL,"No Application Specified",NULL,MB_ICONERROR);
 	// TODO: 在此置入程式碼。
	/*
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全域字串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_NEWHOOK, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 執行應用程式初始設定:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_NEWHOOK);
	InstallHook();
	SetTarget();

	// 主訊息迴圈:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	RemoveHook();
	return (int) msg.wParam;*/

}


#if 0
//
//  函式: MyRegisterClass()
//
//  用途: 登錄視窗類別。
//
//  註解:
//
//  只有當您希望此程式碼能相容比 Windows 95 的 'RegisterClassEx' 函式更早的 Win32 系統時，才會需要
// 加入及使用這個函式。您必須呼叫這個函式，讓應用程式取得與它相關的 '正確格式 (Well Formed)' 圖示。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_NEWHOOK);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_NEWHOOK;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   函式: InitInstance(HANDLE, int)
//
//   用途: 儲存執行個體控制代碼並且建立主視窗
//
//   註解:
//
//        在這個函式中，我們會將執行個體控制代碼儲存在全域變數中，
//        並且建立和顯示主程式視窗。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 將執行個體控制代碼儲存在全域變數中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函式: WndProc(HWND, unsigned, WORD, LONG)
//
//  用途:  處理主視窗的訊息。
//
//  WM_COMMAND	- 處理應用程式功能表
//  WM_PAINT	- 繪製主視窗
//  WM_DESTROY	- 傳送結束訊息然後返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// 剖析功能表選取項目:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此加入任何繪圖程式碼...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// [關於] 方塊的訊息處理常式。
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
#endif