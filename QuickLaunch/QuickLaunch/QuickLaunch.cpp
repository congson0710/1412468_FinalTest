// QuickLaunch.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "QuickLaunch.h"
#include <shellapi.h>
#include <vector>
#include <tchar.h>
#include <strsafe.h>
#include <conio.h>
#include <fstream>
#include <iostream>
#include <commctrl.h>
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "ComCtl32.lib")

using namespace std;
#define MAX_LOADSTRING 100
#define WM_TRAYICON (WM_USER +1)
// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

NOTIFYICONDATA g_notifyIconData;
HMENU g_menu;
UINT WM_TASKBARCREATED = 0;
bool showWdw = true;
HHOOK hHook = NULL;
HINSTANCE moduleHandle;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK doKeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);

//Global Funcs
void InitNotifyIconData(HWND hWnd);
bool ListDirectoryContents(const TCHAR *sDir);
void Compare(TCHAR *buffer, int numfile);
void Addfile2list(int numfile);
void SaveFile();
void LoadFile();
void Sort();

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_QUICKLAUNCH, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_QUICKLAUNCH));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_QUICKLAUNCH));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_QUICKLAUNCH);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND hWnd;

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		200, 200, 500, 500, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}
	InitNotifyIconData(hWnd);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	Shell_NotifyIcon(NIM_ADD, &g_notifyIconData);
	ShowWindow(hWnd, SW_HIDE);
	WM_TASKBARCREATED = 1;
	showWdw = false;

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
typedef struct
{
	TCHAR Name[MAX_PATH];
	TCHAR Path[2048];
	int Usage = 0;
}File;

File fileindex[2048];
int numfile = 0;
HWND hEdit;
HWND hListbox;
int flag = 1;
int UsedApp = 0;
wfstream f;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR *buffer = NULL;
	int size;
	if (message == WM_TASKBARCREATED && !IsWindowVisible(hWnd))
	{
		Shell_NotifyIcon(NIM_ADD, &g_notifyIconData);
		ShowWindow(hWnd, SW_HIDE);
	}

	switch (message)
	{

	case WM_CREATE:
	{
		g_menu = CreatePopupMenu();
		AppendMenu(g_menu, MF_STRING, ID_SCAN, TEXT("Scan Apps"));
		AppendMenu(g_menu, MF_STRING, ID_ABOUT, TEXT("View Statistics"));
		AppendMenu(g_menu, MF_STRING, ID_TRAY_EXIT, TEXT("Exit"));


		moduleHandle = GetModuleHandle(NULL);
		hHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)doKeyboardHookProc, moduleHandle, 0);

		hEdit = CreateWindowEx(0, L"EDIT", L""
			, WS_CHILD | WS_VISIBLE |WS_BORDER |ES_LEFT,
			100, 50, 300, 25,
			hWnd, NULL, hInst, NULL);
		
		hListbox = CreateWindowEx(0, L"LISTBOX", L""
			, LBS_NOTIFY |LBS_WANTKEYBOARDINPUT| WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_VSCROLL ,
			100, 100, 300, 300,
			hWnd, NULL, hInst, NULL);

		ListDirectoryContents(L"C:\\Windows\\System32");
		flag = 1;
		ListDirectoryContents(L"C:\\Program Files (x86)");
		LoadFile();
		Sort();
		Addfile2list(numfile);

		break;
	}
	case WM_SYSCOMMAND:
	{
		switch (wParam & 0xfff0)
		{
		case SC_MINIMIZE:
		{
			if (WM_TASKBARCREATED)
				Shell_NotifyIcon(NIM_DELETE, &g_notifyIconData);
			Shell_NotifyIcon(NIM_ADD, &g_notifyIconData);
			WM_TASKBARCREATED = 1;
			showWdw = false;
			ShowWindow(hWnd, SW_HIDE);
		}break;
		case SC_CLOSE:
		{
			SaveFile();
			UnhookWindowsHookEx(hHook);
			PostQuitMessage(0);
		}break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
		}
	}
	case WM_TRAYICON:
	{
		switch (wParam)
		{
		case ID_TRAY_APP_ICON:
			break;
		}
		if (lParam == WM_LBUTTONUP)
		{
			showWdw = true;
			Shell_NotifyIcon(NIM_DELETE, &g_notifyIconData);
			WM_TASKBARCREATED = 0;
			ShowWindow(hWnd, SW_SHOW);
			SetFocus(hEdit);
		}
		else if (lParam == WM_RBUTTONDOWN)
		{
			POINT curPoint;
			GetCursorPos(&curPoint);
			SetForegroundWindow(hWnd);

			UINT clicked = TrackPopupMenu(g_menu, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, 0, hWnd, NULL);
			if (clicked == ID_TRAY_EXIT)
			{
				PostQuitMessage(0);
			}
			if (clicked == ID_ABOUT)
			{
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			}
			if (clicked == ID_SCAN)
			{
				if (WM_TASKBARCREATED)
					Shell_NotifyIcon(NIM_DELETE, &g_notifyIconData);
				WM_TASKBARCREATED = 0;
				ShowWindow(hWnd, SW_SHOW);
			}
		}
		break;
	}

	case WM_COMMAND:
	{
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmEvent)
		{
		case EN_CHANGE:
		{
			if (lParam == (LPARAM)hEdit)
			{
				size = GetWindowTextLength(hEdit);
				buffer = new TCHAR[size + 1];
				GetWindowText(hEdit, buffer, size + 1);
				Compare((TCHAR*)buffer, numfile);
				SendMessage(hListbox, LB_RESETCONTENT, 0, 0);
				Addfile2list(numfile);
				SendMessage(hListbox, LB_SETCURSEL, 0, 0);
			}
		}break;
		case LBN_DBLCLK:
		{
			int lbItem = (int)SendMessage(hListbox, LB_GETCURSEL, 0, 0);
			int i = (int)SendMessage(hListbox, LB_GETITEMDATA, lbItem, 0);
			ShellExecute(NULL, _T("open"), fileindex[i].Path, NULL, NULL, SW_SHOWNORMAL);
			if (fileindex[i].Usage == 0)
			{
				UsedApp++;
			}
			(fileindex[i].Usage)++;
			Sort();
			SendMessage(hListbox, LB_RESETCONTENT, 0, 0);
			Addfile2list(numfile);

		}break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
		}
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}

	case WM_VKEYTOITEM:
	{
		switch (LOWORD(wParam))
		{
		case VK_RETURN:
		{
			int lbItem = (int)SendMessage(hListbox, LB_GETCURSEL, 0, 0);
			int i = (int)SendMessage(hListbox, LB_GETITEMDATA, lbItem, 0);
			ShellExecute(NULL, _T("open"), fileindex[i].Path, NULL, NULL, SW_SHOWNORMAL);
			if (fileindex[i].Usage == 0)
			{
				UsedApp++;
			}
			(fileindex[i].Usage)++;
			Sort();
			SendMessage(hListbox, LB_RESETCONTENT, 0, 0);
			Addfile2list(numfile);
		}break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
		}
	}break;
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_DOWN:
			{
			SetFocus(hListbox);
			}
			break;
		}
	}break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
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

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		if (fileindex[0].Usage != 0)
		{
			SetDlgItemText(hDlg, IDC_STATIC1, fileindex[0].Name);
		}
		else
		{
			SetDlgItemText(hDlg, IDC_STATIC1, L"No data!");
		}
		if (fileindex[1].Usage != 0)
		{
			SetDlgItemText(hDlg, IDC_STATIC2, fileindex[1].Name);
		}
		else
		{
			SetDlgItemText(hDlg, IDC_STATIC2, L"No data!");
		}
		if (fileindex[2].Usage != 0)
		{
			SetDlgItemText(hDlg, IDC_STATIC3, fileindex[2].Name);
		}
		else
		{
			SetDlgItemText(hDlg, IDC_STATIC3, L"No data!");
		}
		if (fileindex[3].Usage != 0)
		{
			SetDlgItemText(hDlg, IDC_STATIC4, fileindex[3].Name);
		}
		else
		{
			SetDlgItemText(hDlg, IDC_STATIC4, L"No data!");
		}
		if (fileindex[4].Usage != 0)
		{
			SetDlgItemText(hDlg, IDC_STATIC5, fileindex[4].Name);
		}
		else
		{
			SetDlgItemText(hDlg, IDC_STATIC5, L"No data!");
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		MoveToEx(hdc, 20, 20, NULL);
		LineTo(hdc, 20, 285);
		MoveToEx(hdc, 20, 285, NULL);
		LineTo(hdc, 500, 285);
		if (fileindex[0].Usage != 0)
		{
			Rectangle(hdc, 40, 285, 70, 30 + 255 - ((fileindex[0].Usage * 255) / 100));
		}
		else
		{
			Rectangle(hdc, 40, 285, 70, 285);
		}
		if (fileindex[1].Usage != 0)
		{
			Rectangle(hdc, 125, 285, 155, 30 + 255 - ((fileindex[1].Usage * 255) / 100));
		}
		else
		{
			Rectangle(hdc, 125, 285, 155, 285);
		}
		if (fileindex[2].Usage != 0)
		{
			Rectangle(hdc, 215, 285, 245, 30 + 255 - ((fileindex[2].Usage * 255) / 100));
		}
		else
		{
			Rectangle(hdc, 215, 285, 245, 285);
		}
		if (fileindex[3].Usage != 0)
		{
			Rectangle(hdc, 310, 285, 340, 30 + 255 - ((fileindex[3].Usage * 255) / 100));
		}
		else
		{
			Rectangle(hdc, 310, 285, 340, 285);
		}
		if (fileindex[4].Usage != 0)
		{
			Rectangle(hdc, 415, 285, 445, 30 + 255 - ((fileindex[3].Usage * 255) / 100));
		}
		else
		{
			Rectangle(hdc, 415, 285, 445, 285);
		}
		EndPaint(hDlg, &ps);
		break;
	}
	return (INT_PTR)FALSE;
}


void InitNotifyIconData(HWND hWnd)
{
	memset(&g_notifyIconData, 0, sizeof(NOTIFYICONDATA));
	g_notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	g_notifyIconData.hWnd = hWnd;
	g_notifyIconData.uID = ID_TRAY_APP_ICON;
	g_notifyIconData.uFlags = NIF_ICON |
		NIF_MESSAGE |
		NIF_TIP;
	g_notifyIconData.uCallbackMessage = WM_TRAYICON;
	g_notifyIconData.hIcon = (HICON)LoadImage(NULL, TEXT("QuickLaunch.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	wcscpy_s(g_notifyIconData.szTip, TEXT("What are you looking at?"));
}
/////////////////////////////////////////////


bool ListDirectoryContents(const TCHAR *sDir)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;
	TCHAR sPath[2048];

	if (flag == 1)
	{
		StringCchCopy(sPath, MAX_PATH, sDir);
		StringCchCat(sPath, MAX_PATH, TEXT("\\*.*"));
	}
	else
	{
		StringCchCopy(sPath, MAX_PATH, sDir);
		StringCchCat(sPath, MAX_PATH, TEXT("\\*.exe"));
	}

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	do
	{
		if (wcscmp(fdFile.cFileName, L".") != 0
			&& wcscmp(fdFile.cFileName, L"..") != 0)
		{
			StringCchCopy(sPath, MAX_PATH, sDir);
			StringCchCat(sPath, MAX_PATH, TEXT("\\"));
			StringCchCat(sPath, MAX_PATH, fdFile.cFileName);
			if (fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
			{
				flag = 0;
				ListDirectoryContents(sPath);
			}
			else
			{
				if (wcsstr(sPath, L".exe") != NULL)
				{
					int size = _tcsclen(sPath);
					int size2 = _tcsclen(fdFile.cFileName);
					TCHAR temp[2048] = { L"\0" };
					_tcsncpy(temp, sPath, size - 4);
					if (wcsstr(temp, L".exe") == NULL)
					{
						_tcsncpy(fileindex[numfile].Path, sPath, size);
						_tcsncpy(fileindex[numfile].Name, fdFile.cFileName, size2 - 4);
						numfile++;
					}
					flag = 1;
				}
			}
		}
	} while (FindNextFile(hFind, &fdFile)); //Find the next file.

	FindClose(hFind); //Always, Always, clean things up!
	return true;
}

void Addfile2list(int numfile)
{
	for (int j = 0; j < numfile; j++)
	{
		int pos = (int)SendMessage(hListbox, LB_ADDSTRING, 0,
			(LPARAM)fileindex[j].Name);
		SendMessage(hListbox, LB_SETITEMDATA, pos, (LPARAM)j);
	}
}

void Compare(TCHAR* buffer, int numfile)
{
	int k = 0;
	TCHAR *str = NULL;
	int size;
	for (int i = 0; i < numfile; i++)
	{
		size = (int)SendMessage(hListbox, LB_GETTEXTLEN, (WPARAM)i, 0);
		str = new TCHAR[size + 1];
		SendMessage(hListbox, LB_GETTEXT, (WPARAM)i, (LPARAM)str);
		if (wcsstr((TCHAR*)_strlwr((char*)str), (TCHAR*)_strlwr((char*)buffer)) != NULL)
		{
			File temp = fileindex[k];
			fileindex[k] = fileindex[i];
			fileindex[i] = temp;
			k++;
		}
	}
}

LRESULT CALLBACK doKeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (!showWdw)
	{
		Shell_NotifyIcon(NIM_ADD, &g_notifyIconData);
		WM_TASKBARCREATED = 1;
		ShowWindow(hWnd, SW_HIDE);
	}
	if (showWdw)
	{
		if (WM_TASKBARCREATED)
			Shell_NotifyIcon(NIM_DELETE, &g_notifyIconData);
		WM_TASKBARCREATED = 0;
		ShowWindow(hWnd, SW_SHOW);
	}

	if ((GetAsyncKeyState(VK_SPACE) && GetAsyncKeyState(VK_LCONTROL) && GetAsyncKeyState(VK_LSHIFT)< 0))
	{
		showWdw = !showWdw;
	}

	return CallNextHookEx(hHook, nCode, wParam, lParam);
}


void SaveFile()
{
	if (UsedApp != 0)
	{

		f.open("data.txt", ios::out);
		f << UsedApp << endl;
		for (int i = 0; i < numfile; i++)
		{
			if (fileindex[i].Usage != 0)
			{
				f << fileindex[i].Name << endl;
				f << fileindex[i].Usage << endl;
			}
		}
		f.close();
	}
}

void LoadFile()
{
	if(f.eof())
	{
		return;
	}
	else {
		f.open("data.txt", ios::in);
		f >> UsedApp;
		TCHAR name[MAX_PATH];
		int usage;

		for (int i = 0; i < UsedApp; i++)
		{
			f >> name;
			f >> usage;
			for (int j = 0; j < numfile; j++)
			{
				if ((wcsstr(fileindex[j].Name, name) != NULL) && (_tcscmp(fileindex[j].Name, name) == 0))
				{
					fileindex[j].Usage = usage;
				}
			}
		}
	}
	f.close();
}

void Sort()
{
	for (int i = 0; i<numfile; i++)
		for(int j = i+1; j<numfile; j++)
		{
			if(fileindex[i].Usage < fileindex[j].Usage)
			{
				File temp = fileindex[j];
				fileindex[j] = fileindex[i];
				fileindex[i] = temp;
			}
		}
}