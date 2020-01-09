#include <stdafx.h>
#include "MsgHook.h"
#include <Windows.h>

HMODULE g_hModule ;
HHOOK g_hook = NULL;

LRESULT CALLBACK HookProc(
	int code, // hook code
	WPARAM wParam, // removal option
	LPARAM lParam // message
	)
{
	if(g_hook)
	{
		return CallNextHookEx(g_hook,code,wParam,lParam);
	}
	return 1;
}

void SetMsgHookOn()
{
	OutputDebugString(_T("SetMsgHookOn enter--------"));
	g_hook = SetWindowsHookEx(WH_GETMESSAGE,HookProc,g_hModule,0);
	if(g_hook == NULL )
	{
		OutputDebugString(_T("SetWindowsHookEx false"));
	}
}

void SetMsgHookOff()
{
	OutputDebugString(_T("SetMsgHookOff enter++++++++"));
	BOOL ret = FALSE;
	if(g_hook)
	{
		ret = UnhookWindowsHookEx(g_hook);
	}
	if(ret == FALSE)
	{
		OutputDebugString(_T("UnhookWindowsHookEx false++++++++"));
	}
}

// �µ�ȫ�ֹ��ӱ�ϵͳǿ��ע�뵽�����к�֪ͨ���Ŀ�����
void NotifyYourApp(BOOL bTatch)
{
	HWND targetWnd = FindWindow(MAIN_APP_CLASS,MAIN_APP_TITLE);
	if(targetWnd == NULL)
	{
		OutputDebugString(_T("target exe not found"));
		// Ŀ�����û����������û�ô���ȡ��ȫ�ֹ���
		SetMsgHookOff();
		return;
	}

	HMODULE hModule = GetModuleHandle(NULL);
	TCHAR exePath[MAX_PATH] = {0};
	int ret = GetModuleFileName(hModule,exePath,MAX_PATH);
	CString strPath = exePath;
	int lastFlag = strPath.ReverseFind('\\');
	CString exeName = strPath.Right(strPath.GetLength() - lastFlag - 1);
	if(exeName.CompareNoCase(_T("iexplore.exe")) == 0)
	{
		OutputDebugString(_T("windows hook proc inject iexplore.exe"));
		// ��ǰע��Ľ���ΪIE�����,֪ͨĿ�����
		HANDLE ieHandle = GetCurrentProcess();
		PostMessage(targetWnd,WM_IE_OPEN,bTatch,(LPARAM)ieHandle);
	}
}

// DLL ���
BOOL APIENTRY DllMain( HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	g_hModule = hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			NotifyYourApp(TRUE);
		}
		break;
	case DLL_PROCESS_DETACH:
		{
			// ʲô��� ����DLL�ᱻж�أ������˳���
			NotifyYourApp(FALSE);
		}
		break;
	}
	return TRUE;
}